#ifndef GUI_CPU_H__
#define GUI_CPU_H__

#include <raylib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct GuiCpuState {
    Rectangle bounds;
    Vector2 position;
    int width, height;
    int minWidth, minHeight;
    int padding;
    int fontSize;

    const char *registerLabel;

    char registerValues[32][16];

    int registerLabelWidth, registerLabelHeight;
} GuiCpuState;

static void GuiCpuRefresh(GuiCpuState *state);

#endif // GUI_CPU_H__

#ifdef GUI_CPU_IMPLEMENTATION

#include <raygui.h>
#include <string.h>

GuiCpuState InitGuiCpu(void)
{
    GuiCpuState state = { 0 };

    state.padding = 12;
    state.fontSize = 20;
    state.minHeight = 100;
    state.minWidth = 100;

    state.registerLabelWidth = 100;
    state.registerLabelHeight = 20;

    const char *initText = "0x00";

    for (size_t i = 0; i < 32; i++)
    {
        strcpy(state.registerValues[i], initText);
    }

    state.bounds.width = 100;
    state.bounds.height = 100;
    state.bounds.x = GetScreenWidth() / 2.0;
    state.bounds.y = GetScreenHeight() / 2.0;
    
    return state;
}

void GuiCpu(GuiCpuState *state)
{
    GuiWindowBox(state->bounds, "CPU");
}

#endif // GUI_CPU_IMPLEMENTATION