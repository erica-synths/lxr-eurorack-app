/*
 * DrumVoice.h
 *
 *  Created on: 03.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef DRUMVOICE_H_
#define DRUMVOICE_H_

#include "Soundengine/DSPAudio/Oscillator.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/ResonantFilter.h"
#include "Soundengine/DSPAudio/distortion.h"
#include "Soundengine/DSPAudio/SlopeEg2.h"
#include "Soundengine/DSPAudio/Decay.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/DSPAudio/lfo.h"
#include "Soundengine/DSPAudio/1PoleLp.h"
#include "Soundengine/DSPAudio/transientGenerator.h"
#include "Soundengine/DSPAudio/snapEg.h"
#include "Soundengine/DSPAudio/dither.h"
//------------------------------------------------------------------------
enum
{
	IS_PLAYING = 0x1,
};
//------------------------------------------------------------------------
typedef struct VoiceStruct
{
	OscInfo 	osc;		// the main fm oscillator
	OscInfo 	modOsc;		// the mod osc

	float		fmModAmount;

	float	 	vol;		// volume of the voice
	float 		velo;
	uint8_t 	pan;
	float 	triggeredPanL; //only updated with the latest pan value when triggered
	float 	triggeredPanR;
	//float 		panModifier;

	int16_t 	oscSample;

	DecayEg		oscPitchEg;
	float		egPitchModAmount;
	float 		offset;				//the start offset for the waveform - used to generate clicks

	TransientGenerator transGen;
	Lfo 		lfo;
	SlopeEg2	oscVolEg;
	float 		egValueOscVol;
	float 		volEgValueBlock[OUTPUT_DMA_SIZE];
	Distortion distortion;

#if ENABLE_DRUM_SVF
	ResonantFilter filter;
	uint8_t		filterType; // bit 0 = lp, bit 1 = hp, bit 3 = bp on/off
#endif

#ifdef USE_AMP_FILTER
	OnePoleFilter ampFilter;
#endif

#if (AMP_EG_SYNC==0)
	double ampFilterInput;
	double lastGain;
#endif

#if ENABLE_MIX_OSC
	uint8_t mixOscs;
#endif

	float decimationCnt;
	float decimationRate;
	SnapEg snapEg;

	uint8_t volumeMod;	//modulate volume by midi velocity if 1

	Dither dither;
	float combinedVolume;
//	OnePoleFilter FaderVolumeFilter;

#if SMOOTH_MIXER_VOL_PAN_DRIVE
	OnePoleFilter MixerPageFilter[2];
#endif

	float pwm;
	float cvFreq;

	float velocity; //last accent value
} DrumVoice;
//------------------------------------------------------------------------


//array holding all the voices
extern DrumVoice voiceArray[NUM_DRUM_VOICES];

//initialize all the parameters to sane values
void initDrumVoice();

void Drum_setCv1VOct(const uint8_t voiceNr, float oct);

void Drum_trigger(const uint8_t voiceNr, const uint8_t vol, const uint8_t note);

void Drum_updatePan(const uint8_t voiceNr);

/** block based calculation*/
void calcDrumVoiceSyncBlock(const uint8_t voiceNr, int16_t* buf, const uint8_t size);

/** calculate envelopes etc (all 16 samples */
void calcDrumVoiceAsync(const uint8_t voiceNr);

//-0.5 = left 0=both max 0.5=right*/
void setPan(const uint8_t voiceNr, const uint8_t pan);

/** set the oscillator start phase for all OSCs*/
void drum_setPhase(const uint8_t phase, const uint8_t voiceNr);

#endif /* DRUMVOICE_H_ */
