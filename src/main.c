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

    uint32_t data =  RiscInstructionCreateR(
        0b0110011,
        1,
        0b000,
        2,
        3,
        0b00000000
    );

    cpu.registers[2] = 4;
    cpu.registers[3] = 5;

    RiscMemoryWrite(&memory, 0, DWORD, data);

    RiscCpuExecute(&cpu);

    printf("%d\n", cpu.registers[1]);

    return 0;
}