#include "risc/risc-cpu.h"
#include "risc/risc-memory.h"

#include <string.h>

int RiscCpuInitialize(risc_cpu_t *cpu, risc_memory_t *memory)
{
    if (memory == NULL)
    {
        RiscMemoryInitialize(memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    }

    memset(cpu->registers, 0, sizeof(uint32_t) * RISC_INSTRUCTION_32I_COUNT);
    cpu->pc = 0x00000000;

    cpu->memory = memory;

    return 0;
}

int RiscCpuDestroy(risc_cpu_t *cpu)
{
    RiscMemoryDestroy(cpu->memory);
    return 0;
}

int RiscCpuFetch(risc_cpu_t *cpu, uint32_t *instruction)
{
    return 0;
}

int RiscCpuExecute(risc_cpu_t *cpu, uint32_t op)
{
    return 0;
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
