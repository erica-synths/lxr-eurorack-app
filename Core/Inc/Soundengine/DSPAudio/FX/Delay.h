/*
 * simpleDelay.h
 *
 *  Created on: 29.11.2013
 *      Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef SIMPLEDELAY_H_
#define SIMPLEDELAY_H_


#include "../1PoleLp.h"
#include "Soundengine/DSPAudio/FX/FxRamBuffer.h"
/*
#include "filter.h"
#include "shaper.h"
#include "Redux.h"
#*/
//#include "OtaFourPole.h"
/*
 * size =
 * variation =
 * composition = filter LP
 */

#define SECONDS_TO_SAMPLES(s) (s*REAL_FS)

#define LOW_MIN SECONDS_TO_SAMPLES(0.0006)
#define LOW_MAX SECONDS_TO_SAMPLES(0.02)

#define HI_MIN SECONDS_TO_SAMPLES(0.02)
#define HI_MAX FX_BUFFER_SIZE //SECONDS_TO_SAMPLES(6)

enum DelayTypesEnum
{
	DELAY_NORMAL,
	DELAY_PINGPONG
};

typedef struct DelayStruct
{

	uint32_t oldRead;
	uint32_t newRead;
	uint32_t write;
	float fracReadNew,fracReadOld;
	float interpValue;
	float interpStep;
	int16_t tmpBuffer[2];
	int16_t tmpBuffer2[2]; //used for stereo or pingpong delay
	float feedback;
	float range;

	float delayTime;
	float pendingDelayTime;
	uint8_t timePending;

	OnePoleFilter tone;
	OnePoleFilter tone2;
	//RingBuffer timeBuffer;
	uint8_t delayType;

} Delay;


void Delay_init(Delay* d);
void Delay_calcBuffer(Delay* d, uint8_t bufferIdx);
void Delay_process(Delay *d, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Delay_processPingPong(Delay *d, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Delay_setDelayTime(Delay* d, float t);
void Delay_setFeedback(Delay* d, float fb);
void Delay_setRange(Delay* d, float r);
void Delay_setFilter(Delay* d, float f);





#endif /* SIMPLEDELAY_H_ */
