/*
 * 1PoleLp.c
 *
 *  Created on: 08.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

//-------------------------------------------------------------
#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "stm32f7xx_hal.h"
#include "Soundengine/DSPAudio/1PoleLp.h"
#include "math.h"
//-------------------------------------------------------------
void initOnePole(OnePoleFilter* filter)
{
	filter->b = 0.f;
	filter->r = 0.01f;
}
//-------------------------------------------------------------
IN_ITCM_RAM float calcOnePole(OnePoleFilter* filter, double input)
{
	//result = a × b + c
	filter->b = fmaf(filter->r, input - filter->b, filter->b);
	//filter->b += filter->r * (input - filter->b);
	return filter->b;
}
//-------------------------------------------------------------
float calcOnePoleAdaptiveVolume(OnePoleFilter* filter, double input)
{

	const double diff = fabs(input-filter->b);
	if(diff >0.5 || (input < 0.01))
	{
		filter->b += 0.5f * (input - filter->b);
	}
	else if (diff > 0.05f)
	{
		filter->b += 0.01f * (input - filter->b);
	}
	else  filter->b += 0.001f * (input - filter->b);

	return filter->b;
}
//-------------------------------------------------------------
void calcOnePoleBlock(OnePoleFilter* filter, int16_t* buf, const uint8_t size)
{
	uint8_t i;
	for(i=0;i<size;i++)
	{
		filter->b += filter->r * (buf[i] - filter->b);
		buf[i] = filter->b;
	}
}
//-------------------------------------------------------------
void calcOnePoleBlockFixedInput(OnePoleFilter* filter, float input,float* output, const uint8_t size)
{
	uint8_t i;
	for(i=0;i<size;i++)
	{
		filter->b += filter->r * (input - filter->b);
		output[i] = filter->b;
	}
}
//-------------------------------------------------------------
void setOnePoleCoef(OnePoleFilter* filter,double val)
{
	filter->r = val;
}
//-------------------------------------------------------------
void setOnePoleValue(OnePoleFilter* filter,double val)
{
	filter->b = val;
}
//-------------------------------------------------------------
void setOnePoleFreq(OnePoleFilter* filter,double f)
{
	filter->r = CALC_FILTER_FREQ(f);
}
