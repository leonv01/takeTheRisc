#include "main_gui.h"
#include "risc/risc_cpu.h"
#include "risc/risc_memory.h"
#include "tools/file_reader.h"
#include "tools/logger.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#include "gui_components/gui_cpu.h"
#include "gui_components/gui_headbar.h"
#include "gui_components/gui_file_dialog.h"
#include "gui_components/gui_toast.h"
#include "gui_components/gui_loaded_program.h"
#include "gui_components/gui_toolbar.h"

#include <raylib.h>
#include <pthread.h>

/* -------------------------------------------------------------------------- */
typedef enum Mode {
    LIGHT_MODE,
    DARK_MODE
} Mode;

typedef enum WindowId {
    WINDOW_ID_CPU,
    WINDOW_ID_LOADED_PROGRAM,
    WINDOW_ID_COUNT
} WindowId;

static WindowId windowOrder[WINDOW_ID_COUNT] = {
    WINDOW_ID_CPU,
    WINDOW_ID_LOADED_PROGRAM
};

static void BringWindowToFront(WindowId id);
static Rectangle GetWindowBounds(WindowId id);
static bool IsWindowActive(WindowId id);
static void HandleWindowFocus(void);

/* -------------------------------------------------------------------------- */

/* ---------------------------- Callback handler ---------------------------- */
static void CallbackLoadElf(void);
static void CallbackStep(void);
static void CallbackRun(void);
static void CallbackReset(void);
static void CallbackOpenFile(void);
static void CallbackClose(void);
static void CallbackStop(void);
static void CallbackOpenCpu(void);
static void CallbackOpenLoadedProgram(void);
/* -------------------------------------------------------------------------- */

/* -------------------------------- Variables ------------------------------- */
static pthread_t cpuThread;
static pthread_mutex_t cpuMutex = PTHREAD_MUTEX_INITIALIZER;
static bool cpuThreadRunning = false;
static bool emulatorActive = true;

static risc_cpu_t cpu;
static risc_memory_t memory;
static Mode currentMode = LIGHT_MODE;
GuiCpuState cpuState;
GuiHeadbarState headbarState;
GuiWindowFileDialogState fileDialogState;
GuiToastState toastState;
GuiLoadedProgramState loadedProgramState;
GuiToolbarState toolbarState;

static GuiHeadbarCallbacks guiHeadbarCallbacks = {
    .onLoadElf  = CallbackLoadElf,
    .onStep     = CallbackStep,
    .onRun      = CallbackRun,
    .onReset    = CallbackReset,
    .onStop     = CallbackStop
};

static GuiToolbarCallbacks guiToolbarCallbacks = {
    .onOpenCpu = CallbackOpenCpu,
    .onOpenLoadedProgram = CallbackOpenLoadedProgram
};

static GuiWindowFileDialogCallbacks guiWindowFileDialogCallbacks = {
    .onOpenFile = CallbackOpenFile,
    .onClose = CallbackClose
};
/* -------------------------------------------------------------------------- */

/* ---------------------------- Helper functions ---------------------------- */

static int InitializeComponents(void);
static int DestroyComponents(void);
static void *CpuThread(void *arg);

/* -------------------------------------------------------------------------- */

int GuiInitialize(int width, int height, const char *title)
{
    SetTargetFPS(144);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, title);

    InitializeComponents();

    return 0;
}

int GuiRun(void)
{
    while (WindowShouldClose() == false)
    {
        HandleWindowFocus();

        BeginDrawing();
        {
            ClearBackground(GRAY);
            
            for (size_t i = 0; i < WINDOW_ID_COUNT; i++)
            {
                switch (windowOrder[i])
                {
                case WINDOW_ID_CPU: GuiCpu(&cpuState, &cpu); break;
                case WINDOW_ID_LOADED_PROGRAM: GuiLoadedProgram(&loadedProgramState, &cpu); break;
                default: break;
                }
            }
            

            GuiHeadbar(&headbarState);
            GuiToolbar(&toolbarState);
            GuiWindowFileDialog(&fileDialogState);
            GuiToast(&toastState);
        }
        EndDrawing();
    }

    return 0;
}

int GuiDestroy(void)
{
    CloseWindow();

    return 0;
}

/* -------------------------------------------------------------------------- */

static int InitializeComponents(void)
{
    LoggerInitialize(DEBUG, stdout);

    RiscMemoryInitialize(&memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    RiscCpuInitialize(&cpu, &memory);

    cpuState = InitGuiCpu();
    headbarState = InitGuiHeadbar(guiHeadbarCallbacks);
    fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory(), guiWindowFileDialogCallbacks);
    toastState = InitGuiToastState();
    loadedProgramState = InitGuiLoadedProgram();
    toolbarState = InitGuiToolbar(guiToolbarCallbacks);
    pthread_create(&cpuThread, NULL, CpuThread, NULL);

    return 0;
}

static int DestroyComponents(void)
{
    RiscCpuDestroy(&cpu);

    emulatorActive = false;
    pthread_join(cpuThread, NULL);

    return 0;
}

/* -------------------------------------------------------------------------- */

void CallbackLoadElf(void)
{
    LoggerMessage(DEBUG, "Load ELF");

    fileDialogState.windowActive = true;

}

void CallbackStep(void)
{
    LoggerMessage(DEBUG, "Step");
    RiscCpuExecute(&cpu);
    cpuThreadRunning = false;
}

void CallbackRun(void)
{
    LoggerMessage(DEBUG, "Run");

    emulatorActive = true;
    cpuThreadRunning = true;
}

void CallbackReset(void)
{
    LoggerMessage(DEBUG, "Reset");
    RiscMemoryInitialize(&memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    RiscCpuInitialize(&cpu, &memory);
}

void CallbackStop(void)
{
    emulatorActive = false;
    cpuThreadRunning = false;
}

void CallbackOpenFile(void)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s/%s", fileDialogState.dirPathText, fileDialogState.fileNameText);

    LoggerMessage(INFO, "Loading file: %s", buffer);
    FileReaderLoadMemoryElf(buffer, &cpu);

    GuiToastShow(&toastState, "File loaded", TOAST_SUCCESS, 2.0f);
}

void CallbackClose(void)
{

}

void CallbackOpenCpu(void)
{
    cpuState.isWindowActive = !cpuState.isWindowActive;
}

void CallbackOpenLoadedProgram(void)
{
    loadedProgramState.isWindowActive = !loadedProgramState.isWindowActive;
}


void *CpuThread(void *arg)
{
    LoggerMessage(DEBUG, "Thread initialized");
    while (emulatorActive)
    {
        if (cpuThreadRunning)
        {
            pthread_mutex_lock(&cpuMutex);
            if (cpu.halted == false)
            {
                RiscCpuExecute(&cpu);
            }
            else
            {
                cpuThreadRunning = false;
            }
            pthread_mutex_unlock(&cpuMutex);
        }
        else
        {
            struct timespec ts = { 0, 1000000 };
            nanosleep(&ts, NULL);
        }
    }

    return NULL;
}

static void BringWindowToFront(WindowId id)
{
    int index = -1;
    for (int i = 0; i < (int)WINDOW_ID_COUNT; i++)
    {
        if (windowOrder[i] == id)
        {
            index = i;
            break;
        }
    }
    
    if (index != -1 && index < (int)WINDOW_ID_COUNT - 1)
    {
        for (int i = index; i < (int)WINDOW_ID_COUNT - 1; i++)
        {
            windowOrder[i] = windowOrder[i + 1];
        }
        windowOrder[WINDOW_ID_COUNT - 1] = id;
    }
}

static Rectangle GetWindowBounds(WindowId id)
{
    switch (id)
    {
    case WINDOW_ID_CPU: return cpuState.bounds;
    case WINDOW_ID_LOADED_PROGRAM: return loadedProgramState.bounds;
    default: return (Rectangle){ 0 };
    }
}

static bool IsWindowActive(WindowId id)
{
    switch (id)
    {
    case WINDOW_ID_CPU:            return cpuState.isWindowActive;
    case WINDOW_ID_LOADED_PROGRAM: return loadedProgramState.isWindowActive;
    default:                       return false;
    }
}

static void HandleWindowFocus(void)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) == false) return;

    if (fileDialogState.windowActive) return;

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, headbarState.bounds)) return;
    if (CheckCollisionPointRec(mouse, toolbarState.bounds)) return;

    for (int i = (int)WINDOW_ID_COUNT - 1; i >= 0; i--)
    {
        WindowId id = windowOrder[i];
        if (!IsWindowActive(id)) continue;

        Rectangle bounds = GetWindowBounds(id);
        if (CheckCollisionPointRec(mouse, bounds))
        {
            BringWindowToFront(id);
            break;
        }
    }
}
