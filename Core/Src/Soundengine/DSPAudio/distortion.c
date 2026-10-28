/*
 * distortion.c
 *
 *  Created on: 14.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/DSPAudio/distortion.h"
#include "Soundengine/config.h"
#include "math.h"
//--------------------------------------------------
IN_ITCM_RAM __inline void setDistortionShape(Distortion *dist, float shape)
{
	shape -= 0.0078125; //to prevent division by 0
	dist->shape = 2*(shape)/(1-(shape));
}
//--------------------------------------------------
IN_ITCM_RAM void calcDistBlock(const Distortion *dist, int16_t* buf, const uint8_t size)
{
	uint8_t i;
	for(i=0;i<size;i++)
	{
			float x = buf[i]/32767.f;
			//const float shape1 = 1+dist->shape;
			x = (1+dist->shape)*x/(1+dist->shape*fabsf(x));
			buf[i] = (x*32767);
	}
}
//--------------------------------------------------
void calcDistBlockFiltered(Distortion *dist, int16_t* buf, const uint8_t size)
{
	uint8_t i;
	for(i=0;i<size;i++)
	{
#if SMOOTH_MIXER_VOL_PAN_DRIVE
			float shape	= calcOnePole( &dist->DriveParamFilter, dist->shape);
#else
			float shape = dist->shape;
#endif
			float x = buf[i]/32767.f;
			//const float shape1 = 1+dist->shape;
			x = (1+shape)*x/(1+shape*fabsf(x));
			buf[i] = (x*32767);
	}
}
//--------------------------------------------------

float distortion_calcSampleFloat(const Distortion *dist, float x)
{
	return (1+dist->shape)*x/(1+dist->shape*fabsf(x));
}
