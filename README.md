# TakeTheRisc

**TakeTheRisc** is an interactive 32-bit RISC-V (RV32I) instruction set architecture (ISA) emulator and graphical debugger written in C99, featuring a responsive desktop GUI powered by [Raylib](https://www.raylib.com/) and [Raygui](https://github.com/raysan5/raygui).

The project is designed as both an educational platform for learning computer architecture and a functional tool for stepping through and analyzing compiled RISC-V binaries and ELF executables in real time.

---

## Table of Contents

- [Project Scope](#project-scope)
- [Key Features](#key-features)
- [Architecture Overview](#architecture-overview)
- [Supported ISA Specifications](#supported-isa-specifications)
- [Graphical Interface & Debugger](#graphical-interface--debugger)
- [Project Structure](#project-structure)
- [Building and Running](#building-and-running)
- [Running the Test Suite](#running-the-test-suite)
- [Development & Authorship](#development--authorship)
- [Roadmap](#roadmap)
- [References & Resources](#references--resources)

---

## Project Scope

The primary objective of **TakeTheRisc** is to build a complete, transparent, and developer-friendly RISC-V environment from the ground up:

1. **Cycle-Accurate Instruction Execution**: Emulate the unprivileged user-level integer instruction set (RV32I) in compliance with the official RISC-V specifications.
2. **Direct Binary & ELF Loading**: Provide native loading of 32-bit ELF binaries (`PT_LOAD` segments) and flat raw binaries directly into emulated memory with proper entry point initialization.
3. **Decoupled Multi-Threaded Core**: Run emulation execution on a dedicated background POSIX worker thread (`pthread`) so that stepping, high-speed execution loops, and breakpoint halts never freeze the user interface.
4. **Immediate-Mode Graphical Debugger**: Offer a floating-window GUI that provides real-time visibility into the CPU register file, memory layout, program counter, and execution control.

---

## Key Features

- **RV32I Core Emulation**: Full decode-execute engine for 32-bit integer instructions with zero-register (`x0`) enforcement.
- **Flat Memory Subsystem**: Configurable base address (`0x80000000` by default) with byte, halfword, and word reads and writes.
- **ELF32 File Loader**: Parses ELF headers and program headers, loads executable segments, and sets the program counter to `e_entry`.
- **Multi-Threaded Execution**: Non-blocking GUI with synchronized worker thread execution (`Step`, `Run`, `Stop`, `Reset`).
- **Dynamic Z-Order Window Manager**: Interactive floating windows with bring-to-front focus on click and draggable title bars.
- **Live Disassembly & Code View**: Virtualized scrolling instruction list with PC tracking and address jump navigation.
- **Full Register Inspector**: 2-column register file viewer showing all 32 registers with official RISC-V ABI names (`zero`, `ra`, `sp`, `gp`, `tp`, `t0`-`t6`, `s0`-`s11`, `a0`-`a7`) and dynamic value highlighting.
- **Built-in File Dialog & Toasts**: In-app file system explorer to select and load binaries without external dependencies.
- **Automated Verification**: Integrated [Unity](https://github.com/ThrowTheSwitch/Unity) unit tests and official `riscv-tests` compliance validation.

---

## Architecture Overview

```
                      +---------------------------------------+
                      |               Main GUI                |
                      |  (Raylib / Raygui Render Loop @ 144Hz)|
                      +-------------------+-------------------+
                                          |
                        Mutex-Protected Execution Control
                                          |
                      +-------------------v-------------------+
                      |         CPU Worker Thread             |
                      |       (pthread Background Loop)       |
                      +-------------------+-------------------+
                                          |
                 +------------------------+------------------------+
                 |                                                 |
+----------------v----------------+               +----------------v----------------+
|       risc_cpu (RV32I)          |               |          risc_memory            |
|  - 32 General Registers (x0-x31)|<------------->|  - Base: 0x80000000             |
|  - Program Counter (PC)         |  Read / Write |  - 1 MiB Default Space          |
|  - Instruction Decoder & Ops    |               |  - Byte, Half, Word Access      |
+---------------------------------+               +---------------------------------+
                 ^                                                 ^
                 |                                                 |
                 +------------------------+------------------------+
                                          |
                          +---------------+---------------+
                          |          file_reader          |
                          |  - ELF32 Segment Parser       |
                          |  - Raw Binary Memory Mapper   |
                          +-------------------------------+
```

---

## Supported ISA Specifications

### RV32I Base Integer Instruction Set

| Category | Instructions |
| :--- | :--- |
| **Arithmetic (R-Type)** | `ADD`, `SUB`, `SLT`, `SLTU`, `XOR`, `OR`, `AND`, `SLL`, `SRL`, `SRA` |
| **Arithmetic (I-Type)** | `ADDI`, `SLTI`, `SLTIU`, `XORI`, `ORI`, `ANDI`, `SLLI`, `SRLI`, `SRAI` |
| **Loads (I-Type)** | `LB`, `LH`, `LW`, `LBU`, `LHU` |
| **Stores (S-Type)** | `SB`, `SH`, `SW` |
| **Branches (B-Type)** | `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU` |
| **Jumps (J/I-Type)** | `JAL`, `JALR` |
| **Upper Immediate** | `LUI`, `AUIPC` |
| **System / Compliance** | `ECALL`, `EBREAK`, `tohost` memory write trap |

---

## Graphical Interface & Debugger

The graphical user interface is built as a modular set of desktop components:

- **Top Headbar**: Contains emulator action buttons:
  - **Load ELF**: Opens the file explorer.
  - **Step**: Executes a single instruction and pauses.
  - **Run**: Starts continuous asynchronous execution.
  - **Stop**: Halts worker thread execution.
  - **Reset**: Re-initializes memory and registers.
- **Toolbar**: Quick toggle switches to show or hide the floating inspector windows.
- **CPU Inspector Window**:
  - Displays `PC: 0x...` and an active status badge (`HALTED` / `RUNNING`).
  - Displays all 32 registers in two balanced 16-row columns with ABI names and live values.
  - Draggable title bar with viewport-clamped movement.
- **Program Disassembly Window**:
  - Displays virtual memory address, raw hex instruction, and disassembled mnemonic.
  - Active instruction pointer (`->`) automatically tracks the current PC.
  - Search jump bar to jump directly to any hex address.
  - Breakpoint indicator infrastructure.
- **File Dialog Window**: Native graphical file picker supporting directory navigation and file loading.
- **Toast Notifications**: Automatic non-intrusive alerts for operations like file loading and execution warnings.

---

## Project Structure

```
TakeTheRisc/
├── CMakeLists.txt              # Top-level build configuration
├── extern/                     # External dependencies as submodules
│   ├── raylib/                 # Raylib graphics library
│   ├── raygui/                 # Immediate-mode GUI library
│   └── unity/                  # Unit testing framework
├── src/
│   ├── main.c                  # Main application entry point
│   ├── risc/                   # Core emulation engine
│   │   ├── risc_cpu.c/h        # CPU registers, opcode dispatch, instruction handlers
│   │   └── risc_memory.c/h     # Virtual memory translation and read/write operations
│   ├── tools/                  # Utility and infrastructure libraries
│   │   ├── file_reader.c/h     # ELF32 header parsing and memory population
│   │   └── logger.c/h          # Multi-level logging and instruction tracing
│   └── gui/                    # GUI components and main interface
│       ├── main_gui.c/h        # Main window loop, thread dispatch, Z-order manager
│       └── gui_components/     # Modular UI panels
│           ├── gui_cpu.c/h            # CPU & register inspector panel
│           ├── gui_headbar.c/h        # Top action toolbar
│           ├── gui_toolbar.c/h        # Window toggle bar
│           ├── gui_loaded_program.c/h # Disassembly view & breakpoint list
│           ├── gui_file_dialog.c/h    # File navigation dialog
│           └── gui_toast.c/h          # Toast notification system
└── tests/                      # Verification and compliance test suites
    ├── cpu/                    # Unit tests for instruction execution
    ├── memory/                 # Memory read/write tests
    └── riscv-tests/            # Official RISC-V compliance test suite binaries
```

---

## Building and Running

### Prerequisites

- **C Compiler**: GCC or Clang supporting C99
- **CMake**: Version 3.26 or higher
- **Build Tool**: Ninja or GNU Make
- **Graphics / Window Dependencies**: Standard Raylib dependencies on Linux (X11, GL, pthread):
  ```bash
  # Debian / Ubuntu / Mint
  sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev \
                   libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

  # Fedora / RHEL
  sudo dnf install alsa-lib-devel libX11-devel libXrandr-devel libXi-devel \
                   mesa-libGL-devel libXcursor-devel libXinerama-devel
  ```

### Build Instructions

1. **Clone the repository and submodules**:
   ```bash
   git clone --recurse-submodules https://github.com/yourusername/TakeTheRisc.git
   cd TakeTheRisc
   ```

2. **Configure the build with CMake**:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug
   ```

3. **Compile the project**:
   ```bash
   cmake --build build
   ```

4. **Run the emulator**:
   ```bash
   ./build/TakeTheRisc
   ```

---

## Running the Test Suite

The test suite is integrated into CMake using CTest:

```bash
ctest --test-dir build --output-on-failure
```

Or run individual test suites directly:
```bash
./build/test_cpu_instructions
./build/test_cpu_riscv_tests
./build/test_memory_read
./build/test_memory_write
```

---

## Development & Authorship

This project was built through a collaborative engineering workflow between the primary human developer and an agentic AI coding assistant ([Google DeepMind's Antigravity](https://deepmind.google/)). **All code, architectural adjustments, and refactorings proposed by the AI assistant were strictly supervised, reviewed, and verified by the human developer through static inspection, unit testing, and manual runtime validation.**

### Explicit Human Contributions

The overall architecture, core system design, instruction handling, and foundational features were authored directly by the human developer:

- **Human Supervision & Code Verification**:
  - Supervised, reviewed, tested, and validated all AI-assisted refactorings and code proposals to ensure strict compliance with project goals, C99 idioms, and runtime stability.
- **Core CPU & Instruction Architecture**:
  - Authored the fundamental CPU structure (`risc_cpu_t`), register file (`x0`-`x31`), program counter (`pc`), and function-pointer instruction dispatch tables.
  - Implemented the bulk of the RV32I integer instruction set:
    - Arithmetic & Logical operations (R-type and I-type: `ADD`, `SUB`, `SLT`, `SLTU`, `XOR`, `OR`, `AND`, `SLL`, `SRL`, `SRA`, `ADDI`, etc.)
    - Branching instructions (`BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU`) and Jumps (`JAL`, `JALR`)
    - Load & Store instructions with endianness and sign/zero extensions (`LB`, `LH`, `LW`, `LBU`, `LHU`, `SB`, `SH`, `SW`)
    - System calls (`ECALL`), halt handling, and software-driven `tohost` compliance trap logic.
- **Memory & Binary Loading**:
  - Implemented the virtual memory model (`risc_memory_t`) with base offsets and memory boundaries.
  - Wrote the ELF32 executable parser (`Elf32_Ehdr`, `Elf32_Phdr`), loading loadable segments (`PT_LOAD`) and initializing the entry point, as well as flat binary loaders.
- **GUI Scaffolding & Tooling**:
  - Integrated the Raylib and Raygui libraries into the C99 build.
  - Implemented initial GUI panels including the headbar, file dialog, toast alerts, program inspection view, and window toolbars.
  - Implemented the logging system (`logger.c/h`) and opcode debug tracing.
- **Testing & DevOps**:
  - Established the CMake build environment and automated GitHub Actions CI pipelines with required Linux graphics dependencies.
  - Integrated the Unity test framework and authored extensive instruction-level test suites.
  - Configured Docker environments and ported the official `riscv-tests` compliance suite into the automated test runner.

### AI Pair-Programming Role

The AI assistant was utilized as an interactive pair programmer for refactoring, concurrency design, low-level debugging, and documentation (with all output subject to human supervision):

- **Project Documentation**:
  - Authored this comprehensive `README.md` file (covering the project scope, technical architecture, ISA specifications, build instructions, and authorship details) under the developer's direction.
- **Architectural Refactoring & Modularization**:
  - Refactored GUI components from monolithic single-header stb-style files into clean, decoupled C99 compilation units (`.h` declarations and `.c` definitions) with automatic CMake dependency tracking.
  - Implemented a dynamic Z-order windowing system using the Painter's algorithm with reverse hit-testing and bring-to-front focus stacking.
- **Concurrency & UI Responsiveness**:
  - Structured the multi-threaded emulator runtime (`pthread` worker thread synchronized with Raylib/Raygui) to ensure continuous instruction execution loops never block or stutter the rendering pipeline.
- **Component Polish**:
  - Completed the CPU Inspector view with full 32-register ABI naming, PC tracing, and status indicators.
  - Added viewport-clamped window dragging logic to floating panels.

---

## Roadmap

- [x] RV32I Base Integer Instruction Set
- [x] ELF32 Loader (`PT_LOAD` segments, entry point resolution)
- [x] Responsive Multi-Threaded GUI (Raylib + Raygui)
- [x] Dynamic Z-Order window management and draggable inspectors
- [x] CPU Inspector with ABI register names and status
- [ ] Interactive breakpoint execution trigger in the GUI
- [ ] RV32M Extension (Hardware Integer Multiplication and Division)
- [ ] Control and Status Register (CSR) subsystem (Zicsr)
- [ ] Privilege Levels (Machine, Supervisor, User modes)
- [ ] Basic memory management unit (Sv32 paging) to support bootable kernels

---

## References & Resources

- [Writing a Simple RISC-V Emulator in Plain C](https://fmash16.github.io/content/posts/riscv-emulator-in-c.html)
- [RISC-V ISA Reference (Cornell CS3410)](https://www.cs.cornell.edu/courses/cs3410/2025fa/rsrc/riscv-instructions-2.html)
- [Binary Loading Process in RISC-V Emulators](https://deepwiki.com/fmash16/riscv_emulator/7.1-binary-loading)
- [Extracting the Raw Contents of an ELF Section](https://www.baeldung.com/linux/file-elf-extract-raw-contents)
- [Official RISC-V User-Level ISA Specification](https://riscv.org/technical/specifications/)
