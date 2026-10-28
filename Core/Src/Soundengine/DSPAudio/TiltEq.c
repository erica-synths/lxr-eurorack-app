/*
 * TiltEq.c
 *
 *  Created on: Sep 1, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "Soundengine/DSPAudio/TiltEq.h"


static inline float
fasterpow2 (float p)
{
  float clipp = (p < -126) ? -126.0f : p;
  union { uint32_t i; float f; } v = { (uint32_t) ( (1 << 23) * (clipp + 126.94269504f) ) };
  return v.f;
}

static inline float
fasterexp (float p)
{
  return fasterpow2 (1.442695040f * p);
}



void TiltEq_init(TiltEq* eq)
{
	TiltEq_setFreq(eq, 400);
}

float TiltEq_process(TiltEq* eq, float in)
{

	eq->lp_out = eq->a0*in + eq->b1*eq->lp_out;
	return eq->lgain*eq->lp_out + eq->hgain*(in - eq->lp_out);
	/*
	eq->lp_out = eq->a0*in + eq->b1*eq->lp_out;
	return in + eq->lgain*eq->lp_out + eq->hgain*(in - eq->lp_out);
	*/
}

void TiltEq_setFreq(TiltEq* eq, float fc)
{
	//filter
	const float omega = 2*M_PI*fc;
	const float n = 1/(SR3 + omega);
	eq->a0 = 2*omega*n;
	eq->b1 = (SR3 - omega)*n;
}

void TiltEq_setParameter(TiltEq* eq, float p)
{
	p = p*p*p; //TODO shaper
	if(p>=0)
	{
		//highpass
		eq->lgain = 0;
		eq->hgain = 1;

		TiltEq_setFreq(eq, (20000)*p);

	} else {
		//lowpass
		eq->lgain = 1;
		eq->hgain = 0;

		TiltEq_setFreq(eq, (20000)*(1+p));
	}
	/*
	float g1,g2;
	if (gain > 0) {
		g1 = -eq->gfactor*gain;
		g2 = gain;
	} else {
		g1 = -gain;
		g2 = eq->gfactor*gain;
	};

	//two separate gains

	eq->lgain = fasterexp(g1/eq->amp)-1;
	eq->hgain = fasterexp(g2/eq->amp)-1;

*/
}
