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
    cpu->instructionTypeHandler[0b11011] = &RiscOpJump;
    cpu->instructionTypeHandler[0b11001] = &RiscOpJump;
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
    return 0;
}

int RiscOpArithmeticI(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpLoad(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpStore(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpBranch(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpJump(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpUpperImm(risc_cpu_t *cpu, uint32_t instruction)
{
    return 0;
}

int RiscOpSystem(risc_cpu_t *cpu, uint32_t instruction)
{
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
