/*
 * distortion.h
 *
 *  Created on: 11.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef DISTORTION_H_
#define DISTORTION_H_
//--------------------------------------------------
#include "stm32f7xx_hal.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/1PoleLp.h"
//--------------------------------------------------
typedef struct DistStruct
{
	float shape;
	float inv_shape;

#if SMOOTH_MIXER_VOL_PAN_DRIVE
	OnePoleFilter DriveParamFilter;
#endif
}Distortion;
//--------------------------------------------------
void setDistortionShape(Distortion *dist, float shape);
//--------------------------------------------------
void calcDistBlock(const Distortion *dist, int16_t* buf, const uint8_t size);
//--------------------------------------------------
void calcDistBlockFiltered(Distortion *dist, int16_t* buf, const uint8_t size);
//--------------------------------------------------
float distortion_calcSampleFloat(const Distortion *dist, float x);
//--------------------------------------------------
#endif /* DISTORTION_H_ */
