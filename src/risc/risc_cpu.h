#ifndef RISC_CPU_H__
#define RISC_CPU_H__

#include <stdint.h>
#include <stdbool.h>

#include "risc/risc_memory.h"

#define RISC_REGISTER_COUNT 32
#define RISC_CSR_COUNT 4096
#define RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT 32

#define RISCV_EXTENSION_M
#define RISCV_EXTENSION_A
#define RISCV_EXTENSION_ZICSR
#define RISCV_XLEN 32

#define RISC_ADDRESS_TOHOST 0x80001000

struct risc_cpu_t;

typedef int (*InstructionTypeHandler)(struct risc_cpu_t *, uint32_t);

typedef enum RISC_TRAP_TYPE
{
    RISC_TRAP_INTERRUPT,
    RISC_TRAP_EXCEPTION
} RISC_TRAP_TYPE;

typedef enum RISC_TRAP_CAUSE
{
    RISC_TRAP_INSTRUCTION_ADDRESS_MISALIGNED    = 0,
    RISC_TRAP_INSTRUCTION_ACCESS_FAULT          = 1,
    RISC_TRAP_ILLEGAL_INSTRUCTION               = 2,
    RISC_TRAP_BREAKPOINT                        = 3,
    RISC_TRAP_LOAD_ADDRESS_MISALIGNED           = 4,
    RISC_TRAP_LOAD_ACCESS_FAULT                 = 5,
    RISC_TRAP_STORE_AMO_ADDRESS_MISALIGNED      = 6,
    RISC_TRAP_STORE_AMO_ACCESS_FAULT            = 7,
    RISC_TRAP_ECALL_U_MODE                      = 8,
    RISC_TRAP_ECALL_S_MODE                      = 9,
    RISC_TRAP_ECALL_M_MODE                      = 11,
    RISC_TRAP_ECALL_INSTRUCTION_PAGE_FAULT      = 12,
    RISC_TRAP_ECALL_LOAD_PAGE_FAULT             = 13,
    RISC_TRAP_STORE_AMO_PAGE_FAULT              = 15,
    RISC_TRAP_DOUBLE_TRAP                       = 16,
    RISC_TRAP_SOFTWARE_CHECK                    = 18,
    RISC_TRAP_HARDWARE_ERROR                    = 19
} RISC_TRAP_CAUSE;

typedef enum RISC_ABI
{
    RISC_ABI_ZERO = 0,
    RISC_ABI_RA = 1,
    RISC_ABI_SP = 2,
    RISC_ABI_GP = 3,
    RISC_ABI_TP = 4
} RISC_ABI;

typedef enum RISC_PRIVILEGE_MODE
{
    RISC_PRIVILEGE_U = 0b00,
    RISC_PRIVILEGE_S = 0b01,
    RISC_PRIVILEGE_R = 0b10,
    RISC_PRIVILEGE_M = 0b11
} RISC_PRIVILEGE_MODE;

typedef enum RISC_CSR_REGISTERS
{
    // Supervisor Level CSR
    RISC_CSR_SSTATUS        = 0x100,
    RISC_CSR_SIE            = 0x104,
    RISC_CSR_STVEC          = 0x105,
    RISC_CSR_SCOUNTEREN     = 0x106,
    RISC_CSR_SENVCFG        = 0x10A,
    RISC_CSR_SCOUNTINHIBIT  = 0x120,
    RISC_CSR_SSCRATCH       = 0x140,
    RISC_CSR_SEPC           = 0x141,
    RISC_CSR_SCAUSE         = 0x142,
    RISC_CSR_STVAL          = 0x143,
    RISC_CSR_SIP            = 0x144,
    RISC_CSR_SCOUNTOVF      = 0xDA0,

    // Machine Information Registers
    RISC_CSR_MVENDORID  = 0xF11,
    RISC_CSR_MARCHID    = 0xF12,
    RISC_CSR_MIMPID     = 0xF13,
    RISC_CSR_MHARTID    = 0xF14,
    RISC_CSR_MCONFIGPTR = 0xF15,

    // Machine Trap Setup
    RISC_CSR_MSTATUS    = 0x300,
    RISC_CSR_MISA       = 0x301,
    RISC_CSR_MEDELEG    = 0x302,
    RISC_CSR_MIDELEG    = 0x303,
    RISC_CSR_MIE        = 0x304,
    RISC_CSR_MTVEC      = 0x305,
    RISC_CSR_MCOUNTEREN = 0x306,
    RISC_CSR_MSTATUSH   = 0x310,
    RISC_CSR_MEDELEGH   = 0x312,

    // Machine Trap Handling
    RISC_CSR_MSCRATCH   = 0x340,
    RISC_CSR_MEPC       = 0x341,
    RISC_CSR_MCAUSE     = 0x342,
    RISC_CSR_MTVAL      = 0x343,
    RISC_CSR_MIP        = 0x344,
    RISC_CSR_MIINST     = 0x34A,
    RISC_CSR_MTVAL2     = 0x34B
} RISC_CSR_REGISTERS;

typedef struct risc_cpu_t
{
    uint32_t registers[RISC_REGISTER_COUNT];
    uint32_t csrs[RISC_CSR_COUNT];
    uint32_t pc;

    InstructionTypeHandler instructionTypeHandler[RISC_INSTRUCTION_HANDLER_COUNT_32I_COUNT];

    risc_memory_t *memory;

    bool halted;

    uint32_t reservationAddress;
    bool reservationValid;

    RISC_PRIVILEGE_MODE privilegeMode;
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

int RiscOpAmo(risc_cpu_t *cpu, uint32_t instruction);

void RiscEnterTrap(risc_cpu_t *cpu, RISC_TRAP_CAUSE trapCause, RISC_TRAP_TYPE trapType);

uint32_t RiscInstructionCreateR(
    uint32_t opcode, 
    uint32_t rd, 
    uint32_t funct3, 
    uint32_t rs1, 
    uint32_t rs2, 
    uint32_t funct7
);

uint32_t RiscInstructionCreateI(
    uint32_t opcode,
    uint32_t rd,
    uint32_t funct3,
    uint32_t rs1,
    uint32_t imm
);

uint32_t RiscInstructionCreateS(
    uint32_t opcode, 
    uint32_t funct3, 
    uint32_t rs1, 
    uint32_t rs2,
    uint32_t imm 
);

uint32_t RiscInstructionCreateB(
    uint32_t opcode,
    uint32_t funct3,
    uint32_t rs1,
    uint32_t rs2,
    uint32_t imm
);

uint32_t RiscInstructionCreateJ(
    uint32_t opcode,
    uint32_t rd,
    uint32_t imm
);

uint32_t RiscInstructionCreateU(
    uint32_t opcode, 
    uint32_t rd, 
    uint32_t imm
);

#endif // RISC_CPU_H__
