#include "gui/main_gui.h"

int main(void)
{
    GuiInitialize(1280, 720, "Take the RISC-V");
    GuiRun();
    GuiDestroy();
}