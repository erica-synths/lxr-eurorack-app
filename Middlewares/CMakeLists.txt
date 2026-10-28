# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# Middleware: FatFs (SD card).
#
# Built as an OBJECT library, like the rest of the firmware, so every object is
# linked directly exactly as in the original STM32CubeIDE build (a static
# archive could silently prefer a __weak default over an override).

# --- FatFs + CubeMX SD glue (FATFS/App, FATFS/Target) -------------------------
add_library(fatfs OBJECT
    Third_Party/FatFs/src/diskio.c
    Third_Party/FatFs/src/ff.c
    Third_Party/FatFs/src/ff_gen_drv.c
    Third_Party/FatFs/src/option/syscall.c
    ${PROJECT_SOURCE_DIR}/FATFS/App/fatfs.c
    ${PROJECT_SOURCE_DIR}/FATFS/Target/bsp_driver_sd.c
    ${PROJECT_SOURCE_DIR}/FATFS/Target/fatfs_platform.c
    ${PROJECT_SOURCE_DIR}/FATFS/Target/sd_diskio.c
)

target_include_directories(fatfs PUBLIC
    Third_Party/FatFs/src
    ${PROJECT_SOURCE_DIR}/FATFS/App
    ${PROJECT_SOURCE_DIR}/FATFS/Target
)

target_link_libraries(fatfs PUBLIC stm32f7_hal)
target_compile_options(fatfs PRIVATE -w)
