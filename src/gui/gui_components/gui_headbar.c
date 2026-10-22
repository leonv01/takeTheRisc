#include "gui_headbar.h"

#include <raygui.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

GuiHeadbarState InitGuiHeadbar(GuiHeadbarCallbacks callbacks)
{
    GuiHeadbarState state = {
        .bounds = { 0, 0, (float)GetScreenWidth(), 40 },
        .buttonWidth = 90,
        .buttonHeight = 28,
        .buttonCount = 5,
        .buttons = {
            { .label = "Load ELF", .iconId = 198, .buttonCallback = callbacks.onLoadElf },
            { .label = "Step", .iconId = 134, .buttonCallback = callbacks.onStep },
            { .label = "Run", .iconId = 131, .buttonCallback = callbacks.onRun },
            { .label = "Stop", .iconId = 133, .buttonCallback = callbacks.onStop },
            { .label = "Reset", .iconId = 210, .buttonCallback = callbacks.onReset },
        },
        .padding = 12,
        .fontSize = 20,
    };

    return state;
}

void GuiHeadbar(GuiHeadbarState *state)
{
    state->bounds.width = (float)GetScreenWidth();
    GuiPanel(state->bounds, NULL);

    float currentX = state->bounds.x + state->padding;
    float buttonY = state->bounds.y + (state->bounds.height - state->buttonHeight) / 2.0f;

    for (int i = 0; i < state->buttonCount; i++) 
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
