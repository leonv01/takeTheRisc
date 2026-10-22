#ifndef GUI_TOAST_H__
#define GUI_TOAST_H__

#include <raylib.h>
#include <stdbool.h>

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
