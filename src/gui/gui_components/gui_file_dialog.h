#ifndef GUI_WINDOW_FILE_DIALOG_H
#define GUI_WINDOW_FILE_DIALOG_H

#include "raylib.h"

typedef struct GuiWindowFileDialogCallbacks {
    void (*onOpenFile)(void);
    void (*onClose)(void);
} GuiWindowFileDialogCallbacks;

// Gui file dialog context data
typedef struct 
{
    // Window management variables
    bool windowActive;
    Rectangle windowBounds;
    Vector2 panOffset;
    bool dragMode;
    bool supportDrag;

    // UI variables
    bool dirPathEditMode;
    char dirPathText[1024];

    int filesListScrollIndex;
    bool filesListEditMode;
    int filesListActive;

    bool fileNameEditMode;
    char fileNameText[1024];
    bool SelectFilePressed;
    bool CancelFilePressed;
    int fileTypeActive;
    int itemFocused;

    // Custom state variables
    FilePathList dirFiles;
    char filterExt[256];
    char dirPathTextCopy[1024];
    char fileNameTextCopy[1024];

    int prevFilesListActive;

    bool saveFileMode;

    // Callbacks
    void (*openFileCallback)(void);
    void (*closeCallback)(void);

} GuiWindowFileDialogState;

GuiWindowFileDialogState InitGuiWindowFileDialog(const char *initPath, GuiWindowFileDialogCallbacks callbacks);
void GuiWindowFileDialog(GuiWindowFileDialogState *state);

#endif // GUI_WINDOW_FILE_DIALOG_H
