/*
 * lfo.h
 *
 *  Created on: 14.01.2013
 *
 *      Non bandlimited low freq. oscillator
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef LFO_H_
#define LFO_H_
//-------------------------------------------------------------
#include "stm32f7xx_hal.h"
#include "Soundengine/DSPAudio/Oscillator.h"
#include "Soundengine/DSPAudio/wavetable.h"	//sine
#include "Soundengine/config.h"
//#include "Soundengine/Sequencer/sequencer.h"
#include "Soundengine/DSPAudio/modulationNode.h"
//-------------------------------------------------------------
enum
{
	LFO_SINE,
	LFO_TRI,
	LFO_SAW_UP,
	LFO_SAW_DOWN,
	LFO_REC,
	LFO_NOISE,
	LFO_EXP_UP,
	LFO_EXP_DOWN,
	NUM_LFO_WAVEFORMS
};


#define LFO_MAX_F 		200 //[Hz]
#define LFO_SR 			(REAL_FS/(float)OUTPUT_DMA_SIZE)
//-------------------------------------------------------------
typedef struct LfoStruct
{
	uint32_t 	phase;		// the current phase of the LFO (upper bits cnt 0-255)
	uint32_t 	phaseInc;	// the phase increment controls the LFO frequency
	uint8_t 	waveform;	// selects the waveform
	uint8_t 	retrigger;	// defines the voice nr that retriggers the LFO (0=no retrigger)
	uint32_t 	phaseOffset;// the phase value to which the LFO is retriggered
	float 		rnd;
	uint8_t 	sync;
	float 		freq;
//	ModulationNode modTarget;
	float		modNodeValue;

	float value;
} Lfo;
//-------------------------------------------------------------
void lfo_init(Lfo *lfo);

void lfo_calcNextValue(Lfo* lfo);

void lfo_setFreq(Lfo *lfo, float f);
void lfo_setSync(Lfo* lfo, uint8_t sync);
void lfo_recalcSync();
void lfo_retrigger(uint8_t voiceNr);

#endif /* LFO_H_ */
