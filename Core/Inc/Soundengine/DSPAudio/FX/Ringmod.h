/*
 * Ringmod.h
 *
 *  Created on: 26.08.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_SOUNDENGINE_DSPAUDIO_FX_RINGMOD_H_
#define INC_SOUNDENGINE_DSPAUDIO_FX_RINGMOD_H_


#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "../Oscillator.h"

typedef struct RingmodStruct
{

	float freq;	//drive
	OscInfo osc;


} Ringmod;



void Ringmod_init(Ringmod *rm);
void Ringmod_process(Ringmod *rm, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Ringmod_setFreq(Ringmod *rm, float f);
void Ringmod_setWave(Ringmod *rm, uint8_t w);
void Ringmod_resetPhase(Ringmod* rm);

#endif /* INC_SOUNDENGINE_DSPAUDIO_FX_RINGMOD_H_ */
