#include "risc/risc-cpu.h"
#include "risc/risc-memory.h"

#include <string.h>

#define GET_OPCODE(x)(x & 0b00111111)

static int RiscOpError(risc_cpu_t *cpu, uint32_t instruction);

static inline uint32_t InstructionGetOpcode(uint32_t instruction);
static inline uint32_t InstructionGetRd(uint32_t instruction);
static inline uint32_t InstructionGetRs1(uint32_t instruction);
static inline uint32_t InstructionGetRs2(uint32_t instruction);
static inline uint32_t InstructionGetFunct3(uint32_t instruction);
static inline uint32_t InstructionGetFunct7(uint32_t instruction);
static inline uint32_t InstructionGetImmI(uint32_t instruction);
static inline uint32_t InstructionGetImmS(uint32_t instruction);
static inline uint32_t InstructionGetImmB(uint32_t instruction);
static inline uint32_t InstructionGetImmU(uint32_t instruction);
static inline uint32_t InstructionGetImmJ(uint32_t instruction);

int RiscCpuInitialize(risc_cpu_t *cpu, risc_memory_t *memory)
{
    if (memory == NULL)
    {
        RiscMemoryInitialize(memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    }

    memset(cpu->registers, 0, sizeof(uint32_t) * RISC_INSTRUCTION_32I_COUNT);
    cpu->pc = 0x00000000;

    cpu->memory = memory;

    for (size_t i = 0; i < RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT; i++)
    {
        cpu->instructionTypeHandler[i] = &RiscOpError;
    }
    
    cpu->instructionTypeHandler[0b01100] = &RiscOpArithmeticR;
    cpu->instructionTypeHandler[0b00100] = &RiscOpArithmeticI;
    cpu->instructionTypeHandler[0b00000] = &RiscOpLoad;
    cpu->instructionTypeHandler[0b01000] = &RiscOpStore;
    cpu->instructionTypeHandler[0b11000] = &RiscOpBranch;
    cpu->instructionTypeHandler[0b11011] = &RiscOpJumpJ;
    cpu->instructionTypeHandler[0b11001] = &RiscOpJumpI;
    cpu->instructionTypeHandler[0b01101] = &RiscOpUpperImm;
    cpu->instructionTypeHandler[0b11100] = &RiscOpSystem;

    return 0;
}

int RiscCpuDestroy(risc_cpu_t *cpu)
{
    RiscMemoryDestroy(cpu->memory);
    return 0;
}

int RiscCpuExecute(risc_cpu_t *cpu)
{
    cpu->registers[0] = 0;
    uint32_t data = 0;

    int status = RiscMemoryRead(cpu->memory, cpu->pc, DWORD, &data);

    uint8_t opcode = InstructionGetOpcode(data);

    cpu->instructionTypeHandler[opcode >> 2](cpu, data);

    cpu->pc += 4;

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
            reg[rd] = reg[rs1] >> reg[rs2];
            break;
        case 0b0100000101:
            // Shift right arithmetic
            reg[rd] = reg[rs1] >> reg[rs2];
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
        default:
            status = 1;
            break;
    }

    return status;
}

int RiscOpArithmeticI(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t funct7 = InstructionGetFunct7(instruction);
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t imm = InstructionGetImmI(instruction);

    uint32_t funct10 = funct7 << 3 | funct3;

    uint32_t *reg = &cpu->registers[0];

    int status = 0;

    switch (funct10)
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
            if ((int32_t) reg[rs1] < imm) 
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
            reg[rd] = reg[rs1] << imm;
            break;
        case 0b101:
            // Shift right logical (zero-extend)
            reg[rd] = reg[rs1] >> imm;
            break;
        case 0b100000101:
            // Shift right arithmetic (sign-extend)
            reg[rd] = reg[rs1] >> imm;
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
            reg[rd] = (int32_t) data;
            break;
        case 0b100:
            status = RiscMemoryRead(cpu->memory, address, BYTE, &data);
            reg[rd] = data;
            break;
        case 0b001:
            status = RiscMemoryRead(cpu->memory, address, WORD, &data);
            reg[rd] = (int32_t) data;
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
    uint32_t imm = InstructionGetImmI(instruction);
    uint32_t funct3 = InstructionGetFunct3(instruction);
    uint32_t rs1 = InstructionGetRs1(instruction);
    uint32_t rs2 = InstructionGetRs2(instruction);

    uint32_t *reg = &cpu->registers[0];

    uint32_t address = reg[rs1] + imm;

    int status = 0;

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
            if ((int32_t) reg[rs1] < (int32_t) reg[rs2])
            {
                cpu->pc += imm;
            }
            break;
        case 0b110:
            if (reg[rs1] < reg[rs2])
            {
                cpu->pc += imm;
            }
            break;
        case 0b101:
            if ((int32_t) reg[rs1] >= (int32_t) reg[rs2])
            {
                cpu->pc += imm;
            }
            break;
        case 0b111:
            if (reg[rs1] >= reg[rs2])
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

    reg[rd] = cpu->pc + 4;
    cpu->pc = reg[rs1] + imm;

    return 0;
}

int RiscOpUpperImm(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t imm = InstructionGetImmU(instruction);

    uint32_t *reg = &cpu->registers[0];

    

    reg[rd] = cpu->pc + (imm << 12); // TODO: Check if that is correct

    return 0;
}

int RiscOpSystem(risc_cpu_t *cpu, uint32_t instruction)
{
    uint32_t rd = InstructionGetRd(instruction);
    uint32_t imm = InstructionGetImmU(instruction);

    uint32_t *reg = &cpu->registers[0];

    reg[rd] = imm << 12; 

    return 0;
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
        (instruction & 0x80000000) >> 19 |  // imm[12]
        ((instruction >> 7) & 0x01) << 11 | // imm[11]
        ((instruction >> 25) & 0x3F) >> 5 | // imm[10:5]
        ((instruction >> 8) & 0x0F) >> 1    // imm[4:1]
    );

    return (uint32_t) imm;
}

uint32_t InstructionGetImmU(uint32_t instruction)
{
    return (int32_t)(instruction & 0xFFFFF999);
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
