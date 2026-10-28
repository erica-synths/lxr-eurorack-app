/*
 * CymbalVoice.h
 *
 *  Created on: 16.06.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef CYMBALVOICE_H_
#define CYMBALVOICE_H_

#include "stm32f7xx_hal.h"
#include "Oscillator.h"
#include "config.h"
#include "ResonantFilter.h"
#include "SlopeEg2.h"
#include "random.h"
#include "transientGenerator.h"
#include "BufferTools.h"
#include "lfo.h"
#include "distortion.h"
#include "snapEg.h"
#include "Soundengine/DSPAudio/1PoleLp.h"

typedef struct CymbalStruct
{

	OscInfo 	osc;		// the tonal oscillator
	OscInfo 	modOsc;		// the mod osc1
	OscInfo 	modOsc2;	// the mod osc2
	float		fmModAmount1;
	float		fmModAmount2;

	float	 	vol;		// volume of the voice
	//float		panL;		// [0:1]
	//float		panR;		// [0:1]
	uint8_t pan;
	float 	triggeredPanL; //only updated with the latest pan value when triggered
	float 	triggeredPanR;
	//float 		panModifier;

	float 		velo;

	int32_t		noiseSample;

	ResonantFilter filter;
	uint8_t		filterType;

	Lfo 		lfo;
	TransientGenerator transGen;
	Distortion distortion;

	SlopeEg2	oscVolEg;
	float 		egValueOscVol;

	uint8_t 	volumeMod;	//modulate volume by midi velocity if 1

	SnapEg 		snapEg;

	float combinedVolume;
	//OnePoleFilter FaderVolumeFilter;

	float pwm;
	float cvFreq;

	float velocity; //last accent value

} CymbalVoice;

extern CymbalVoice cymbalVoice;


//initialize all the parameters to sane values
void Cymbal_init();
void Cymbal_setCv1VOct(float oct);
void Cymbal_trigger( const uint8_t vel, const uint8_t note);
void Cymbal_updatePan();
void Cymbal_calcSyncBlock(int16_t* buf, const uint8_t size);

/** calculate envelopes etc (all 16 samples */
void Cymbal_calcAsync();

//-0.5 = left 0=both max 0.5=right*/
void Cymbal_setPan(const uint8_t pan);


#endif /* CYMBALVOICE_H_ */
