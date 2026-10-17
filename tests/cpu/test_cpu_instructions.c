#include "unity.h"
#include "risc/risc_memory.h"
#include "risc/risc_cpu.h"
#include "unity_internals.h"

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

static void assert_type_r(uint32_t funct3, uint32_t funct7, uint32_t val1, uint32_t val2, uint32_t expected)
{
    cpu.registers[1] = val1;
    cpu.registers[2] = val2;
    uint32_t inst = RiscInstructionCreateR(0b110011, 3, funct3, 1, 2, funct7);
    
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));
    TEST_ASSERT_EQUAL_HEX32(expected, cpu.registers[3]);
}

static void assert_type_i(uint32_t funct3, uint32_t val1, int32_t imm, uint32_t expected)
{
    cpu.registers[1] = val1;

    uint32_t inst = RiscInstructionCreateI(0b010011, 2, funct3, 1, imm);
    TEST_ASSERT_EQUAL_INT(0, execute_instruction(inst));
    TEST_ASSERT_EQUAL_HEX32(expected, cpu.registers[2]);
}

void test_r_arithmetic(void)
{
    // Add
    assert_type_r(0b000, 0b0000000, 10, 20, 30);
    // Sub
    assert_type_r(0b000, 0b0100000, 50, 20, 30);
    // And
    assert_type_r(0b111, 0b0000000, 0b01100110, 0b11000000, 0b01000000);
    // Or
    assert_type_r(0b110, 0b0000000, 0b01100110, 0b11000000, 0b11100110);
    // Xor
    assert_type_r(0b100, 0b0000000, 0b01100110, 0b11000000, 0b10100110);
    // Shift left
    assert_type_r(0b001, 0b0000000, 0b00000001, 4, 0b00010000);
    // Shift right (logical)
    assert_type_r(0b101, 0b0000000, 0b00010000, 4, 0b00000001);
    // Shift right
    assert_type_r(0b101, 0b0100000, 0b00010000, 4, 0b00000001);
    // Less than (signed)
    assert_type_r(0b010, 0b0000000, 2, 3, 1); //TODO: Adjust for signed
    assert_type_r(0b010, 0b0000000, 3, 2, 0);
    // Less than (unsigned)
    assert_type_r(0b011, 0b0000000, 2, 3, 1); //TODO: Adjust for unsigned
    assert_type_r(0b011, 0b0000000, 3, 2, 0);
}

void test_i_arithmetic(void)
{
    // Add
    assert_type_i(0b000, 10, 20, 30);
    // And
    assert_type_i(0b111, 0b01100110, 0b11000000, 0b01000000);
    // Or
    assert_type_i(0b110, 0b01100110, 0b11000000, 0b11100110);
    // Xor
    assert_type_i(0b100, 0b01100110, 0b11000000, 0b10100110);
    // Less than (signed)
    assert_type_i(0b010, 2, 3, 1); 
    assert_type_i(0b010, 3, 2, 0); 
    // Less than (unsigned)
    assert_type_i(0b011, 2, 3, 1); 
    assert_type_i(0b011, 3,2, 0); 

    // Shift left
    assert_type_r(0b001, 0b0000000, 0b00000001, 4, 0b00010000);
    // Shift right (logical)
    assert_type_r(0b101, 0b0000000, 0b00010000, 4, 0b00000001);
    // Shift right
    assert_type_r(0b101, 0b0100000, 0b00010000, 4, 0b00000001);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_r_arithmetic);
    RUN_TEST(test_i_arithmetic);

    return UNITY_END();
}