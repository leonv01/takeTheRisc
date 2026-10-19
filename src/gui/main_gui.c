#include "main_gui.h"
#include "risc/risc_cpu.h"
#include "risc/risc_memory.h"
#include "tools/file_reader.h"
#include "tools/logger.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION

#define GUI_CPU_IMPLEMENTATION
#include "gui_components/gui_cpu.h"

#define GUI_HEADBAR_IMPLEMENTATION
#include "gui_components/gui_headbar.h"

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_components/gui_file_dialog.h"

#include <raylib.h>
#include <pthread.h>

/* -------------------------------------------------------------------------- */
typedef enum Mode {
    LIGHT_MODE,
    DARK_MODE
} Mode;

typedef void (*Renderables)(void);
/* -------------------------------------------------------------------------- */

/* ---------------------------- Callback handler ---------------------------- */
static void CallbackLoadElf(void);
static void CallbackStep(void);
static void CallbackRun(void);
static void CallbackReset(void);
static void CallbackOpenFile(void);
static void CallbackClose(void);
static void CallbackStop(void);
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

static GuiHeadbarCallbacks guiHeadbarCallbacks = {
    .onLoadElf = CallbackLoadElf,
    .onStep = CallbackStep,
    .onRun = CallbackRun,
    .onReset = CallbackReset,
    .onStop = CallbackStop
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
        BeginDrawing();
        {
            ClearBackground(GRAY);
            GuiCpu(&cpuState);
            GuiHeadbar(&headbarState);
            GuiWindowFileDialog(&fileDialogState);
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
    cpuThreadRunning = true;
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
}

void CallbackClose(void)
{

}

static void *CpuThread(void *arg)
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