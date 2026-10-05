#include "main_gui.h"

#include <raylib.h>

int GuiInitialize(void)
{
    int screenWidth = 1280;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "RISC-V Emulator");

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        {
            ClearBackground(GRAY);
        }
        EndDrawing();
    }

    CloseWindow();
}