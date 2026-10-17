#include "unity.h"
#include "risc/risc_memory.h"
#include "risc/risc_cpu.h"
#include "unity_internals.h"

#include <stdbool.h>

static risc_cpu_t cpu;
static risc_memory_t memory;

void setUp(void)
{
    RiscMemoryInitialize(&memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    RiscCpuInitialize(&cpu, &memory);
}

void tearDown(void)
{
    RiscCpuDestroy(&cpu);
}

static int execute_instruction(uint32_t instruction)
{
    RiscMemoryWrite(cpu.memory, cpu.pc, DWORD, instruction);
    return RiscCpuExecute(&cpu);
}

static void assert_type_r(uint32_t opcode, uint32_t funct3, uint32_t funct7, uint32_t val1, uint32_t val2, uint32_t expected)
{
    cpu.registers[1] = val1;
    cpu.registers[2] = val2;
    uint32_t inst = RiscInstructionCreateR(opcode, 3, funct3, 1, 2, funct7);
    
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));
    TEST_ASSERT_EQUAL_HEX32(expected, cpu.registers[3]);
}

static void assert_type_i(uint32_t opcode, uint32_t funct3, uint32_t val1, int32_t imm, uint32_t expected)
{
    cpu.registers[1] = val1;

    uint32_t inst = RiscInstructionCreateI(opcode, 2, funct3, 1, imm);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));
    TEST_ASSERT_EQUAL_HEX32(expected, cpu.registers[2]);
}

static void assert_load_i(uint32_t funct3, uint32_t base_addr, uint32_t offset, uint32_t test_data, DATA_SIZE size, uint32_t expected)
{
    uint32_t target_addr = base_addr + offset;
    RiscMemoryWrite(cpu.memory, target_addr, size, test_data);

    cpu.registers[1] = base_addr;

    uint32_t inst = RiscInstructionCreateI(0b0000011, 2, funct3, 1, offset);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));
    TEST_ASSERT_EQUAL_HEX32(expected, cpu.registers[2]);
}

static void assert_store_s(uint32_t funct3, uint32_t base_addr, uint32_t offset, uint32_t test_data, DATA_SIZE size, uint32_t expected)
{
    uint32_t target_addr = base_addr + offset;

    cpu.registers[1] = base_addr;
    cpu.registers[2] = test_data;

    uint32_t inst = RiscInstructionCreateS(0b0100011, funct3, 1, 2, offset);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));

    uint32_t stored_memory = 0;
    RiscMemoryRead(cpu.memory, target_addr, size, &stored_memory);
    TEST_ASSERT_EQUAL_HEX32(expected, stored_memory);
}

static void assert_branch_b(uint32_t funct3, uint32_t val1, uint32_t val2, int32_t offset, bool expect_taken)
{
    uint32_t start_pc = 0x1000;
    cpu.pc = start_pc;

    cpu.registers[1] = val1;
    cpu.registers[2] = val2;

    uint32_t inst = RiscInstructionCreateB(0b1100011, funct3, 1, 2, offset);
    
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));

    uint32_t expected_pc = expect_taken ? (start_pc + offset) : (start_pc + 4);
    TEST_ASSERT_EQUAL_HEX32(expected_pc, cpu.pc);
}

static void assert_jump_j(uint32_t offset)
{
    uint32_t start_pc = 0x1000;
    cpu.pc = start_pc;
    cpu.registers[1] = 0;

    uint32_t inst = RiscInstructionCreateJ(0b1101111, 1, offset);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));

    TEST_ASSERT_EQUAL_HEX32(start_pc + offset, cpu.pc);

    TEST_ASSERT_EQUAL_HEX32(start_pc + 4, cpu.registers[1]);
}

static void assert_jump_i(uint32_t base_addr, uint32_t offset)
{
    uint32_t start_pc = 0x1000;
    cpu.pc = start_pc;
    cpu.registers[1] = 0;
    cpu.registers[2] = base_addr;

    uint32_t inst = RiscInstructionCreateI(0b1100111, 1, 0b000, 2, offset);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));

    TEST_ASSERT_EQUAL_HEX32(base_addr + offset, cpu.pc);
    TEST_ASSERT_EQUAL_HEX32(start_pc + 4, cpu.registers[1]);
}

/* -------------------------------------------------------------------------- */

void test_r_arithmetic(void)
{
    // Add
    assert_type_r(0b110011, 0b000, 0b0000000, 10, 20, 30);
    // Sub
    assert_type_r(0b110011, 0b000, 0b0100000, 50, 20, 30);
    // And
    assert_type_r(0b110011, 0b111, 0b0000000, 0b01100110, 0b11000000, 0b01000000);
    // Or
    assert_type_r(0b110011, 0b110, 0b0000000, 0b01100110, 0b11000000, 0b11100110);
    // Xor
    assert_type_r(0b110011, 0b100, 0b0000000, 0b01100110, 0b11000000, 0b10100110);
    // Shift left
    assert_type_r(0b110011, 0b001, 0b0000000, 0b00000001, 4, 0b00010000);
    // Shift right (logical)
    assert_type_r(0b110011, 0b101, 0b0000000, 0b00010000, 4, 0b00000001);
    // Shift right
    assert_type_r(0b110011, 0b101, 0b0100000, 0b00010000, 4, 0b00000001);
    // Less than (signed)
    assert_type_r(0b110011, 0b010, 0b0000000, 2, 3, 1); //TODO: Adjust for signed
    assert_type_r(0b110011, 0b010, 0b0000000, 3, 2, 0);
    // Less than (unsigned)
    assert_type_r(0b110011, 0b011, 0b0000000, 2, 3, 1); //TODO: Adjust for unsigned
    assert_type_r(0b110011, 0b011, 0b0000000, 3, 2, 0);
}

void test_i_arithmetic(void)
{
    // Add
    assert_type_i(0b010011, 0b000, 10, 20, 30);
    // And
    assert_type_i(0b010011, 0b111, 0b01100110, 0b11000000, 0b01000000);
    // Or
    assert_type_i(0b010011, 0b110, 0b01100110, 0b11000000, 0b11100110);
    // Xor
    assert_type_i(0b010011, 0b100, 0b01100110, 0b11000000, 0b10100110);
    // Less than (signed)
    assert_type_i(0b010011, 0b010, 2, 3, 1); 
    assert_type_i(0b010011, 0b010, 3, 2, 0); 
    // Less than (unsigned)
    assert_type_i(0b010011, 0b011, 2, 3, 1); 
    assert_type_i(0b010011, 0b011, 3,2, 0); 

    // Shift left
    assert_type_r(0b110011, 0b001, 0b0000000, 0b00000001, 4, 0b00010000);
    // Shift right (logical)
    assert_type_r(0b110011, 0b101, 0b0000000, 0b00010000, 4, 0b00000001);
    // Shift right
    assert_type_r(0b110011, 0b101, 0b0100000, 0b00010000, 4, 0b00000001);
}

void test_i_load(void)
{
    // Load byte (sign-extend)
    assert_load_i(0b000, 0x100, 4, 127, BYTE, 0x0000007F);
    assert_load_i(0b000, 0x100, 4, -128, BYTE, 0xFFFFFF80);
    // Load byte (zero-extend)
    assert_load_i(0b100, 0x100, 4, 127, BYTE, 0x0000007F);
    assert_load_i(0b100, 0x100, 4, -128, BYTE, 0x00000080);
    // Load word (sign-extend)
    assert_load_i(0b001, 0x100, 4, 1023, WORD, 0x000003FF);
    assert_load_i(0b001, 0x100, 4, -1024, WORD, 0xFFFFFC00);
    // Load word (zero-extend)
    assert_load_i(0b101, 0x100, 4, 1023, WORD, 0x000003FF);
    assert_load_i(0b101, 0x100, 4, -1024, WORD, 0x0000FC00);
    // Load dword
    assert_load_i(0b010, 0x100, 4, 0x1FFFFFFF, DWORD, 0x1FFFFFFF);
    assert_load_i(0b010, 0x100, 4, 0x9FFFFFFF, DWORD, 0x9FFFFFFF);
}

void test_i_store(void)
{
    // Store byte
    assert_store_s(0b000, 0x100, 4, 0x1234567F, BYTE, 0x7F);
    assert_store_s(0b000, 0x100, 4, 0x12345680, BYTE, 0x80);

    // Store word
    assert_store_s(0b001, 0x100, 4, 0x1234BEEF, WORD, 0xBEEF);

    // Store dword
    assert_store_s(0b010, 0x100, 4, 0xDEADBEEF, DWORD, 0xDEADBEEF);
}

void test_b_branch(void)
{
    // Branch if equal
    assert_branch_b(0b000, 42, 42, 0x124, true);
    assert_branch_b(0b000, 42, 41, 0x124, false);

    // Branch if not equal
    assert_branch_b(0b001, 42, 41, 0x124, true);
    assert_branch_b(0b001, 42, 42, 0x124, false);

    // Branch if less than (signed)
    assert_branch_b(0b100, -5, 42, 0x124, true);
    assert_branch_b(0b100, 42, 42, 0x124, false);
    assert_branch_b(0b100, -42, -42, 0x124, false);
    assert_branch_b(0b100, 43, -5, 0x124, false);

    // Branch if less than (unsigned)
    assert_branch_b(0b110, 5,-42, 0x124, true);
    assert_branch_b(0b110, 42, 42, 0x124, false);
    assert_branch_b(0b110, -42, -42, 0x124, false);
    assert_branch_b(0b110, -43, 42, 0x124, false);

    // Branch if greater or equal (signed)
    assert_branch_b(0b101, 5, -42, 0x124, true);
    assert_branch_b(0b101, 42, 42, 0x124, true);
    assert_branch_b(0b101, -42, -42, 0x124, true);
    assert_branch_b(0b101, -43, 5, 0x124, false);

    // Branch if greater or equal (unsigned)
    assert_branch_b(0b111, -42,5, 0x124, true);
    assert_branch_b(0b111, 42, 42, 0x124, true);
    assert_branch_b(0b111, -42, -42, 0x124, true);
    assert_branch_b(0b111, 41, 42, 0x124, false);
}

void test_j_jump(void)
{
    assert_jump_j(0x124);
    assert_jump_j(-0x124);
    assert_jump_j(0x1824);
    assert_jump_j(0x04);
}

void test_i_jump(void)
{
    assert_jump_i(0x2000, 0x40);
    assert_jump_i(0x2000, -0x40);
    assert_jump_i(0x3000, -0x00);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_r_arithmetic);
    RUN_TEST(test_i_arithmetic);
    RUN_TEST(test_i_load);
    RUN_TEST(test_i_store);
    RUN_TEST(test_b_branch);
    RUN_TEST(test_j_jump);
    RUN_TEST(test_i_jump);

    return UNITY_END();
}