#ifndef GUI_LOADED_PROGRAM_H__
#define GUI_LOADED_PROGRAM_H__

#include <raylib.h>
#include <stdint.h>
#include <stdbool.h>
#include "risc/risc_cpu.h"

#define MAX_BREAKPOINTS 32

typedef struct GuiBreakpoint
{
    bool isActive;
    uint32_t address;
} GuiBreakpoint;

typedef struct GuiLoadedProgramState
{
    Rectangle bounds;
    Vector2 scroll;
    Rectangle view;
    bool isWindowActive;

    uint32_t startAddress;
    uint32_t instructionCount;
    int lineHeight;
    int fontSize;
    bool followPC;

    char searchText[32];
    bool searchEditMode;

    GuiBreakpoint breakpoints[MAX_BREAKPOINTS];
    int breakpointCount;
} GuiLoadedProgramState;

GuiLoadedProgramState InitGuiLoadedProgram(void);
void GuiLoadedProgram(GuiLoadedProgramState *state, const risc_cpu_t *cpu);

#endif // GUI_LOADED_PROGRAM_H__
