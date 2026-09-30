#include "risc/risc-cpu.h"

#include <string.h>

int RiscCpuInitialize(risc_cpu_t *cpu)
{
    memset(cpu->registers, sizeof(uint32_t) * RISC_INSTRUCTION_32I_COUNT, 0);
    cpu->pc = 0x00000000;
}

int RiscCpuDestroy(risc_cpu_t *cpu)
{

}

int RiscCpuFetch(risc_cpu_t *cpu, uint32_t *instruction)
{

}

int RiscCpuExecute(risc_cpu_t *cpu, uint32_t op)
{

}

int RiscOpArithmeticR(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpArithmeticI(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpLoad(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpStore(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpBranch(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpJump(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpUpperImm(risc_cpu_t *cpu, uint32_t instruction)
{
    
}

int RiscOpSystem(risc_cpu_t *cpu, uint32_t instruction)
{
    
}
