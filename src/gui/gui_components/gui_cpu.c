#include "gui_cpu.h"

#include <raygui.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

GuiCpuState InitGuiCpu(void)
{
    GuiCpuState state = { 
        .padding = 12,
        .fontSize = 20,
        .minHeight = 100,
        .minWidth = 100,
        .bounds.width = 100,
        .bounds.height = 100,
        .bounds.x = GetScreenWidth() / 2.0f,
        .bounds.y = GetScreenHeight() / 2.0f,
        .isWindowActive = true
    };
    
    return state;
}

void GuiCpu(GuiCpuState *state, const risc_cpu_t *cpu)
{
    (void)cpu;
    if (state->isWindowActive == false) return;

    if (GuiWindowBox(state->bounds, "CPU"))
    {
        state->isWindowActive = !state->isWindowActive;
    }

    for (size_t i = 0; i < REGISTER_COUNT; i++)
    {
        DrawRectangle(
            state->bounds.x + 4,
            state->bounds.y + 4 + (i * state->padding),
            36,
            24,
            BLACK
        );
    }
}
