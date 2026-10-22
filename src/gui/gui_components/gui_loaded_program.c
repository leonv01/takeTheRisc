#include "gui_loaded_program.h"
#include "risc/risc_memory.h"

#include <raygui.h>
#include <stdio.h>
#include <stdlib.h>

static void RemoveBreakpoint(GuiLoadedProgramState *state, uint32_t addr) __attribute__((unused));
static void AddBreakpoint(GuiLoadedProgramState *state, uint32_t addr) __attribute__((unused));

GuiLoadedProgramState InitGuiLoadedProgram(void)
{
    GuiLoadedProgramState state = {
        .bounds = { 20, 60, 480, 500 },
        .scroll = { 0, 0 },
        .isWindowActive = true,
        .startAddress = RISC_MEMORY_DEFAULT_BASE,
        .instructionCount = 1000,
        .lineHeight = 22,
        .fontSize = 16,
        .followPC = true,
        .searchEditMode = false,
        .searchText = {0},
        .breakpointCount = 0,
        .breakpoints = {{0}}
    };

    return state;
}

void GuiLoadedProgram(GuiLoadedProgramState *state, const risc_cpu_t *cpu)
{
    if (state->isWindowActive == false) return;

    if (GuiWindowBox(state->bounds, "Loaded Program"))
    {
        state->isWindowActive = !state->isWindowActive;
    }

    // Define search field
    float searchY = state->bounds.y + 28;
    float searchHeight = 24;

    Rectangle searchBoxRect = {
        state->bounds.x + 8,
        searchY,
        state->bounds.width - 16 - 70,
        searchHeight
    };

    Rectangle jumpButtonRect = {
        state->bounds.x + state->bounds.width - 8 - 64,
        searchY,
        64,
        searchHeight
    };

    bool executeJump = false;

    if (GuiTextBox(searchBoxRect, state->searchText, sizeof(state->searchText), state->searchEditMode))
    {
        state->searchEditMode = !state->searchEditMode;
        if (state->searchEditMode == false)
        {
            executeJump = true;
        }
    }

    if (GuiButton(jumpButtonRect, "Jump"))
    {
        executeJump = true;
    }

    if (executeJump && state->searchText[0] != '\0')
    {
        uint32_t targetAddress = (uint32_t)strtoul(state->searchText, NULL, 16);

        if (targetAddress >= state->startAddress)
        {
            int index = (targetAddress - state->startAddress) / 4;
            if (index < 0) index = 0;
            if (index >= (int) state->instructionCount) index = state->instructionCount - 1;

            state->scroll.y = -((float) index * state->lineHeight);
        }
    }
    
    float panelTop = searchY + searchHeight + 6;
    Rectangle panelBounds = {
        state->bounds.x + 4,
        panelTop,
        state->bounds.width - 8,
        state->bounds.height - (panelTop - state->bounds.y) - 4
    };
    
    Rectangle content = {
        0, 0,
        panelBounds.width - 16,
        (float) state->instructionCount * state->lineHeight
    };
    
    GuiScrollPanel(panelBounds, NULL, content, &state->scroll, &state->view);

    
    BeginScissorMode(
        state->view.x, 
        state->view.y, 
        state->view.width, 
        state->view.height
    );

    int firstVisible = (-state->scroll.y) / state->lineHeight;
    int visibleCount = (state->view.height / state->lineHeight) + 2;

    for (int i = 0; i < firstVisible + visibleCount && i < (int)state->instructionCount; i++) 
    {
        uint32_t addr = state->startAddress + (i * 4);
        float rowY = panelBounds.y + state->scroll.y + (i * state->lineHeight);

        uint32_t instruction = 0;
        RiscMemoryRead(cpu->memory, addr, DWORD, &instruction);

        bool isCurrentPC = (cpu->pc == addr);
        if (isCurrentPC)
        {
            DrawRectangle(panelBounds.x, rowY, panelBounds.width, state->lineHeight, (Color){ 65, 105, 225, 100 });
            DrawText("->", panelBounds.x + 6, rowY + 3, state->fontSize, YELLOW);
        }

        DrawText(TextFormat("0x%08X", addr), panelBounds.x + 30, rowY + 3, state->fontSize, BLACK);
        DrawText(TextFormat("0x%08X", instruction), panelBounds.x + 130, rowY + 3, state->fontSize, GOLD);

        DrawText("inst", panelBounds.x + 220, rowY + 3, state->fontSize, RAYWHITE);

        bool hasBreakpoint = false;

        for (int b = 0; b < state->breakpointCount; b++)
        {
            if (state->breakpoints[b].address == addr && state->breakpoints[b].isActive)
            {
                hasBreakpoint = true;
            }
        }

        // TODO fix breakpoint logic
        /* 
        Rectangle gutterRect = { panelBounds.x, rowY, 24, (float) state->lineHeight };
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, gutterRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (hasBreakpoint) {
                RemoveBreakpoint(state, addr);
            }
            else
            {
                AddBreakpoint(state, addr);
            }
        }
        */

        if (hasBreakpoint)
        {
            DrawCircle(panelBounds.x + 12, rowY + state->lineHeight / 2.0f, 5.0f, RED);
        }

    }
    EndScissorMode();
}

static void RemoveBreakpoint(GuiLoadedProgramState *state, uint32_t addr)
{
    for (int i = 0; i < MAX_BREAKPOINTS; i++)
    {
        if (state->breakpoints[i].isActive && state->breakpoints[i].address == addr)
        {
            state->breakpoints[i].isActive = false;
            state->breakpoints[i].address = 0;
            state->breakpointCount--;
            break;
        }
    }
}

static void AddBreakpoint(GuiLoadedProgramState *state, uint32_t addr)
{
    for (int i = 0; i < MAX_BREAKPOINTS; i++)
    {
        if (state->breakpoints[i].isActive && state->breakpoints[i].address == addr) return;
    }
    for (int i = 0; i < MAX_BREAKPOINTS; i++)
    {
        if (!state->breakpoints[i].isActive)
        {
            state->breakpoints[i].address = addr;
            state->breakpoints[i].isActive = true;
            state->breakpointCount++;
            break;
        }
    }
}
