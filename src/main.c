#include <assert.h>
#include <stdio.h>

#include "risc/risc-cpu.h"
#include "risc/risc-memory.h"

int main(void)
{
    risc_cpu_t cpu;
    risc_memory_t memory;

    const uint32_t testValue = 0x12345678;

    RiscMemoryInitialize(&memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    RiscCpuInitialize(&cpu, &memory);

    RiscCpuExecute(&cpu);

    return 0;
}