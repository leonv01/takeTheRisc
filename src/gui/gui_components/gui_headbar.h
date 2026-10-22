#ifndef GUI_HEADBAR_H__
#define GUI_HEADBAR_H__

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>

#define HEADBAR_BUTTON_COUNT 10

typedef struct GuiHeadbarCallbacks {
    void (*onLoadElf)(void);
    void (*onStep)(void);
    void (*onRun)(void);
    void (*onReset)(void);
    void (*onStop)(void);
} GuiHeadbarCallbacks;

typedef struct GuiMenuButton {
    const char *label;
    Vector2 position;
    int iconId;
    bool active;
    void (*buttonCallback)(void);
} GuiMenuButton;

// Gui header bar context data
typedef struct GuiHeadbarState {
    Rectangle bounds;
    int width, height;
    int padding;
    int fontSize;
    int buttonWidth, buttonHeight;
    int buttonCount;

    GuiMenuButton buttons[HEADBAR_BUTTON_COUNT];
} GuiHeadbarState;

GuiHeadbarState InitGuiHeadbar(GuiHeadbarCallbacks callbacks);
void GuiHeadbar(GuiHeadbarState *state);

#endif // GUI_HEADBAR_H__
