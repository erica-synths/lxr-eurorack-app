# LXR V2 Eurorack firmware (STM32F765VG)

Firmware for the LXR V2 Eurorack drum module: an STM32F765 (Cortex-M7, 216 MHz) running the drum
voices, FX bus, trigger and CV inputs, MIDI, SD-card kit storage and the LCD/encoder UI.

> **Licence and trademarks:** the firmware source code is free software under the GNU GPL version 3
> ([LICENSE](LICENSE)). The names **"Erica Synths" and "LXR"** and the **LXR hardware** are the property
> of Erica Synths and are **not** covered by this licence. Building and installing your own firmware is
> at your own risk: Erica Synths provides no support for self-built or modified firmware.
> See [NOTICE.md](NOTICE.md).

## Building

You need [CMake](https://cmake.org/download/) 3.21+, [Ninja](https://ninja-build.org/), the
[Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
(`arm-none-eabi-gcc`, tested with 14.3.rel1) and [Python 3](https://www.python.org/downloads/), all on `PATH`.

- macOS: `brew install cmake ninja && brew install --cask gcc-arm-embedded`. Don't use
  `brew install arm-none-eabi-gcc`: it lacks the C library and the build fails with it.
- Windows: `winget install Kitware.CMake Ninja-build.Ninja Python.Python.3.12` plus Arm's installer
  (tick "Add path to environment variable").
- Linux: `sudo apt install cmake ninja-build python3` plus Arm's toolchain, with its `bin` folder on `PATH`.

Then, in this folder:

```sh
cmake --preset release
cmake --build --preset release
```

The update file is written to `build/release/LXRE_V2_V<version>.img`. The version is set by
`project(VERSION ...)` in [CMakeLists.txt](CMakeLists.txt) and shown on the start-up screen.

| Option | Effect |
|---|---|
| `cmake --preset debug` | Debug build (`-O3 -g3`, `DEBUG` defined) in `build/debug/` |
| `-DARM_TOOLCHAIN_DIR=<bin folder>` | Use a toolchain that isn't on `PATH` |
| `-DLXR_ITCM_OPT=-O2` | Lower optimisation for code in the 16 KB ITCM, if a change or compiler makes it overflow at `-O3` |
| `-DLXR_GENERATE_LISTING=ON` | Also write a disassembly listing (`.list`) |

To start from scratch (e.g. after switching compilers), delete the `build/` folder.

## Installing the update

1. Copy `LXRE_V2_V<version>.img` to the top folder of a FAT32 SD card and delete any older `LXRE*.img`
   files from it.
2. With the module powered off, insert the card, hold down the encoder button and power on. Keep
   holding until "LXR Updater" appears.
3. Wait for "Update OK!". The module restarts with the new firmware.

The updater (bootloader) is never changed by an update, so a failed or interrupted update can simply be
repeated. The module's updater only looks for files named `LXRE*.img`, so desktop LXR update files on
the same card are ignored.

## Programming with a debug probe

Not needed for updates. With an ST-Link and [OpenOCD](https://openocd.org/) on `PATH`:

```sh
cmake --build --preset release --target flash        # firmware only, bootloader untouched
```

For blank devices, configure with the Eurorack bootloader, either from source
(`-DLXR_BOOTLOADER_DIR=path/to/LXR_V2_STM32F765_Bootloader`, built with `LXR_BOOTLOADER_EURORACK`) or
prebuilt (`-DLXR_BOOTLOADER_HEX=path/to/LXR_V2_Eurorack_Bootloader.hex`). The build then also writes
`LXRE_V2_V<version>_full.hex/.bin` (whole flash from 0x08000000) and enables the `flash_full` target.

> **Warning:** `flash_full` overwrites the bootloader and asks you to type `yes` first (or set
> `LXR_CONFIRM_BOOTLOADER_OVERWRITE=yes`). If it is interrupted, the module can only be recovered with a
> debug probe.

## Memory map

| Region | Address | Size | Contents |
|---|---|---|---|
| FLASH | 0x08000000 | 32 KB | Bootloader |
| FLASH | 0x08008000 | 992 KB | This firmware |
| ITCM | 0x00000000 | 16 KB | `IN_ITCM_RAM` DSP code |
| DTCM | 0x20000000 | 128 KB | `INDTCM`/`INDTCMZ` data, DMA buffers, stack |
| RAM | 0x20020000 | 384 KB | `.data`, `.bss`, heap |

## Source layout

Originally an STM32CubeIDE project; the CubeMX `.ioc` was removed because regenerating code from it
overwrites application code. Pins, clocks and peripherals live in `Core/Inc/main.h`, `Core/Src/main.c`
and `Core/Src/stm32f7xx_hal_msp.c`. Source files are listed explicitly in the `CMakeLists.txt` files
(`Core/` is split into `lxr_platform`, `lxr_io`, `lxr_ui`, `lxr_preset`, `lxr_dsp`, `lxr_engine` and
`lxr_midi`); add new files there.

## Licence

Copyright (C) 2012-2026 Erica Synths. Licensed under the [GNU GPL version 3](LICENSE) (GPL-3.0-only)
with the additional permission and terms in [NOTICE.md](NOTICE.md): modified firmware must be marked as
modified and must not be presented as official Erica Synths firmware. The trademarks and the hardware
are not licensed. Third-party components (STMicroelectronics, Arm, ChaN FatFs) keep their own licences,
see [NOTICE.md](NOTICE.md#third-party-components).
