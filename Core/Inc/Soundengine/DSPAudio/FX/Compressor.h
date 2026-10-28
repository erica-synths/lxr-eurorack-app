/*
 * Compressor.h
 *
 *  Created on: Aug 27, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_SOUNDENGINE_DSPAUDIO_FX_COMPRESSOR_H_
#define INC_SOUNDENGINE_DSPAUDIO_FX_COMPRESSOR_H_
#include "Soundengine/globals.h"
#include "Soundengine/config.h"


typedef struct CompressorStruct
{

	float tresh;
	float attack;
	//float slope;
	float release;
	float ratio;
	float markupGain;

	//double filter;
	// envelope
	double  env;
	//double rms;


} Compressor;



void Compressor_init(Compressor *c);
void Compressor_process(Compressor *c, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Compressor_setAttack(Compressor *c, float a);
void Compressor_setTreshold(Compressor *c, float t);
void Compressor_setRelease(Compressor *c, float t);
void Compressor_setRatio(Compressor *c, uint8_t r);
void Compressor_setGain(Compressor *c, float g);

#endif /* INC_SOUNDENGINE_DSPAUDIO_FX_COMPRESSOR_H_ */
