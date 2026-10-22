#include "gui_cpu.h"

#include <raygui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *RISC_ABI_NAMES[REGISTER_COUNT] = {
    "zero", "ra",  "sp",  "gp",  "tp",  "t0",  "t1",  "t2",
    "s0",   "s1",  "a0",  "a1",  "a2",  "a3",  "a4",  "a5",
    "a6",   "a7",  "s2",  "s3",  "s4",  "s5",  "s6",  "s7",
    "s8",   "s9",  "s10", "s11", "t3",  "t4",  "t5",  "t6"
};

GuiCpuState InitGuiCpu(void)
{
    GuiCpuState state = { 
        .bounds = { 510, 60, 440, 455 },
        .panOffset = { 0, 0 },
        .dragMode = false,
        .padding = 4,
        .fontSize = 14,
        .isWindowActive = true
    };
    
    return state;
}

void GuiCpu(GuiCpuState *state, const risc_cpu_t *cpu)
{
    if (!state->isWindowActive) return;

    // Window dragging logic
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Rectangle titleBar = { state->bounds.x, state->bounds.y, state->bounds.width - 24, 24 };
        if (CheckCollisionPointRec(mouse, titleBar))
        {
            state->dragMode = true;
            state->panOffset = (Vector2){ mouse.x - state->bounds.x, mouse.y - state->bounds.y };
        }
    }

    if (state->dragMode)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            state->bounds.x = mouse.x - state->panOffset.x;
            state->bounds.y = mouse.y - state->panOffset.y;

            if (state->bounds.x < 0) state->bounds.x = 0;
            if (state->bounds.y < 40) state->bounds.y = 40;
            if (state->bounds.x + state->bounds.width > GetScreenWidth())
                state->bounds.x = GetScreenWidth() - state->bounds.width;
            if (state->bounds.y + state->bounds.height > GetScreenHeight())
                state->bounds.y = GetScreenHeight() - state->bounds.height;
        }
        else
        {
            state->dragMode = false;
        }
    }

    // Render Window Frame
    if (GuiWindowBox(state->bounds, "CPU State & Registers"))
    {
        state->isWindowActive = false;
        return;
    }

    uint32_t pc = cpu ? cpu->pc : 0;
    bool isHalted = cpu ? cpu->halted : false;

    // --- Header Section: PC & Halted Status ---
    float headerY = state->bounds.y + 28;
    float headerHeight = 32;

    // PC Display
    DrawText("PC:", (int)state->bounds.x + 12, (int)headerY + 7, 16, DARKGRAY);
    DrawText(TextFormat("0x%08X", pc), (int)state->bounds.x + 44, (int)headerY + 7, 16, (Color){ 30, 90, 200, 255 });

    // Status Badge (HALTED vs RUNNING)
    Rectangle statusBadge = {
        state->bounds.x + state->bounds.width - 100,
        headerY + 4,
        88,
        22
    };

    if (isHalted)
    {
        DrawRectangleRounded(statusBadge, 0.35f, 4, (Color){ 220, 38, 38, 255 });
        int textW = MeasureText("HALTED", 12);
        DrawText("HALTED", (int)(statusBadge.x + (statusBadge.width - textW) / 2.0f), (int)statusBadge.y + 5, 12, RAYWHITE);
    }
    else
    {
        DrawRectangleRounded(statusBadge, 0.35f, 4, (Color){ 22, 163, 74, 255 });
        int textW = MeasureText("RUNNING", 12);
        DrawText("RUNNING", (int)(statusBadge.x + (statusBadge.width - textW) / 2.0f), (int)statusBadge.y + 5, 12, RAYWHITE);
    }

    // Separator under header
    DrawLine((int)state->bounds.x + 8, (int)(headerY + headerHeight), 
             (int)(state->bounds.x + state->bounds.width - 8), (int)(headerY + headerHeight), (Color){ 210, 210, 210, 255 });

    // --- Column Setup (2 columns of 16 registers) ---
    float colHeaderY = headerY + headerHeight + 5;
    float colWidth = (state->bounds.width - 24) / 2.0f;
    float col0_x = state->bounds.x + 8;
    float col1_x = col0_x + colWidth + 8;

    // Column Headers
    DrawText("REG", (int)col0_x + 6, (int)colHeaderY, 11, GRAY);
    DrawText("ABI", (int)col0_x + 36, (int)colHeaderY, 11, GRAY);
    DrawText("VALUE", (int)col0_x + 88, (int)colHeaderY, 11, GRAY);

    DrawText("REG", (int)col1_x + 6, (int)colHeaderY, 11, GRAY);
    DrawText("ABI", (int)col1_x + 36, (int)colHeaderY, 11, GRAY);
    DrawText("VALUE", (int)col1_x + 88, (int)colHeaderY, 11, GRAY);

    DrawLine((int)state->bounds.x + 8, (int)colHeaderY + 16,
             (int)(state->bounds.x + state->bounds.width - 8), (int)colHeaderY + 16, (Color){ 230, 230, 230, 255 });

    // Center divider between the two columns
    float midX = (col0_x + colWidth + col1_x) / 2.0f;
    DrawLine((int)midX, (int)colHeaderY, (int)midX, (int)(colHeaderY + 18 + 16 * 22), (Color){ 230, 230, 230, 255 });

    // --- 16 Register Rows ---
    float startY = colHeaderY + 18;
    float rowHeight = 22;

    for (int row = 0; row < 16; row++)
    {
        float rowY = startY + (row * rowHeight);

        // --- Column 0 (x0 - x15) ---
        int reg0 = row;
        uint32_t val0 = (cpu && reg0 < REGISTER_COUNT) ? cpu->registers[reg0] : 0;

        if (row % 2 == 1)
        {
            DrawRectangle((int)col0_x, (int)rowY, (int)colWidth, (int)rowHeight, (Color){ 246, 246, 248, 255 });
        }

        DrawText(TextFormat("x%-2d", reg0), (int)col0_x + 6, (int)rowY + 4, state->fontSize, (Color){ 70, 70, 70, 255 });
        DrawText(RISC_ABI_NAMES[reg0], (int)col0_x + 36, (int)rowY + 4, state->fontSize, (reg0 == 2 ? (Color){ 180, 70, 20, 255 } : (Color){ 100, 100, 100, 255 }));

        Color valColor0 = (val0 != 0) ? (Color){ 20, 60, 160, 255 } : (Color){ 150, 150, 150, 255 };
        DrawText(TextFormat("0x%08X", val0), (int)col0_x + 88, (int)rowY + 4, state->fontSize, valColor0);

        // --- Column 1 (x16 - x31) ---
        int reg1 = row + 16;
        uint32_t val1 = (cpu && reg1 < REGISTER_COUNT) ? cpu->registers[reg1] : 0;

        if (row % 2 == 1)
        {
            DrawRectangle((int)col1_x, (int)rowY, (int)colWidth, (int)rowHeight, (Color){ 246, 246, 248, 255 });
        }

        DrawText(TextFormat("x%-2d", reg1), (int)col1_x + 6, (int)rowY + 4, state->fontSize, (Color){ 70, 70, 70, 255 });
        DrawText(RISC_ABI_NAMES[reg1], (int)col1_x + 36, (int)rowY + 4, state->fontSize, (Color){ 100, 100, 100, 255 });

        Color valColor1 = (val1 != 0) ? (Color){ 20, 60, 160, 255 } : (Color){ 150, 150, 150, 255 };
        DrawText(TextFormat("0x%08X", val1), (int)col1_x + 88, (int)rowY + 4, state->fontSize, valColor1);
    }
}
