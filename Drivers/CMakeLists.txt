# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# STM32F7 HAL + CMSIS (vendor code, generated/copied by STM32CubeMX).
#
# Only the HAL modules enabled in Core/Inc/stm32f7xx_hal_conf.h are built.

add_library(stm32f7_hal OBJECT
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2s.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rng.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sd.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
    STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_sdmmc.c
)

target_include_directories(stm32f7_hal PUBLIC
    CMSIS/Include
    CMSIS/Device/ST/STM32F7xx/Include
    STM32F7xx_HAL_Driver/Inc
    STM32F7xx_HAL_Driver/Inc/Legacy
)

# lxr_board_config provides stm32f7xx_hal_conf.h (Core/Inc).
target_link_libraries(stm32f7_hal PUBLIC lxr_mcu lxr_board_config)

# Vendor code: don't drown the build log in warnings we won't fix.
target_compile_options(stm32f7_hal PRIVATE -w)
