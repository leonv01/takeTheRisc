#ifndef GUI_TOAST_H__
#define GUI_TOAST_H__

#include "raygui.h"
#include "raylib.h"

typedef enum GuiToastType
{
    TOAST_INFO,
    TOAST_WARNING,
    TOAST_ERROR,
    TOAST_SUCCESS
} GuiToastType;

typedef struct GuiToastState
{
    Rectangle bounds;
    double startTime;
    float duration;

    int height;
    int fontSize;
    bool hasInformation;
    GuiToastType toastType;
    const char *text;
} GuiToastState;

GuiToastState InitGuiToastState(void);
void GuiToast(GuiToastState *state);
void GuiToastShow(
    GuiToastState *state, 
    const char *text,
    GuiToastType type,
    float duration
);

#endif // GUI_TOAST_H__

#ifdef GUI_TOAST_H_IMPLEMENTATION

#include "raygui.h"
#include <string.h>

GuiToastState InitGuiToastState(void)
{
    GuiToastState state = {
        .height = 50,
        .bounds = { .x = 0, .y = GetScreenHeight() / 2.0 - 10 / 2.0, .height = 50, .width = (float)GetScreenWidth() },
        .startTime = GetTime(),
        .fontSize = 20,
        .duration = 5.0f,
        .hasInformation = false,
        .text = "",
        .toastType = TOAST_INFO
    };

    return state;
};

void GuiToast(GuiToastState *state)
{

    if (state->hasInformation == false) return;

    if (GetTime() - state->startTime >= state->duration)
    {
        state->hasInformation = false;
        return;
    }

    Color bgColor = DARKGRAY;
    switch (state->toastType)
    {
        case SUCCESS: bgColor = (Color){ 46, 139, 87, 240 };  break; 
        case ERROR:   bgColor = (Color){ 178, 34, 34, 240 };  break; 
        case WARNING: bgColor = (Color){ 218, 165, 32, 240 }; break; 
        case INFO:    bgColor = (Color){ 70, 130, 180, 240 }; break;
        default: break;
    }

    state->bounds.x = (GetScreenWidth() - state->bounds.width) / 2.0f;
    state->bounds.y = GetScreenHeight() - state->height - 20;
    state->bounds.width = MeasureText(state->text, state->fontSize) + 40;
    state->bounds.height = state->height;

    DrawRectangleRounded(state->bounds, 0.3f, 4, bgColor);
    DrawRectangleRoundedLines(state->bounds, 0.3f, 4, RAYWHITE);

    int textWidth = MeasureText(state->text, state->fontSize);
    DrawText(
        state->text,
        state->bounds.x + (state->bounds.width - textWidth) / 2.0f,
        state->bounds.y + (state->bounds.height - state->fontSize) / 2.0f,
        state->fontSize,
        RAYWHITE
    );
}

void GuiToastShow(
    GuiToastState *state, 
    const char *text,
    GuiToastType type,
    float duration
)
{
    state->text = text;
    state->toastType = type;
    state->duration = duration;
    state->startTime = GetTime();
    state->hasInformation = true;
}

#endif