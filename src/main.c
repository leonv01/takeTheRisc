#include "gui/main_gui.h"

#include "risc/risc_cpu.h"
#include "risc/risc_memory.h"
#include "tools/file_reader.h"
#include "tools/logger.h"
#include <stdio.h>

int main(void)
{
    GuiInitialize(1280, 720, "Take the RISC-V");
    GuiRun();
    GuiDestroy();
}