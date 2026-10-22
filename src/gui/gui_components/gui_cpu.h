#ifndef GUI_CPU_H__
#define GUI_CPU_H__

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "risc/risc_cpu.h"

#define REGISTER_COUNT 32

typedef struct GuiCpuState {
    Rectangle bounds;
    Vector2 position;
    int width, height;
    int minWidth, minHeight;
    int padding;
    int fontSize;

    bool isWindowActive;
} GuiCpuState;

GuiCpuState InitGuiCpu(void);
void GuiCpu(GuiCpuState *state, const risc_cpu_t *cpu);

#endif // GUI_CPU_H__
