![Lumon][mac-screenshot]

# MacLumon
A Simulation of LUMON's Macrodata Refinement UI for Classic Macs.

A hack for [MARCHintosh 2026]

[mac-screenshot]: https://github.com/marciot/MacLumon/raw/main/images/mdr_animation.gif "Macrodata Refinement Animation"
[MARCHintosh 2026]: https://marchintosh.com

---

## This fork — [reisfni/MacLumon](https://github.com/reisfni/MacLumon)

All credit for the original application goes to [marciot](https://github.com/marciot/MacLumon). This fork adds:

- **Auto-click attract/demo mode** — after a 5-second startup delay the app simulates a worker clicking numbers on its own, so it runs unattended on display hardware. A real click pauses the demo for 60 seconds before it resumes.
- **Screen saver with burn-in protection** — after 30 minutes of inactivity the bouncing Lumon globe screen saver activates automatically, then returns to the work screen after 15 seconds.
- **Build system** — a `CMakeLists.txt` for building with the [Retro68](https://github.com/autc04/Retro68) cross-compiler toolchain on a modern machine, plus the decoded `Lumon.rsrc.as` resource file needed by the build.

This branch also includes an **After Dark 3.x screen saver module** — see below.

### Download

Pre-built disk images are available on the [Releases page](https://github.com/reisfni/MacLumon/releases):

- `MacLumon.dsk` — 800 KB floppy disk image (use with an emulator or write to a real floppy)
- `MacLumon.bin` — MacBinary file for direct transfer to a Mac
- `LumonAD.dsk` — After Dark module disk image
- `LumonAD.bin` — After Dark module MacBinary file

### Building from source

Requires the [Retro68](https://github.com/autc04/Retro68) cross-compiler toolchain.

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=~/.retro68-deps/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake ..
make
```

Tested on a Macintosh SE/30 running System 7.5.3.

---

## After Dark module

This branch includes a screen saver module for [After Dark 3.x](https://en.wikipedia.org/wiki/After_Dark_(software)). Instead of just a bouncing logo, the module runs the full MDR work screen — number grid, progress bars, bin labels, Lumon globe header — with the auto-clicking attract mode, all managed by After Dark.

### Installing

1. Download `LumonAD.dsk` from the [Releases page](https://github.com/reisfni/MacLumon/releases) and copy `LumonAD` to your Mac, or transfer `LumonAD.bin` directly
2. Copy the `LumonAD` file into `System Folder → After Dark Files`
3. Open the After Dark control panel — **Lumon Industries** will appear in the module list
4. Select it and After Dark will handle the rest

### How it works

The module is compiled as an `ADgm` code resource (file type `ADGM`) using the same Retro68 toolchain as the main application. It shares the WorkMode and Logo drawing code, so the screen saver looks identical to the standalone app.
