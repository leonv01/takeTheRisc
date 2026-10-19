#include "main_gui.h"
#include "risc/risc_cpu.h"
#include "risc/risc_memory.h"
#include "tools/logger.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION

#define GUI_CPU_IMPLEMENTATION
#include "gui_components/gui_cpu.h"

#define GUI_HEADBAR_IMPLEMENTATION
#include "gui_components/gui_headbar.h"

#include <raylib.h>

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

/* -------------------------------------------------------------------------- */

static risc_cpu_t cpu;
static risc_memory_t memory;
static Mode currentMode = LIGHT_MODE;
GuiCpuState cpuState;
GuiHeadbarState headbarState;

static GuiHeadbarCallbacks guiHeadbarCallbacks = {
    .onLoadElf = CallbackLoadElf,
    .onStep = CallbackStep,
    .onRun = CallbackRun,
    .onReset = CallbackReset
};

/* ---------------------------- Helper functions ---------------------------- */

static int InitializeComponents(void);
static int DestroyComponents(void);

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

    return 0;
}

static int DestroyComponents(void)
{
    RiscCpuDestroy(&cpu);

    return 0;
}

/* -------------------------------------------------------------------------- */

void CallbackLoadElf(void)
{
    LoggerMessage(DEBUG, "Load ELF");
}

void CallbackStep(void)
{
    LoggerMessage(DEBUG, "Step");
    RiscCpuExecute(&cpu);
}

void CallbackRun(void)
{
    LoggerMessage(DEBUG, "Run");

    for (; cpu.halted == false; )
    {
        RiscCpuExecute(&cpu);
    }
}

void CallbackReset(void)
{
    LoggerMessage(DEBUG, "Reset");
    RiscMemoryInitialize(&memory, RISC_MEMORY_DEFAULT_BASE, RISC_MEMORY_DEFAULT_SIZE);
    RiscCpuInitialize(&cpu, &memory);
}
