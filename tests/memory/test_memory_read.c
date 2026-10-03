#include "unity.h"
#include "risc/risc-memory.h"

void setUp(void)
{

}

void tearDown(void)
{

}

void test_memory_read_byte_little_endian(void)
{
    uint8_t rawMemory[] = { 0x12 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0;
    
    int status = RiscMemoryRead(&memory, 0, BYTE, &value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX32(0x12, value);
}

void test_memory_read_word_little_endian(void)
{
    uint8_t rawMemory[] = { 0x34, 0x12 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0;
    
    int status = RiscMemoryRead(&memory, 0, WORD, &value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX32(0x1234, value);
}

void test_memory_read_dword_little_endian(void)
{
    uint8_t rawMemory[] = { 0x78, 0x56, 0x34, 0x12 };

    risc_memory_t memory = {
        .data = rawMemory,
        .size = sizeof(rawMemory),
        .base = 0x00000000
    };

    uint32_t value = 0;

    int status = RiscMemoryRead(&memory, 0, DWORD, &value);

    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, value);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_memory_read_byte_little_endian);
    RUN_TEST(test_memory_read_word_little_endian);
    RUN_TEST(test_memory_read_dword_little_endian);

    return UNITY_END();
}