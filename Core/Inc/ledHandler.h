/*
 * ledHandler.h
 *
 * Created: 25.04.2012 16:01:19
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef LEDHANDLER_H_
#define LEDHANDLER_H_


#include "IO/DIO.h"


void led_init();

//turn on 1 of the 8 selection LEDS corresponding to the active page number
// input = [0:7]
void led_setActivePage(uint8_t pageNr);

void led_setActiveVoiceLeds(uint8_t pattern);
void led_setActiveVoice(uint8_t voiceNr);

void led_setActiveVoiceLedsTemp(uint8_t voiceNr);

void led_setActiveSelectButton(uint8_t butNr);
void led_clearLeds_1To8();

/** set the state of all select mode 2 leds*/
void led_setMode2Leds(uint8_t value);

void led_setValue(uint8_t val, uint8_t ledNr);
void led_setValueTemp(uint8_t val, uint8_t ledNr);

//void led_getValue(uint8_t ledNr);

void led_clearAll();

/** toggle a led*/
void led_toggle(uint8_t ledNr);

/** toggle a led temporary (can be reset to original state via led_reset() function)*/
void led_toggleTemp(uint8_t ledNr);
/** reset the led state to its original value (undo for temp operations)*/
void led_reset(uint8_t ledNr);
#if !EURORACK_MODULE
//update BarButtons in pattern Mode
void led_initPatternSelectLeds();

//update the sequencer leds with active steps

void led_updateSeqLeds();

void led_updateBarLeds();
/** clear all 16 seq leds*/
void led_clearSequencerLeds();

/** clear seq leds 9-16*/
void led_clearSequencerLeds9_16();
/** clear seq leds 1-8*/
void led_clearSequencerLeds1_8();

void led_clearSelectLeds();
#endif
void led_clearVoiceLeds();

void led_setMode2(uint8_t status);
void led_setPage(uint8_t page);

//uint8_t led_idx2SelectLed(uint8_t idx);

/**toggles the currently active step from the chase light indicator*/
#if !EURORACK_MODULE
void led_setActive_step(uint8_t stepNr);

void led_clearActive_step();
#endif

/**pulse a led.
this function flashes the LED once.
only NUM_OF_PULSABLE_LEDS (default 8) LEDs can be pulsed simultanously!*/
void led_pulseLed(uint8_t ledNr);

/** activate and deactivate blinking mode for LEDs*/
void led_setBlinkLed(const uint8_t ledNr, const uint8_t onOff);
void led_clearAllBlinkLeds();

/**this function needs to be called periodically to check if it is time to turn of a pulsed led*/
void led_tickHandler();


void led_blinkHandler();
void led_resetBlinkTimer();

//some LED helper functions for non clustered groups in LXR V2
#if !EURORACK_MODULE
uint8_t led_idx2StepLed(uint8_t idx);
uint8_t led_idx2SelectLed(uint8_t idx);
#endif

enum LedNumbers
{
	LED_EDIT,
	LED_PERF,
	LED_MOD,
	LED_FX,


	LED_1,
	LED_2,
	LED_3,
	LED_4,
	LED_5,
	LED_6,
	LED_7,
	LED_8,

	LED_SHIFT,
	LED_SAVE_LOAD,




};

#endif /* LEDHANDLER_H_ */
