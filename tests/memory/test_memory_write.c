#include "unity.h"
#include "risc/risc-memory.h"

void setUp(void)
{

}

void tearDown(void)
{

}

void test_memory_write_byte_little_endian(void)
{
    uint8_t rawMemory[4] = { 0 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0x12345678;
    
    int status = RiscMemoryWrite(&memory, 0, BYTE, value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX8(0x78, rawMemory[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, rawMemory[1]);
}

void test_memory_write_word_little_endian(void)
{
    uint8_t rawMemory[4] = { 0 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0x12345678;
    
    int status = RiscMemoryWrite(&memory, 0, WORD, value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX8(0x78, rawMemory[0]);
    TEST_ASSERT_EQUAL_HEX8(0x56, rawMemory[1]);
    TEST_ASSERT_EQUAL_HEX8(0x00, rawMemory[2]);
}

void test_memory_write_dword_little_endian(void)
{
    uint8_t rawMemory[4] = { 0 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0x12345678;
    
    int status = RiscMemoryWrite(&memory, 0, DWORD, value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX8(0x78, rawMemory[0]);
    TEST_ASSERT_EQUAL_HEX8(0x56, rawMemory[1]);
    TEST_ASSERT_EQUAL_HEX8(0x34, rawMemory[2]);
    TEST_ASSERT_EQUAL_HEX8(0x12, rawMemory[3]);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_memory_write_byte_little_endian);
    RUN_TEST(test_memory_write_word_little_endian);
    RUN_TEST(test_memory_write_dword_little_endian);

    return UNITY_END();
}