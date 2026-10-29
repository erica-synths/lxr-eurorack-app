# Copyright (C) 2026 Erica Synths
# SPDX-License-Identifier: GPL-3.0-only

# LXR V2 Eurorack application code, split into modules.
#
# The modules call into each other in a cycle (menu <-> preset <-> DSP), so
# they are OBJECT libraries that are all linked directly into the firmware
# executable rather than static libraries with a link order.

add_library(lxr_app_common INTERFACE)
target_link_libraries(lxr_app_common INTERFACE
    lxr_mcu
    lxr_board_config
    stm32f7_hal
    fatfs
)
target_compile_options(lxr_app_common INTERFACE $<$<COMPILE_LANGUAGE:C>:-Wall>)

# Startup, clocks, CubeMX peripheral init, interrupt vectors, newlib stubs.
add_library(lxr_platform OBJECT
    Startup/startup_stm32f765vgtx.s
    Src/main.c
    Src/stm32f7xx_it.c
    Src/stm32f7xx_hal_msp.c
    Src/system_stm32f7xx.c
    Src/syscalls.c
    Src/sysmem.c
    Src/delay.c
)
# Shown on the boot splash screen; comes from project(VERSION) in the top-level CMakeLists.txt.
set_source_files_properties(Src/main.c PROPERTIES
    COMPILE_DEFINITIONS "FIRMWARE_VERSION=\"${PROJECT_VERSION}\""
)

# Hardware inputs/outputs: shift-register buttons/LEDs, pots and CV inputs.
add_library(lxr_io OBJECT
    Src/IO/adcPots.c
    Src/IO/DIO.c
    Src/IO/uart.c
)

# User interface: LCD, encoder, buttons, LEDs, menu pages, screensaver.
add_library(lxr_ui OBJECT
    Src/lcd.c
    Src/encoder.c
    Src/buttonHandler.c
    Src/ledHandler.c
    Src/graphic.c
    Src/frontPanelParser.c
    Src/Parameters.c
    Src/menu/Cc2Text.c
    Src/menu/menu.c
    Src/menu/screensaver.c
)

# Kit/project storage on the SD card.
add_library(lxr_preset OBJECT
    Src/Preset/initKit.c
    Src/Preset/presetManager.c
)

# DSP building blocks, drum voices and master effects.
add_library(lxr_dsp OBJECT
    Src/Soundengine/DSPAudio/1PoleLp.c
    Src/Soundengine/DSPAudio/automationNode.c
    Src/Soundengine/DSPAudio/BufferTools.c
    Src/Soundengine/DSPAudio/CymbalVoice.c
    Src/Soundengine/DSPAudio/Decay.c
    Src/Soundengine/DSPAudio/distortion.c
    Src/Soundengine/DSPAudio/dither.c
    Src/Soundengine/DSPAudio/DrumVoice.c
    Src/Soundengine/DSPAudio/HiHat.c
    Src/Soundengine/DSPAudio/lfo.c
    Src/Soundengine/DSPAudio/mixer.c
    Src/Soundengine/DSPAudio/modulationNode.c
    Src/Soundengine/DSPAudio/Oscillator.c
    Src/Soundengine/DSPAudio/random.c
    Src/Soundengine/DSPAudio/ResonantFilter.c
    Src/Soundengine/DSPAudio/Samples.c
    Src/Soundengine/DSPAudio/SlopeEg2.c
    Src/Soundengine/DSPAudio/snapEg.c
    Src/Soundengine/DSPAudio/Snare.c
    Src/Soundengine/DSPAudio/squareRootLut.c
    Src/Soundengine/DSPAudio/TiltEq.c
    Src/Soundengine/DSPAudio/transientGenerator.c
    Src/Soundengine/DSPAudio/transientTables.c
    Src/Soundengine/DSPAudio/wavetable.c
    Src/Soundengine/DSPAudio/FX/Compressor.c
    Src/Soundengine/DSPAudio/FX/Delay.c
    Src/Soundengine/DSPAudio/FX/Distortion.c
    Src/Soundengine/DSPAudio/FX/FxRamBuffer.c
    Src/Soundengine/DSPAudio/FX/Reverb.c
    Src/Soundengine/DSPAudio/FX/Ringmod.c
    Src/Soundengine/DSPAudio/FX/VerbTools/AllpassDelay.c
    Src/Soundengine/DSPAudio/FX/VerbTools/sinCos.c
    Src/Soundengine/DSPAudio/FX/VerbTools/VerbDelay.c
    Src/Soundengine/DSPAudio/FX/VerbTools/VerbModDelay.c
)

# Audio engine: codec/I2S DMA, sample block rendering, FX bus, trigger and
# CV modulation inputs.
add_library(lxr_engine OBJECT
    Src/Soundengine/AudioCodecManager.c
    Src/Soundengine/cs4344_cs5343.c
    Src/Soundengine/FIFO.c
    Src/Soundengine/FxBus.c
    Src/Soundengine/ModMatrix_Processor.c
    Src/Soundengine/Soundengine.c
    Src/Soundengine/TriggerProcessor.c
)

# MIDI: UART driver, parsers, parameter mapping, voice control.
add_library(lxr_midi OBJECT
    Src/Soundengine/MIDI/CortexParser.c
    Src/Soundengine/MIDI/MidiMessages.c
    Src/Soundengine/MIDI/MidiParser.c
    Src/Soundengine/MIDI/MidiVoiceControl.c
    Src/Soundengine/MIDI/ParameterArray.c
    Src/Soundengine/MIDI/Uart.c
)

set(LXR_APP_MODULES
    lxr_platform
    lxr_io
    lxr_ui
    lxr_preset
    lxr_dsp
    lxr_engine
    lxr_midi
)
foreach(module IN LISTS LXR_APP_MODULES)
    target_link_libraries(${module} PUBLIC lxr_app_common)
endforeach()
set(LXR_APP_MODULES ${LXR_APP_MODULES} PARENT_SCOPE)

# --- ITCM fit -----------------------------------------------------------------
# Functions tagged IN_ITCM_RAM are linked into the 16 KB instruction TCM. Newer
# GCC releases generate more code at -O3 than the GCC 10/11 the project was
# developed with, which overflows ITCM. LXR_ITCM_OPT overrides the optimisation
# level for just the translation units that contain ITCM code.
set(LXR_ITCM_SOURCES
    Src/Preset/presetManager.c
    Src/Soundengine/cs4344_cs5343.c
    Src/Soundengine/FxBus.c
    Src/Soundengine/DSPAudio/1PoleLp.c
    Src/Soundengine/DSPAudio/CymbalVoice.c
    Src/Soundengine/DSPAudio/Decay.c
    Src/Soundengine/DSPAudio/distortion.c
    Src/Soundengine/DSPAudio/DrumVoice.c
    Src/Soundengine/DSPAudio/HiHat.c
    Src/Soundengine/DSPAudio/lfo.c
    Src/Soundengine/DSPAudio/mixer.c
    Src/Soundengine/DSPAudio/modulationNode.c
    Src/Soundengine/DSPAudio/Oscillator.c
    Src/Soundengine/DSPAudio/ResonantFilter.c
    Src/Soundengine/DSPAudio/SlopeEg2.c
    Src/Soundengine/DSPAudio/Snare.c
    Src/Soundengine/DSPAudio/transientGenerator.c
    Src/Soundengine/DSPAudio/FX/Distortion.c
    Src/Soundengine/DSPAudio/FX/Reverb.c
    Src/Soundengine/MIDI/ParameterArray.c
)
if(LXR_ITCM_OPT)
    set_source_files_properties(${LXR_ITCM_SOURCES} PROPERTIES COMPILE_OPTIONS "${LXR_ITCM_OPT}")
endif()


set_property(SOURCE Src/Soundengine/DSPAudio/ResonantFilter.c APPEND PROPERTY COMPILE_OPTIONS -fno-thread-jumps)