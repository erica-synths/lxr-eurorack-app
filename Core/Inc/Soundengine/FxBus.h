/*
 * FxBus.h
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

#ifndef INC_SOUNDENGINE_FXBUS_H_
#define INC_SOUNDENGINE_FXBUS_H_

#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/FX/Distortion.h"
#include "Soundengine/DSPAudio/FX/Ringmod.h"
#include "Soundengine/DSPAudio/FX/Compressor.h"
#include "Soundengine/DSPAudio/FX/Delay.h"
#include "Soundengine/DSPAudio/FX/Reverb.h"

#define USE_REVERB 0

enum FxTypeEnum
{
	FX_OFF,
	FX_DISTORTION,
	FX_RING,
	FX_COMPRESSOR,
	FX_DELAY,
#if USE_REVERB
	FX_REVERB
#endif
};

typedef struct FxStruct
{

	uint8_t fxType;
	uint8_t audioRouting;
	float dryWet;

	float parameter[4];


	MasterDistortion distortion;
	Ringmod ringmod;
	Compressor compressor;
	Delay delay;



#if USE_REVERB
	Reverb reverb;
#endif

} FxBus;

extern INDTCMZ FxBus fxBus;

void FxBus_init();
void FxBus_process(int16_t *fxBuffer, int16_t* outL,int16_t* outR,int16_t* outL2, int16_t* outR2, uint32_t size);
void FxBus_setParameter(uint8_t idx, float value);
void FxBus_updateParameter();
//to prevent NaN pollution from delay
void FxBus_prepareFxChange();


#endif /* INC_SOUNDENGINE_FXBUS_H_ */
