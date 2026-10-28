/*
 * Distortion.h
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

#ifndef SRC_SOUNDENGINE_DSPAUDIO_FX_DISTORTION_H_
#define SRC_SOUNDENGINE_DSPAUDIO_FX_DISTORTION_H_

#include "Soundengine/globals.h"
#include "Soundengine/config.h"


enum DistortionTypesEnum
{
	DISTORTION_TUBE,
	DISTORTION_FOLD,
	DISTORTION_RAT,
};

typedef struct DistortionStruct
{

	uint8_t distortionType;
	float drive;	//drive
	float p1;		//parameter 1
	float p2;		//parameter 2
	float p3;		//parameter 3

	//some filter states used in rat
	double hp1_state[2];
	double hp2_state[2];
	double hp3_state[2];
	double lp1_state[2];
	double lp2_state[2];
	double lp3_state[2];
	double slewState[2];

	double lastOut[2];


} MasterDistortion;



void Distortion_init();
void Distortion_process(MasterDistortion *dist, int16_t *fxBuffer, int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet);
void Distortion_setDrive(MasterDistortion *dist, float d);
void Distortion_setParam1(MasterDistortion *dist, float p);
void Distortion_setParam2(MasterDistortion *dist, float p);
void Distortion_setParam3(MasterDistortion *dist, float p);

#endif /* SRC_SOUNDENGINE_DSPAUDIO_FX_DISTORTION_H_ */
