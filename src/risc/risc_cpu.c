#include "risc/risc_cpu.h"
#include "risc/risc_memory.h"
#include "tools/logger.h"

#include <stdint.h>
#include <string.h>

#define MISA_EXTENSION(x)(1u << ((x - 'A')))
#define MISA_MXL32 1u << 30

#define MSTATUS_TSR     22
#define MSTATUS_SPP     8
#define MSTATUS_SPIE    5
#define MSTATUS_SIE     1
#define MSTATUS_MPRV    17
#define MSTATUS_MPP     11
#define MSTATUS_MPIE    7
#define MSTATUS_MIE     3

typedef enum RISC_INSTRUCTION_TYPE 
{
    INSTRUCTION_ARITHMETIC_R    = 0b01100,
    INSTRUCTION_ARITHMETIC_I    = 0b00100,
    INSTRUCTION_LOAD            = 0b00000,
    INSTRUCTION_STORE           = 0b01000,
    INSTRUCTION_BRANCH          = 0b11000,
    INSTRUCTION_JUMP_J          = 0b11011,
    INSTRUCTION_JUMP_I          = 0b11001,
    INSTRUCTION_UPPER_IMM       = 0b01101,
    INSTRUCTION_UPPER_IMM_2     = 0b00101,
    INSTRUCTION_SYSTEM          = 0b11100,
    INSTRUCTION_AMO             = 0b01011
} RISC_INSTRUCTION_TYPE ;

/* -------------------------------------------------------------------------- */

static inline int RiscOpError(risc_cpu_t *cpu, uint32_t instruction);

static inline uint32_t InstructionGetOpcode(uint32_t instruction);
static inline uint32_t InstructionGetRd(uint32_t instruction);
static inline uint32_t InstructionGetRs1(uint32_t instruction);
static inline uint32_t InstructionGetRs2(uint32_t instruction);
static inline uint32_t InstructionGetFunct3(uint32_t instruction);
static inline uint32_t InstructionGetFunct7(uint32_t instruction);
static inline uint32_t InstructionGetFunct12(uint32_t instruction);
static inline uint32_t InstructionGetCsr(uint32_t instruction);
static inline uint32_t InstructionGetImmI(uint32_t instruction);
static inline uint32_t InstructionGetImmS(uint32_t instruction);
static inline uint32_t InstructionGetImmB(uint32_t instruction);
static inline uint32_t InstructionGetImmU(uint32_t instruction);
static inline uint32_t InstructionGetImmJ(uint32_t instruction);
static inline uint32_t InstructionGetRl(uint32_t instruction);
static inline uint32_t InstructionGetAq(uint32_t instruction);
static inline uint32_t InstructionGetFunc5(uint32_t instruction);

/* -------------------------------------------------------------------------- */

int RiscCpuInitialize(risc_cpu_t *cpu, risc_memory_t *memory)
{
    if (memory == NULL)
    {
        RiscMemoryInitialize(memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    }

    memset(cpu->registers, 0, sizeof(uint32_t) * RISC_REGISTER_COUNT);
    cpu->pc = 0x00000000;

    cpu->memory = memory;

    for (size_t i = 0; i < RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT; i++)
    {
        cpu->instructionTypeHandler[i] = &RiscOpError;
    }
    
    cpu->instructionTypeHandler[INSTRUCTION_ARITHMETIC_R] = &RiscOpArithmeticR;
    cpu->instructionTypeHandler[INSTRUCTION_ARITHMETIC_I] = &RiscOpArithmeticI;
    cpu->instructionTypeHandler[INSTRUCTION_LOAD] = &RiscOpLoad;
    cpu->instructionTypeHandler[INSTRUCTION_STORE] = &RiscOpStore;
    cpu->instructionTypeHandler[INSTRUCTION_BRANCH] = &RiscOpBranch;
    cpu->instructionTypeHandler[INSTRUCTION_JUMP_J] = &RiscOpJumpJ;
    cpu->instructionTypeHandler[INSTRUCTION_JUMP_I] = &RiscOpJumpI;
    cpu->instructionTypeHandler[INSTRUCTION_UPPER_IMM] = &RiscOpUpperImm;
    cpu->instructionTypeHandler[INSTRUCTION_UPPER_IMM_2] = &RiscOpUpperImm;
    cpu->instructionTypeHandler[INSTRUCTION_SYSTEM] = &RiscOpSystem;

    #ifdef RISCV_EXTENSION_A
    cpu->instructionTypeHandler[INSTRUCTION_AMO] = &RiscOpAmo;
    #endif // RISCV_EXTENSION_A

    cpu->halted = false;
    cpu->reservationAddress = 0x00;
    cpu->reservationValid = false;

    cpu->privilegeMode = RISC_PRIVILEGE_M;

    cpu->csrs[RISC_CSR_MISA] = (
        MISA_MXL32 |
        MISA_EXTENSION('A') |
        MISA_EXTENSION('M') |
        MISA_EXTENSION('I') |
        MISA_EXTENSION('S') |
        MISA_EXTENSION('U')
    );

    cpu->csrs[RISC_CSR_MVENDORID] = 0;
    cpu->csrs[RISC_CSR_MARCHID] = 0;
    cpu->csrs[RISC_CSR_MIMPID] = 0;
    cpu->csrs[RISC_CSR_MSTATUS] = 0;
    cpu->csrs[RISC_CSR_MSTATUSH] = 0;

    return 0;
}

int RiscCpuDestroy(risc_cpu_t *cpu)
{
    LoggerMessage(DEBUG, "[%s] Destroying CPU", __FUNCTION__);
    RiscMemoryDestroy(cpu->memory);
    return 0;
}

int RiscCpuExecute(risc_cpu_t *cpu)
{
    cpu->registers[RISC_ABI_ZERO] = 0;
    uint32_t instruction = 0;
    
    uint32_t current_pc = cpu->pc;
    int status = RiscMemoryRead(cpu->memory, current_pc, DWORD, &instruction);

    uint8_t opcode = InstructionGetOpcode(instruction);

    LoggerInstruction(
        DEBUG,
        opcode,
        InstructionGetFunct3(instruction),
        InstructionGetFunct7(instruction),
        InstructionGetRd(instruction),
        InstructionGetRs1(instruction),
        InstructionGetRs2(instruction),
        InstructionGetImmB(instruction)
    );

    cpu->instructionTypeHandler[opcode >> 2](cpu, instruction);

    if (cpu->pc == current_pc)
    {
        cpu->pc += 4;
    }

    return status;
}

int RiscOpArithmeticR(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t funct7 = InstructionGetFunct7(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t rs2 = InstructionGetRs2(instruction);

    uint32_t funct10 = funct7 << 3 | funct3;

    uint32_t *reg = &cpu->registers[0];

    int status = 0;

    #ifdef RISCV_EXTENSION_M
    int64_t s1;
    int64_t s2;
    int64_t us1;
    int64_t us2;
    int64_t res;
    uint64_t ures;

    int32_t dividend = (int32_t) reg[rs1];
    int32_t divisor = (int32_t) reg[rs2];
    uint32_t udividend = (uint32_t) reg[rs1];
    uint32_t udivisor = (uint32_t) reg[rs2];
    #endif // RISCV_EXTENSION_M

    switch (funct10)
    {
        case 0b0000000000:
            // Add
            reg[rd] = reg[rs1] + reg[rs2];
            break;
        case 0b0100000000:
            // Sub
            reg[rd] = reg[rs1] - reg[rs2];
            break;
        case 0b0000000111:
            // And
            reg[rd] = reg[rs1] & reg[rs2];
            break;
        case 0b0000000110:
            // Or
            reg[rd] = reg[rs1] | reg[rs2];
            break;
        case 0b0000000100:
            // Xor
            reg[rd] = reg[rs1] ^ reg[rs2];
            break;
        case 0b0000000001:
            // Shift left
            reg[rd] = reg[rs1] << reg[rs2];
            break;
        case 0b0000000101:
            // Shift right logical
            reg[rd] = reg[rs1] >> (reg[rs2] & 0x1F);
            break;
        case 0b0100000101:
            // Shift right arithmetic
            reg[rd] = (uint32_t) ((int32_t)reg[rs1] >> (reg[rs2] & 0x1F));
            break;
        case 0b0000000010:
            // Less than (signed)
            if ((int32_t) reg[rs1] < (int32_t) reg[rs2]) 
            {
                reg[rd] = 1;
            }
            else 
            {
                reg[rd] = 0;
            }
            break;
        case 0b0000000011:
            // Less than (unsigned)
            if (reg[rs1] < reg[rs2]) 
            {
                reg[rd] = 1;
            }
            else 
            {
                reg[rd] = 0;
            }
            break;
#ifdef RISCV_EXTENSION_M
        
        case 0b0000001000:
            // MUL
            reg[rd] = (uint32_t)((int32_t)reg[rs1] * (int32_t)reg[rs2]);
            break;
        case 0b0000001001:
            // MULH
            s1 = (int64_t)(int32_t)reg[rs1];
            s2 = (int64_t)(int32_t)reg[rs2];
            res = s1 * s2;
            reg[rd] = (uint32_t)(res >> RISCV_XLEN);
            break;
        case 0b0000001010:
            // MULHSU
            s1 = (int64_t)(int32_t)reg[rs1];
            us2 = (uint64_t)reg[rs2];
            res = s1 * (int64_t)us2;
            reg[rd] = (uint32_t)(res >> RISCV_XLEN);
            break;
        case 0b0000001011:
            // MULHU
            us1 = (uint64_t)reg[rs1];
            us2 = (uint64_t)reg[rs2];
            ures = us1 * us2;
            reg[rd] = (uint32_t)(ures >> RISCV_XLEN);
            break;
        case 0b0000001100:
            // DIV
            // division by zero
            if (divisor == 0) 
            {
                reg[rd] = UINT32_MAX;
            }
            // division overflow
            else if (dividend == INT32_MIN && divisor == -1)
            {
                reg[rd] = (uint32_t)INT32_MIN;
            }
            else
            {
                reg[rd] = (uint32_t)(dividend / divisor);
            }
            break;
        case 0b0000001101:
            // DIVU
            // division by zero
            if (udivisor == 0)
            {
                reg[rd] = UINT32_MAX;
            }
            else
            {
                reg[rd] = udividend / udivisor;
            }
            break;
        case 0b0000001110:
            // REM
            if (divisor == 0)
            {
                reg[rd] = (uint32_t)dividend;
            }
            else if (dividend == INT32_MIN && divisor == -1)
            {
                reg[rd] = 0;
            }
            else
            {
                reg[rd] = (uint32_t)(dividend % divisor);
            }
            break;
        case 0b0000001111:
            // REMU
            if (udivisor == 0)
            {
                reg[rd] = udividend;
            }
            else
            {
                reg[rd] = udividend % udivisor;
            }
            break;
#endif // RISCV_EXTENSION_M
        default:
            status = 1;
            break;
    }

    return status;
}

int RiscOpArithmeticI(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t imm = InstructionGetImmI(instruction);

    uint32_t *reg = &cpu->registers[0];

    int status = 0;

    switch (funct3)
    {
        case 0b000:
            // Add
            reg[rd] = reg[rs1] + imm;
            break;
        case 0b111:
            // And
            reg[rd] = reg[rs1] & imm;
            break;
        case 0b110:
            // Or
            reg[rd] = reg[rs1] | imm;
            break;
        case 0b100:
            // Xor
            reg[rd] = reg[rs1] ^ imm;
            break;
        case 0b010:
            // Less than (signed)
            if ((int32_t) reg[rs1] < (int32_t) imm) 
            {
                reg[rd] = 1;
            }
            else 
            {
                reg[rd] = 0;
            }
            break;
        case 0b011:
            // Less than (unsigned)
            if (reg[rs1] < imm) 
            {
                reg[rd] = 1;
            }
            else 
            {
                reg[rd] = 0;
            }
            break;
        case 0b001:
            // Shift left
            reg[rd] = reg[rs1] << (imm & 0x1F);
            break;
        case 0b101:
            if ((instruction >> 30) & 1) 
            {
                // Shift right arithmetic (sign-extend)
                reg[rd] = (uint32_t)((int32_t)reg[rs1] >> (imm & 0x1F));
            }
            else 
            {
                // Shift right logical (zero-extend)
                reg[rd] = reg[rs1] >> (imm & 0x1F);
            }
            break;
        default:
            status = 1;
            break;
    }

    return status;
}

int RiscOpLoad(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t imm = InstructionGetImmI(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);

    uint32_t *reg = &cpu->registers[0];

    uint32_t data = 0;

    uint32_t address = reg[rs1] + imm;

    int status = 0;

    switch (funct3)
    {
        case 0b000:
            status = RiscMemoryRead(cpu->memory, address, BYTE, &data);
            reg[rd] = (uint32_t)(int8_t)data;
            break;
        case 0b100:
            status = RiscMemoryRead(cpu->memory, address, BYTE, &data);
            reg[rd] = data;
            break;
        case 0b001:
            status = RiscMemoryRead(cpu->memory, address, WORD, &data);
            reg[rd] = (uint32_t)(int16_t)data;
            break;
        case 0b101:
            status = RiscMemoryRead(cpu->memory, address, WORD, &data);
            reg[rd] = data;
            break;
        case 0b010:
            status = RiscMemoryRead(cpu->memory, address, DWORD, &data);
            reg[rd] = data;
            break;
        default:
            status = 1;
            break;
        }

    return status;
}

int RiscOpStore(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t imm = InstructionGetImmS(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t rs2 = InstructionGetRs2(instruction);

    uint32_t *reg = &cpu->registers[0];

    uint32_t address = reg[rs1] + imm;

    int status = 0;

    if (address == RISC_ADDRESS_TOHOST && reg[rs2] != 0)
    {
        cpu->halted = true;
        return 0;
    }

    switch (funct3)
    {
        case 0b000:
            status = RiscMemoryWrite(cpu->memory, address, BYTE, reg[rs2]);
            break;
        case 0b001:
            status = RiscMemoryWrite(cpu->memory, address, WORD, reg[rs2]);
            break;
        case 0b010:
            status = RiscMemoryWrite(cpu->memory, address, DWORD, reg[rs2]);
            break;
        default:
            status = 1;
            break;
        }

    return status;
}

int RiscOpBranch(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t imm = InstructionGetImmB(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t rs2 = InstructionGetRs2(instruction);

    uint32_t *reg = &cpu->registers[0];

    int status = 0;

    switch (funct3)
    {
        case 0b000:
            if (reg[rs1] == reg[rs2])
            {
                cpu->pc += imm;
            }
            break;
        case 0b001:
            if (reg[rs1] != reg[rs2])
            {
                cpu->pc += imm;
            }
            break;
        case 0b100:
            if (((int32_t) reg[rs1]) < ((int32_t) reg[rs2]))
            {
                cpu->pc += imm;
            }
            break;
        case 0b110:
            if (((uint32_t)reg[rs1]) < ((uint32_t)reg[rs2]))
            {
                cpu->pc += imm;
            }
            break;
        case 0b101:
            if (((int32_t) reg[rs1]) >= ((int32_t) reg[rs2]))
            {
                cpu->pc += imm;
            }
            break;
        case 0b111:
            if (((uint32_t)reg[rs1]) >= ((uint32_t)reg[rs2]))
            {
                cpu->pc += imm;
            }
            break;
        default:
            status = 1;
            break;
    }

    return status;
}

int RiscOpJumpJ(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t imm = InstructionGetImmJ(instruction);
    uint32_t rd = InstructionGetRd(instruction);

    uint32_t *reg = &cpu->registers[0];

    reg[rd] = cpu->pc + 4;
    cpu->pc += imm;

    return 0;
}

int RiscOpJumpI(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t imm = InstructionGetImmI(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);

    uint32_t *reg = &cpu->registers[0];

    uint32_t target = (reg[rs1] + imm) & ~0x01;

    reg[rd] = cpu->pc + 4;
    cpu->pc = target;

    return 0;
}

int RiscOpUpperImm(risc_cpu_t *cpu, uint32_t instruction)
{
    uint8_t opcode = InstructionGetOpcode(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t imm = InstructionGetImmU(instruction);

    if (opcode == 0b0110111) // LUI
    {
        cpu->registers[rd] = imm;
    }
    else if (opcode == 0b0010111) // AUIPC
    {
        cpu->registers[rd] = cpu->pc + imm;
    }
    return 0;
}

int RiscOpSystem(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t csr = InstructionGetCsr(instruction);
    uint32_t funct12 = InstructionGetFunct12(instruction);
    uint32_t imm = rs1;
    uint32_t *reg = &cpu->registers[0];

    uint32_t oldValue = 0;

    uint32_t mstatus;
    uint32_t mprv;

    switch (funct3)
    {
        case 0b000:
            switch (funct12)
            {
                // ECALL
                case 0b000000000000:
                    RiscEnterTrap(cpu, (RISC_TRAP_CAUSE)(RISC_TRAP_ECALL_U_MODE + cpu->privilegeMode), RISC_TRAP_EXCEPTION);
                    break;
                // EBREAK
                case 0b000000000001:
                    RiscEnterTrap(cpu, RISC_TRAP_BREAKPOINT, RISC_TRAP_EXCEPTION);
                    break;
                // SRET
                case 0b000100000010:
                    if (
                        cpu->privilegeMode < RISC_PRIVILEGE_S || 
                        (cpu->privilegeMode == RISC_PRIVILEGE_S && (((cpu->csrs[RISC_CSR_MSTATUS] >> MSTATUS_TSR) & 0x01) == 1))
                    )
                    {
                        RiscEnterTrap(cpu, RISC_TRAP_ILLEGAL_INSTRUCTION, RISC_TRAP_EXCEPTION);
                        break;
                    }

                    mstatus = cpu->csrs[RISC_CSR_MSTATUS];

                    uint32_t spp = (mstatus >> MSTATUS_SPP) & 0x01;
                    uint32_t spie = (mstatus >> MSTATUS_SPIE) & 0x01;

                    mprv = (mstatus >> MSTATUS_MPRV) & 0x01;

                    if (spp != RISC_PRIVILEGE_M)
                    {
                        mprv = 0;
                    }

                    cpu->privilegeMode = (RISC_PRIVILEGE_MODE) spp;
                    spp = RISC_PRIVILEGE_U;

                    // Clear SIE bit and set it afterwards
                    mstatus &= ~(1 << MSTATUS_SIE);
                    mstatus |= (spie << MSTATUS_SIE);

                    // Set SPIE bit to 1
                    mstatus |= (1 << MSTATUS_SPIE);

                    // Clear SPP bit and set it afterwards
                    mstatus &= ~(1 << MSTATUS_SPP);
                    mstatus |= spp << MSTATUS_SPP;

                    // Clear MPRV bit and set it afterwards
                    mstatus &= ~(1 << MSTATUS_MPRV);
                    mstatus |= mprv << MSTATUS_MPRV;

                    cpu->csrs[RISC_CSR_MSTATUS] = mstatus;

                    cpu->pc = cpu->csrs[RISC_CSR_SEPC];
                    cpu->reservationValid = false;
                    break;
                    
                // MRET
                case 0b001100000010:

                    if (cpu->privilegeMode < RISC_PRIVILEGE_M)
                    {
                        RiscEnterTrap(cpu, RISC_TRAP_ILLEGAL_INSTRUCTION, RISC_TRAP_EXCEPTION);
                        break;
                    }

                    mstatus = cpu->csrs[RISC_CSR_MSTATUS];

                    uint32_t mpp = (mstatus >> MSTATUS_MPP) & 0x03;
                    uint32_t mpie = (mstatus >> MSTATUS_MPIE) & 0x01;
                    
                    mprv = (mstatus >> MSTATUS_MPRV) & 0x01;

                    if (mpp != RISC_PRIVILEGE_M)
                    {
                        mprv = 0;
                    }

                    cpu->privilegeMode = (RISC_PRIVILEGE_MODE) mpp;
                    mpp = RISC_PRIVILEGE_U;

                    mstatus &= ~(1 << MSTATUS_MIE);
                    mstatus |= (mpie << MSTATUS_MIE);

                    // Set MPIE bit to 1
                    mstatus |= (1 << MSTATUS_MPIE);

                    // Clear MPP bit and set it afterwards
                    mstatus &= ~(3 << MSTATUS_MPP);
                    mstatus |= mpp << MSTATUS_MPP;

                    // Clear MPRV bit and set it afterwards
                    mstatus &= ~(1 << MSTATUS_MPRV);
                    mstatus |= mprv << MSTATUS_MPRV;

                    cpu->csrs[RISC_CSR_MSTATUS] = mstatus;

                    cpu->pc = cpu->csrs[RISC_CSR_MEPC];
                    cpu->reservationValid = false;
                    break;
                    
                // MNRET
                case 0b011100000010:
                    break;
            }
            break;
        #ifdef RISCV_EXTENSION_ZICSR
        // CSRRW
        case 0b001:
            if (rd != RISC_ABI_ZERO)
            {
                oldValue = cpu->csrs[csr];
            }
            
            cpu->csrs[csr] = reg[rs1];

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }

            break;
        // CSRRS
        case 0b010:
            oldValue = cpu->csrs[csr];
            
            if (rs1 != RISC_ABI_ZERO)
            {
                cpu->csrs[csr] = oldValue | reg[rs1];
            }

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }
            break;
        // CSRRC
        case 0b011:
            oldValue = cpu->csrs[csr];
            
            if (rs1 != RISC_ABI_ZERO)
            {
                cpu->csrs[csr] = oldValue ^ reg[rs1];
            }

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }
            break;
        // CSRRWI
        case 0b101:
            oldValue = cpu->csrs[csr];
            
            cpu->csrs[csr] = imm;

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }
            break;
        // CSRRSI
        case 0b110:
            oldValue = cpu->csrs[csr];
            
            cpu->csrs[csr] = oldValue ^ imm;

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }
            break;
        // CSRRCI
        case 0b111:
            oldValue = cpu->csrs[csr];
            
            cpu->csrs[csr] = oldValue ^ imm;

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = oldValue;
            }
            break;
        #endif // RISCV_EXTENSION_ZICSR
        default:
            break;
    }

    return 0;
}

int RiscOpAmo(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t rs2 = InstructionGetRs2(instruction);
    uint32_t rl = InstructionGetRl(instruction);
    uint32_t aq = InstructionGetAq(instruction);
    uint32_t funct5 = InstructionGetFunc5(instruction);

    uint32_t *reg = &cpu->registers[0];

    uint32_t funct8 = (funct5 << 3) | funct3;

    uint32_t originalValue = 0;

    switch (funct8)
    {
        // LR.W
        case 0b00010010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            cpu->reservationAddress = reg[rs1];
            cpu->reservationValid = true;
            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // SC.W
        case 0b00011010:
            if (cpu->reservationValid && cpu->reservationAddress == reg[rs1])
            {
                RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, reg[rs2]);
                if (rd != RISC_ABI_ZERO)
                {
                    reg[rd] = 0;
                }
            }
            else
            {
                if (rd != RISC_ABI_ZERO)
                {
                    reg[rd] = 1;
                }
            }

            cpu->reservationValid = false;
            break;
        // AMOSWAP.W
        case 0b00001010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }

            break;
        // AMOADD.W
        case 0b00000010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, reg[rs2] + originalValue);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOXOR.W
        case 0b00100010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, originalValue ^ reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOAND.W
        case 0b01100010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, originalValue & reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOOR.W
        case 0b01000010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, originalValue | reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOMIN.W
        case 0b10000010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, (int32_t)originalValue < (int32_t)reg[rs2] ? originalValue : reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOMAX.W
        case 0b10100010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, (int32_t)originalValue > (int32_t)reg[rs2] ? originalValue : reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOMINU.W
        case 0b11000010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, (uint32_t)originalValue < (uint32_t) reg[rs2] ? (uint32_t) originalValue : (uint32_t) reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
        // AMOMAXU.W
        case 0b11100010:
            RiscMemoryRead(cpu->memory, reg[rs1], DWORD, &originalValue);
            RiscMemoryWrite(cpu->memory, reg[rs1], DWORD, (uint32_t)originalValue > (uint32_t) reg[rs2] ? (uint32_t) originalValue : (uint32_t) reg[rs2]);

            if (rd != RISC_ABI_ZERO)
            {
                reg[rd] = originalValue;
            }
            break;
    }

    return 0;
}

void RiscEnterTrap(risc_cpu_t *cpu, RISC_TRAP_CAUSE trapCause, RISC_TRAP_TYPE trapType)
{
    uint32_t delegate = 0;
    uint32_t cause = (uint32_t) trapCause;

    if (trapType == RISC_TRAP_INTERRUPT)
    {
        delegate = cpu->csrs[RISC_CSR_MIDELEG];
        cause |= (1 << 31);
    }
    else if (trapType == RISC_TRAP_EXCEPTION)
    {
        delegate = cpu->csrs[RISC_CSR_MEDELEG];
    }

    uint32_t mstatus = cpu->csrs[RISC_CSR_MSTATUS];

    if (
        cpu->privilegeMode <= RISC_PRIVILEGE_S &&
        (((delegate >> trapCause) & 0x01) == 1)
    )
    {
        uint32_t sie = (mstatus >> MSTATUS_SIE) & 0x01;

        cpu->csrs[RISC_CSR_SCAUSE] = cause;
        cpu->csrs[RISC_CSR_SEPC] = cpu->pc;
        cpu->csrs[RISC_CSR_STVAL] = 0;

        mstatus &= ~(1 << MSTATUS_SPIE);
        mstatus |= (1 << MSTATUS_SPIE);

        mstatus &= ~(1 << MSTATUS_SPIE);
        mstatus |= (sie << MSTATUS_SPIE);

        mstatus &= ~(1 << MSTATUS_SIE);
        
        mstatus &= ~(1 << MSTATUS_SPP);
        mstatus |= (cpu->privilegeMode << MSTATUS_SPP) & 0x01;

        cpu->csrs[RISC_CSR_MSTATUS] = mstatus;

        cpu->privilegeMode = RISC_PRIVILEGE_S;

        cpu->pc = cpu->csrs[RISC_CSR_STVEC] & ~0x03;
    }
    else
    {
        cpu->csrs[RISC_CSR_MCAUSE] = cause;
        cpu->csrs[RISC_CSR_MEPC] = cpu->pc;
        cpu->csrs[RISC_CSR_MTVAL] = 0;

        uint32_t mie = (mstatus >> MSTATUS_MIE) & 0x01;

        mstatus &= ~(1 << MSTATUS_MPIE);
        mstatus |= (1 << MSTATUS_MPIE);

        mstatus &= ~(1 << MSTATUS_MPIE);
        mstatus |= (mie << MSTATUS_MPIE);

        mstatus &= ~(1 << MSTATUS_MIE);

        mstatus &= ~(3 << MSTATUS_MPP);
        mstatus |= (cpu->privilegeMode << MSTATUS_MPP) & 0x03;

        cpu->csrs[RISC_CSR_MSTATUS] = mstatus;

        cpu->privilegeMode = RISC_PRIVILEGE_M;
        cpu->pc = cpu->csrs[RISC_CSR_MTVEC] & ~0x03;
    }
}

int RiscOpError(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

/* --------------------------- Basic instructions --------------------------- */
uint32_t InstructionGetOpcode(uint32_t instruction)
{
    return instruction & 0x7F;
}

uint32_t InstructionGetRd(uint32_t instruction)
{
    return (instruction >> 7) & 0x1F;
}

uint32_t InstructionGetRs1(uint32_t instruction)
{
    return (instruction >> 15) & 0x1F;
}

uint32_t InstructionGetRs2(uint32_t instruction)
{
    return (instruction >> 20) & 0x1F;
}

uint32_t InstructionGetFunct3(uint32_t instruction)
{
    return (instruction >> 12) & 0x07;
}

uint32_t InstructionGetFunct7(uint32_t instruction)
{
    return (instruction >> 25) & 0x7F;
}

uint32_t InstructionGetFunct12(uint32_t instruction)
{
    return (instruction >> 20) & 0xFFF;
}

uint32_t InstructionGetCsr(uint32_t instruction)
{
    return instruction >> 20;
}

/* ------------------------------- Immediates ------------------------------- */
uint32_t InstructionGetImmI(uint32_t instruction)
{
    return (uint32_t)((int32_t)instruction >> 20);
}

uint32_t InstructionGetImmS(uint32_t instruction)
{
    int32_t imm = (int32_t)(instruction & 0xFE000000) >> 20;
    imm |= (int32_t)((instruction >> 7) & 0x1F);

    return (uint32_t) imm;
}

uint32_t InstructionGetImmB(uint32_t instruction)
{
    int32_t imm = (
        ((int32_t)(instruction & 0x80000000)) >> 19 |  // imm[12]
        ((int32_t)(instruction >> 7) & 0x01) << 11 | // imm[11]
        ((int32_t)(instruction >> 25) & 0x3F) << 5 | // imm[10:5]
        ((int32_t)(instruction >> 8) & 0x0F) << 1    // imm[4:1]
    );

    return (uint32_t) imm;
}

uint32_t InstructionGetImmU(uint32_t instruction)
{
    return instruction & 0xFFFFF000;
}

uint32_t InstructionGetImmJ(uint32_t instruction)
{
    int32_t imm = 0;

    imm |= ((int32_t)(instruction & 0x80000000) >> 11);
    imm |= ((int32_t)((instruction >> 12) & 0xFF) << 12);
    imm |= ((int32_t)((instruction >> 20) & 0x01) << 11);
    imm |= ((int32_t)((instruction >> 21) & 0x3FF) << 1);

    return (uint32_t) imm;
}

uint32_t InstructionGetRl(uint32_t instruction)
{
    return (instruction >> 25) & 0x01;
}

uint32_t InstructionGetAq(uint32_t instruction)
{
    return (instruction >> 26) & 0x01;
}

uint32_t InstructionGetFunc5(uint32_t instruction)
{
    return (instruction >> 27) & 0x1F;
}

/* -------------------------------------------------------------------------- */

uint32_t RiscInstructionCreateR(
    uint32_t opcode, 
    uint32_t rd, 
    uint32_t funct3, 
    uint32_t rs1, 
    uint32_t rs2, 
    uint32_t funct7
)
{
    return  (opcode & 0x7F) |
            ((rd & 0x1F) << 7) |
            ((funct3 & 0x07) << 12) |
            ((rs1 & 0x1F) << 15) |
            ((rs2 & 0x1F) << 20) |
            ((funct7 & 0x7F) << 25);
}

uint32_t RiscInstructionCreateI(
    uint32_t opcode,
    uint32_t rd,
    uint32_t funct3,
    uint32_t rs1,
    uint32_t imm
)
{
    return  (opcode & 0x7F) |
            ((rd & 0x1F) << 7) |
            ((funct3 & 0x07) << 12) |
            ((rs1 & 0x1F) << 15) |
            ((imm & 0xFFF) << 20);
}

uint32_t RiscInstructionCreateS(
    uint32_t opcode, 
    uint32_t funct3, 
    uint32_t rs1, 
    uint32_t rs2,
    uint32_t imm
)
{
    return  (opcode & 0x7F) |
            ((imm & 0x1F) << 7) |
            ((funct3 & 0x07) << 12) |
            ((rs1 & 0x1F) << 15) |
            ((rs2 & 0x1F) << 20) |
            (((imm >> 5) & 0x7F) << 25);
}

uint32_t RiscInstructionCreateB(
    uint32_t opcode,
    uint32_t funct3,
    uint32_t rs1,
    uint32_t rs2,
    uint32_t imm
)
{
    return  (opcode & 0x7F) |
            (((imm >> 11) & 0x01) << 7) |
            (((imm >> 1) & 0x0F) << 8) |
            ((funct3 & 0x07) << 12) |
            ((rs1 & 0x1F) << 15) |
            ((rs2 & 0x1F) << 20) |
            (((imm >> 5) & 0x3F) << 25) |
            (((imm >> 12) & 0x01) << 31);
}

uint32_t RiscInstructionCreateJ(
    uint32_t opcode,
    uint32_t rd,
    uint32_t imm
)
{
    return  (opcode & 0x7F) |
            ((rd & 0x1F) << 7) |
            (((imm >> 12) & 0xFF) << 12) | 
            (((imm >> 11) & 0x01) << 20) | 
            (((imm >> 1)  & 0x3FF) << 21) | 
            (((imm >> 20) & 0x01) << 31);  
}

uint32_t RiscInstructionCreateU(uint32_t opcode, uint32_t rd, uint32_t imm)
{
    return  (opcode & 0x7F) |
            ((rd & 0x1F) << 7) |
            (imm & 0xFFFFF000);
}