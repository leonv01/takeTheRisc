#include "tools/file_reader.h"
#include "unity.h"
#include "risc/risc_memory.h"
#include "risc/risc_cpu.h"
#include "unity_internals.h"

#include <stdbool.h>

#define MAX_CYCLES 10000

#define RISC_EXTENSION_M

static risc_cpu_t cpu;
static risc_memory_t memory;

void setUp(void)
{
    RiscMemoryInitialize(
        &memory, 
        RISC_MEMORY_DEFAULT_BASE, 
        RISC_MEMORY_DEFAULT_SIZE
    );
    RiscCpuInitialize(&cpu, &memory);
}

void tearDown(void)
{
    RiscCpuDestroy(&cpu);
}

/* -------------------------------------------------------------------------- */

static void run_riscv_test(const char *file)
{
    FileReaderLoadMemoryElf(file, &cpu);

    size_t cycles = 0;

    while (cpu.halted == false && cycles < MAX_CYCLES)
    {
        int status = RiscCpuExecute(&cpu);

        TEST_ASSERT_EQUAL_INT(0, status);

        cycles++;
    }

    TEST_ASSERT_LESS_THAN_INT(MAX_CYCLES, cycles);

    TEST_ASSERT_EQUAL_HEX32(0x01, cpu.registers[RISC_ABI_GP]);
}

/* -------------------------------------------------------------------------- */

void test_riscv32(const char *testcase)
{
    char buffer[128];
    const char *path = "tests/riscv-tests";
    
    snprintf(buffer, sizeof(buffer), "%s/%s", path, testcase);

    run_riscv_test(buffer);
}

void test_rv32ui_p_add(void) { test_riscv32("rv32ui-p-add"); }
void test_rv32ui_p_addi(void) { test_riscv32("rv32ui-p-addi"); }
void test_rv32ui_p_and(void) { test_riscv32("rv32ui-p-and"); }
void test_rv32ui_p_andi(void) { test_riscv32("rv32ui-p-andi"); }
void test_rv32ui_p_auipc(void) { test_riscv32("rv32ui-p-auipc"); }
void test_rv32ui_p_beq(void) { test_riscv32("rv32ui-p-beq"); }
void test_rv32ui_p_bge(void) { test_riscv32("rv32ui-p-bge"); }
void test_rv32ui_p_bgeu(void) { test_riscv32("rv32ui-p-bgeu"); }
void test_rv32ui_p_blt(void) { test_riscv32("rv32ui-p-blt"); }
void test_rv32ui_p_bltu(void) { test_riscv32("rv32ui-p-bltu"); }
void test_rv32ui_p_bne(void) { test_riscv32("rv32ui-p-bne"); }
void test_rv32ui_p_fence_i(void) { test_riscv32("rv32ui-p-fence_i"); }
void test_rv32ui_p_jal(void) { test_riscv32("rv32ui-p-jal"); }
void test_rv32ui_p_jalr(void) { test_riscv32("rv32ui-p-jalr"); }
void test_rv32ui_p_lb(void) { test_riscv32("rv32ui-p-lb"); }
void test_rv32ui_p_lbu(void) { test_riscv32("rv32ui-p-lbu"); }
void test_rv32ui_p_ld_st(void) { test_riscv32("rv32ui-p-ld_st"); }
void test_rv32ui_p_lh(void) { test_riscv32("rv32ui-p-lh"); }
void test_rv32ui_p_lhu(void) { test_riscv32("rv32ui-p-lhu"); }
void test_rv32ui_p_lui(void) { test_riscv32("rv32ui-p-lui"); }
void test_rv32ui_p_lw(void) { test_riscv32("rv32ui-p-lw"); }
void test_rv32ui_p_ma_data(void) { test_riscv32("rv32ui-p-ma_data"); }
void test_rv32ui_p_or(void) { test_riscv32("rv32ui-p-or"); }
void test_rv32ui_p_ori(void) { test_riscv32("rv32ui-p-ori"); }
void test_rv32ui_p_sb(void) { test_riscv32("rv32ui-p-sb"); }
void test_rv32ui_p_sh(void) { test_riscv32("rv32ui-p-sh"); }
void test_rv32ui_p_simple(void) { test_riscv32("rv32ui-p-simple"); }
void test_rv32ui_p_sll(void) { test_riscv32("rv32ui-p-sll"); }
void test_rv32ui_p_slli(void) { test_riscv32("rv32ui-p-slli"); }
void test_rv32ui_p_slt(void) { test_riscv32("rv32ui-p-slt"); }
void test_rv32ui_p_slti(void) { test_riscv32("rv32ui-p-slti"); }
void test_rv32ui_p_sltiu(void) { test_riscv32("rv32ui-p-sltiu"); }
void test_rv32ui_p_sltu(void) { test_riscv32("rv32ui-p-sltu"); }
void test_rv32ui_p_sra(void) { test_riscv32("rv32ui-p-sra"); }
void test_rv32ui_p_srai(void) { test_riscv32("rv32ui-p-srai"); }
void test_rv32ui_p_srl(void) { test_riscv32("rv32ui-p-srl"); }
void test_rv32ui_p_srli(void) { test_riscv32("rv32ui-p-srli"); }
void test_rv32ui_p_st_ld(void) { test_riscv32("rv32ui-p-st_ld"); }
void test_rv32ui_p_sub(void) { test_riscv32("rv32ui-p-sub"); }
void test_rv32ui_p_sw(void) { test_riscv32("rv32ui-p-sw"); }
void test_rv32ui_p_xor(void) { test_riscv32("rv32ui-p-xor"); }
void test_rv32ui_p_xori(void) { test_riscv32("rv32ui-p-xori"); }

#ifdef RISC_EXTENSION_M
void test_rv32um_p_div(void) { test_riscv32("rv32um-p-div"); }
void test_rv32um_p_divu(void) { test_riscv32("rv32um-p-divu"); }
void test_rv32um_p_mul(void) { test_riscv32("rv32um-p-mul"); }
void test_rv32um_p_mulh(void) { test_riscv32("rv32um-p-mulh"); }
void test_rv32um_p_mulhsu(void) { test_riscv32("rv32um-p-mulhsu"); }
void test_rv32um_p_mulhu(void) { test_riscv32("rv32um-p-mulhu"); }
void test_rv32um_p_rem(void) { test_riscv32("rv32um-p-rem"); }
void test_rv32um_p_remu(void) { test_riscv32("rv32um-p-remu"); }
#endif // RISC_EXTENSION_M

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_rv32ui_p_add);
    RUN_TEST(test_rv32ui_p_addi);
    RUN_TEST(test_rv32ui_p_and);
    RUN_TEST(test_rv32ui_p_andi);
    RUN_TEST(test_rv32ui_p_auipc);
    RUN_TEST(test_rv32ui_p_beq);
    RUN_TEST(test_rv32ui_p_bge);
    RUN_TEST(test_rv32ui_p_bgeu);
    RUN_TEST(test_rv32ui_p_blt);
    RUN_TEST(test_rv32ui_p_bltu);
    RUN_TEST(test_rv32ui_p_bne);
    RUN_TEST(test_rv32ui_p_fence_i);
    RUN_TEST(test_rv32ui_p_jal);
    RUN_TEST(test_rv32ui_p_jalr);
    RUN_TEST(test_rv32ui_p_lb);
    RUN_TEST(test_rv32ui_p_lbu);
    RUN_TEST(test_rv32ui_p_ld_st);
    RUN_TEST(test_rv32ui_p_lh);
    RUN_TEST(test_rv32ui_p_lhu);
    RUN_TEST(test_rv32ui_p_lui);
    RUN_TEST(test_rv32ui_p_lw);
    RUN_TEST(test_rv32ui_p_ma_data);
    RUN_TEST(test_rv32ui_p_or);
    RUN_TEST(test_rv32ui_p_ori);
    RUN_TEST(test_rv32ui_p_sb);
    RUN_TEST(test_rv32ui_p_sh);
    RUN_TEST(test_rv32ui_p_simple);
    RUN_TEST(test_rv32ui_p_sll);
    RUN_TEST(test_rv32ui_p_slli);
    RUN_TEST(test_rv32ui_p_slt);
    RUN_TEST(test_rv32ui_p_slti);
    RUN_TEST(test_rv32ui_p_sltiu);
    RUN_TEST(test_rv32ui_p_sltu);
    RUN_TEST(test_rv32ui_p_sra);
    RUN_TEST(test_rv32ui_p_srai);
    RUN_TEST(test_rv32ui_p_srl);
    RUN_TEST(test_rv32ui_p_srli);
    RUN_TEST(test_rv32ui_p_st_ld);
    RUN_TEST(test_rv32ui_p_sub);
    RUN_TEST(test_rv32ui_p_sw);
    RUN_TEST(test_rv32ui_p_xor);
    RUN_TEST(test_rv32ui_p_xori);
    
    #ifdef RISC_EXTENSION_M
    RUN_TEST(test_rv32um_p_div);
    RUN_TEST(test_rv32um_p_divu);
    RUN_TEST(test_rv32um_p_mul);
    RUN_TEST(test_rv32um_p_mulh);
    RUN_TEST(test_rv32um_p_mulhsu);
    RUN_TEST(test_rv32um_p_mulhu);
    RUN_TEST(test_rv32um_p_rem);
    RUN_TEST(test_rv32um_p_remu);
    #endif // RISC_EXTENSION_M
    UNITY_END();
}