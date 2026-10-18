#include "gui/main_gui.h"

#include "tools/file_reader.h"

int main(void)
{
    FileReaderLoadMemoryElf("/home/leonv/Projects/taketherisc/tests/riscv-tests/rv32ui-p-add", NULL);

    return 0;
}