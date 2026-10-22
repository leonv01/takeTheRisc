#ifndef GUI_TOOLBAR_H__
#define GUI_TOOLBAR_H__

#include "gui_headbar.h"

#include <raylib.h>
#include <raygui.h>

#define TOOLBAR_BUTTON_COUNT 10

typedef struct GuiToolbarCallbacks {
    void (*onOpenCpu)(void);
    void (*onOpenLoadedProgram)(void);
} GuiToolbarCallbacks;

typedef struct GuiToolbarState {
    Rectangle bounds;
    int width, height;
    int padding;
    int fontSize;
    int buttonWidth, buttonHeight;
    int buttonCount;

    GuiMenuButton buttons[TOOLBAR_BUTTON_COUNT];
} GuiToolbarState;

GuiToolbarState InitGuiToolbar(GuiToolbarCallbacks callbacks);
void GuiToolbar(GuiToolbarState *state);

#endif // GUI_TOOLBAR_H__