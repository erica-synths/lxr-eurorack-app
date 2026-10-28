/*
 * ResonantFilter.h
 *
 *  Created on: 05.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef RESONANTFILTER_H_
#define RESONANTFILTER_H_

/*
 * ResonantFilter.c
 *
 *  Created on: 05.04.2012
 *      Author: Julian
 */
//----------------------------------------------------
#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "stm32f7xx_hal.h"
#include "Soundengine/datatypes.h"
#include "math.h"
#include "Soundengine/DSPAudio/distortion.h"
//----------------------------------------------------
#define ENABLE_NONLINEAR_INTEGRATORS 	0
#define FILTER_GAIN 					0x7fff

#define USE_SHAPER_NONLINEARITY 0


//----------------------------------------------------
enum filterTypeEnum
{
	FILTER_LP=1,
	FILTER_HP,
	FILTER_BP,
	FILTER_UNITY_BP,
	FILTER_NOTCH,
	FILTER_PEAK,
	FILTER_NAIVE_2_POLE,
	FILTER_OFF
};
//----------------------------------------------------
typedef struct ResoFilterStruct
{
	float f;	/**< cutoff frequency as integer from 0 to 0xffff where 0xffff = SR*/
	float g;    /**< embedded integrator gain (Fig 3.11), wc == wa*/
	float q;	/**< q value calculated from setReso()*/

	float s1;
	float s2;

	float a,b;
#if ENABLE_NONLINEAR_INTEGRATORS
	float zi;	//input z^(-1)
#endif

	float drive;

#if USE_SHAPER_NONLINEARITY
	Distortion shaper;
#endif
} ResonantFilter;
//------------------------------------------------------------------------------------
void SVF_setReso(ResonantFilter* filter, float feedback);
//------------------------------------------------------------------------------------
void SVF_init();
//------------------------------------------------------------------------------------
void SVF_setDrive(ResonantFilter* filter, float drive);
//------------------------------------------------------------------------------------
void SVF_directSetFilterValue(ResonantFilter* filter, float val);
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_calcBlockZDF(ResonantFilter* filter, const uint8_t type, int16_t* buf, const uint8_t size);
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_calcBlockCheapZDF(ResonantFilter* filter, const uint8_t type, int16_t* buf, const uint8_t size);
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_recalcFreq(ResonantFilter* filter);
//------------------------------------------------------------------------------------
void SVF_reset(ResonantFilter* filter);

#endif /* RESONANTFILTER_H_ */
