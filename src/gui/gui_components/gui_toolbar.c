#include "gui_toolbar.h"

#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


GuiToolbarState InitGuiToolbar(GuiToolbarCallbacks callbacks)
{
    GuiToolbarState state = {
        .bounds = { 0, 40, (float)GetScreenWidth(), 40 },
        .buttonWidth = 28,
        .buttonHeight = 28,
        .buttonCount = 2,
        .buttons = {
            { .label = "", .iconId = 206, .buttonCallback = callbacks.onOpenCpu },
            { .label = "", .iconId = 207, .buttonCallback = callbacks.onOpenLoadedProgram }
        },
        .padding = 12,
        .fontSize = 20,
    };

    return state;
}

void GuiToolbar(GuiToolbarState *state)
{
    state->bounds.width = (float)GetScreenWidth();
    GuiPanel(state->bounds, NULL);

    float currentX = state->bounds.x + state->padding;
    float buttonY = state->bounds.y + (state->bounds.height - state->buttonHeight) / 2.0f;

    for (size_t i = 0; i < state->buttonCount; i++)
    {
        Rectangle btnBounds = { currentX, buttonY, (float)state->buttonWidth, (float)state->buttonHeight };
        const char *text = state->buttons[i].iconId > 0
            ? TextFormat("#%d#%s", state->buttons[i].iconId, state->buttons[i].label)
            : state->buttons[i].label;

        if (GuiButton(btnBounds, text))
        {
            if (state->buttons[i].buttonCallback)
            {
                state->buttons[i].buttonCallback();
            }
        }

        currentX += state->buttonWidth + state->padding;
    }
}
