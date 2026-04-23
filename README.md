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

The `afterdark-module` branch additionally includes an **After Dark 3.x screen saver module** — the full MDR work screen running as a proper Mac screen saver managed by After Dark.

### Building

Requires the [Retro68](https://github.com/autc04/Retro68) cross-compiler toolchain.

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=~/.retro68-deps/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake ..
make
```

Produces `MacLumon.dsk` (800 KB floppy image) and `MacLumon.bin` (MacBinary) in the `build/` directory.

Tested on a Macintosh SE/30 running System 7.5.3.
