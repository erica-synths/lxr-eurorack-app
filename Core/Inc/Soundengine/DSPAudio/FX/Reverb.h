/*
 * ReverbAlgo.h
 *
 *  Created on: 30.07.2014
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef REVERBALGO_H_
#define REVERBALGO_H_
#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/FX/FxRamBuffer.h"


#include "Delay.h"
#include "Soundengine/DSPAudio/TiltEq.h"
#include "Soundengine/DSPAudio/FX/VerbTools/AllpassDelay.h"
#include "Soundengine/DSPAudio/1PoleLp.h"
#include "Soundengine/DSPAudio/FX/VerbTools/sinCos.h"
#include "Soundengine/DSPAudio/FX/VerbTools/VerbModDelay.h"
#include "Soundengine/DSPAudio/TiltEq.h"
//#include "shaper.h"


#define MAX_DELAY_SIZE MAX_ALLPASS_SIZE//(0x800)
#define EARLY_TAPS 4

#define USE_OUT_2 0
#define USE_LOWER_DENSITY 1
#define HIGH_RES_SINE_COS 0
#define SPIN_SEMI 0


typedef struct ReverbStruct
{
#if SPIN_SEMI
	AllpassDelay apDelay[4][2];
#else
	AllpassDelay apDelay[2];
	AllpassModDelay apModDelay[2];
#endif
	AllpassDelay inputAllpass[4];

#if SPIN_SEMI
	VerbDelay delay[2];
	VerbModDelay modDelay[2];
#else
	VerbDelay delay[4];
#endif
	VerbDelay earlyReflectionDelay;
	VerbDelay preDelay;

	SineCosine lfo;

	float feedback;
	float delayTime;
	float earlyVolume;

	OnePoleFilter lpFeedback;
	OnePoleFilter lpImpulse;

	volatile __fp16 *reverbBufferSram;

	float lastOut;
	float lastOut2;

	TiltEq eq;
	TiltEq eqFb;

} Reverb;

void Reverb_init(Reverb *r);
void Reverb_process(Reverb *r, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Reverb_setDelayTime(Reverb *r, float t);
void Reverb_setFeedback(Reverb *r, float fb);
void Reverb_setVariation(Reverb *r, float v);
void Reverb_setComposition(Reverb *r, float c);


/*
 * feedback = decay
 * time = pre delay
 * dry/wet = dry/wet
 *
 * size = room size
 * variation = diffusion
 * composition = damping
 */
/*
class AllpassLoopVerb : public Delay
{
public:
	AllpassLoopVerb();
	virtual ~AllpassLoopVerb(){};

private:
	  AllpassDelay<revItg> apDelay[4][2];
	  AllpassDelay<revItg> inputAllpass[4];

	  VerbDelay<revItg,MAX_DELAY_SIZE> delay[2];
	  VerbModDelay<revItg,MAX_DELAY_SIZE> modDelay[2];
	  VerbDelay<revItg,MAX_DELAY_SIZE> earlyReflectionDelay;
	  SineCosine lfo;
	  OnePoleLp lpFeedback;
	  OnePoleLp lpImpulse;

	  float lastOut;

	  Shaper feedbackShaper;

	  volatile revItg *reverbBufferSram;

	  inline revItg add(revItg a, revItg b)
	  {
		  return __QADD(a,b);
	  }

	  inline revItg sub(revItg a, revItg b)
	  {
		  return __QSUB(a,b);
	  }

public:
	void process(int16_t* input1, int16_t* input2);
	void setDelayTime(float t);
	void setComposition(float c);
	void setFeedback(float fb);
	void setVariation(float v);

};

*/

#endif /* REVERBALGO_H_ */
