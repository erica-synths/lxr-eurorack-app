/*
 * TriggerOut.h
 *
 *  Created on: 06.05.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#define VOICE_TRIGGER_OUTS 0


#ifndef TRIGGEROUT_H_
#define TRIGGEROUT_H_

#include "Soundengine/globals.h"
/*
#define PIN_TRACK_1 GPIO_Pin_8	//Port D
#define PIN_TRACK_2 GPIO_Pin_9	//Port D
#define PIN_TRACK_3 GPIO_Pin_10	//Port D
#define PIN_TRACK_4 GPIO_Pin_11	//Port D
#define PIN_TRACK_5 GPIO_Pin_12	//Port D
#define PIN_TRACK_6 GPIO_Pin_13	//Port D
#define PIN_TRACK_7 GPIO_Pin_14	//Port D

#define PIN_CLOCK_1 GPIO_Pin_15	//Port D
#define PIN_CLOCK_2 GPIO_Pin_9	//Port A
#define PIN_RESET GPIO_Pin_10	//Port A

#define PIN_CLOCK_IN GPIO_Pin_9 //Port C
#define PIN_RESET_IN GPIO_Pin_8 //Port A
*/

#define LOW_VOLUME 63
#define PULSE_LENGTH_MS 20 //[ms]
#define PULSE_LENGTH (PULSE_LENGTH_MS*4) //systick 0.25ms

#define HANDLE_SEQ_IN_CLOCK_INTERRUPT 0

#if !HANDLE_SEQ_IN_CLOCK_INTERRUPT
extern uint8_t trigger_clockReceived;
#endif

enum
{
	TRIGGER_1 = 0,
	TRIGGER_2,
	TRIGGER_3,
	TRIGGER_4,
	TRIGGER_5,
	TRIGGER_6,
	TRIGGER_7,

	CLOCK_1,
	CLOCK_2,
	TRIGGER_RESET,
	TRIGGER_ALL,
	NUM_PINS,
};


// since we have 1 main steps per quarter
// our native resolution is 4ppq
enum PrescalerPpqIn
{
	PRE_4_PPQ_IN	= 1,
//	PRE_8_PPQ_IN	= 2,
//	PRE_16_PPQ_IN	= 4,
	PRE_24_PPQ_IN	= 2,//6,
};

enum PrescalerPpqOut
{
	PRE_2_PPQ_OUT,
	PRE_4_PPQ_OUT,
	PRE_8_PPQ_OUT,
	PRE_16_PPQ_OUT,
	PRE_24_PPQ_OUT,
	PRE_48_PPQ_OUT,

};

typedef enum TriggerModes
{
	TRIGGER_ON,
	TRIGGER_OFF,
	TRIGGER_PULSE
} triggerMode;

extern uint8_t trigger_dividerClockOut1;
//extern uint8_t trigger_dividerClockOut2;
extern uint8_t trigger_prescalerClockInput;

uint8_t Trigger_getAccent(uint8_t voice);

uint8_t Trigger_getVoiceTriggered(uint8_t voice);
void Trigger_resetVoiceTriggered(uint8_t voice);

//void trigger_init();
//void trigger_tick();
#if VOICE_TRIGGER_OUTS
void trigger_triggerVoice(uint8_t voice, triggerMode mode);
#endif
void trigger_clockTick(uint8_t onOff, uint8_t cnt);//the sequencer calls this function whenever a step is played to generate corresponding trigger out clocks
//void trigger_reset(uint8_t value);
//void trigger_tickPhaseCounter();
//uint8_t trigger_isGateModeOn();
//void trigger_setGatemode(uint8_t onOff);	//if gate mode is on, the trigger out will be high until a note off is send
//void trigger_allOff();
//void trigger_ResetPosition();


#endif /* TRIGGEROUT_H_ */

