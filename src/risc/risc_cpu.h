#ifndef RISC_CPU_H__
#define RISC_CPU_H__

#include <stdint.h>

#include "risc/risc_memory.h"

#define RISC_INSTRUCTION_32I_COUNT 32
#define RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT 32

struct risc_cpu_t;

typedef int (*InstructionTypeHandler)(struct risc_cpu_t *, uint32_t);

typedef struct risc_cpu_t
{
    uint32_t registers[RISC_INSTRUCTION_32I_COUNT];
    uint32_t pc;

    InstructionTypeHandler instructionTypeHandler[RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT];

    risc_memory_t *memory;
} risc_cpu_t;

/* -------------------------------------------------------------------------- */
/*                              Struct operations                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initializes CPU
 * 
 * @param cpu 
 * @return int status code
 */
int RiscCpuInitialize(risc_cpu_t *cpu, risc_memory_t *memory);

/**
 * @brief Destroys CPU and frees memory
 * 
 * @param cpu 
 * @return int status code
 */
int RiscCpuDestroy(risc_cpu_t *cpu);

/**
 * @brief Executes next instruction
 * 
 * @param cpu 
 * @return int status code
 */
int RiscCpuExecute(risc_cpu_t *cpu);

/* -------------------------------------------------------------------------- */
/*                           Instruction operations                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Arithmetic operations (Register-Register)
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpArithmeticR(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief Arithetic operations (Immediate)
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpArithmeticI(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief Load operations
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpLoad(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief Store operations
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpStore(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief Branch operations
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpBranch(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief Jump and link operation
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpJumpJ(risc_cpu_t *cpu, uint32_t instruction);

/**
 * @brief Jump and link register operation
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpJumpI(risc_cpu_t *cpu, uint32_t instruction);

/**
 * @brief Upper immediate operations
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpUpperImm(risc_cpu_t *cpu, uint32_t instruction);
/**
 * @brief System operations
 * 
 * @param cpu 
 * @param instruction 
 * @return int status code
 */
int RiscOpSystem(risc_cpu_t *cpu, uint32_t instruction);

uint32_t RiscInstructionCreateR(
    uint32_t opcode, 
    uint32_t rd, 
    uint32_t funct3, 
    uint32_t rs1, 
    uint32_t rs2, 
    uint32_t funct7
);


#endif // RISC_CPU_H__
