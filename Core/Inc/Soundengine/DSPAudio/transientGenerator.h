/*
 * transientGenerator.h
 *
 *  Created on: 29.06.2012
 *
 *      the transient generator generates a kind of 'click' oneshot sound when triggered
 *      a variety of different, very short [50 ms] transient samples is used
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef TRANSIENTGENERATOR_H_
#define TRANSIENTGENERATOR_H_


#include "stm32f7xx_hal.h"
#include <stdint.h>
#include "transientTables.h"

typedef struct TransientGeneratorStruct
{
	int16_t 	output;
	uint32_t	phase;		// the current phase of the osc
	float	 	pitch;		// we need no accurate tuning for a transient, so we use a pitch factor [0.25f:1.75f]
	uint8_t		waveform;	// the selected transient table of the osc
	float		volume;

	//for midi
	uint16_t	midiFreq;  //upper 8 bit coarse, lower 8 bit fine
	float cvFreq;
} TransientGenerator;


void transient_init(TransientGenerator* transient);

void transient_trigger(TransientGenerator* transient);

void transient_calc(TransientGenerator* transient);
void transient_calcBlock(TransientGenerator* transient, int16_t* buf, const uint8_t size);

void transient_setWaveform(TransientGenerator* transient, const uint8_t waveform);
#endif /* TRANSIENTGENERATOR_H_ */
