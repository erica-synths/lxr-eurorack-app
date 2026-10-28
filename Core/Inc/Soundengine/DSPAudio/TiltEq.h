/*
 * TiltEq.h
 *
 *  Created on: 01.08.2014
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef TILTEQ_H_
#define TILTEQ_H_

#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "math.h"

#define SR3 (3*REAL_FS)
//#define cast_uint32_t static_cast<uint32_t>

typedef struct TiltEqStruct
{
	float amp;
	//float denorm;
	//float sr3;
	float gfactor; //gfactor is the proportional gain
	float a0,b1;
	float lp_out;

	float lgain,hgain;
} TiltEq;


void TiltEq_init(TiltEq* eq);
float TiltEq_process(TiltEq* eq, float in);
void TiltEq_setFreq(TiltEq* eq, float fc);
void TiltEq_setParameter(TiltEq* eq, float p);




//=======================
// tilt eq settings
//
// srate -> sample rate
// fc -> 20-20khz
// gain -> -6 / +6 db
//=======================

#if 0
class TiltEq
{
public:
	TiltEq()
	{
		amp = 6/logf(2);
		//denorm = 10^-30;
		//sr3 = 3*SAMPLE_RATE;
		gfactor = 5;
		setFreq(400);
	};
	~TiltEq(){};

	float process(float in)
	{
		lp_out = a0*in + b1*lp_out;
		return in + lgain*lp_out + hgain*(in - lp_out);
	}

	void setFreq(float fc)
	{
		//filter
		const float omega = 2*M_PI*fc;
		const float n = 1/(SR3 + omega);
		a0 = 2*omega*n;
		b1 = (SR3 - omega)*n;
	}

	void setGain(float gain)
	{
		float g1,g2;
		if (gain > 0) {
		    g1 = -gfactor*gain;
		    g2 = gain;
		} else {
		    g1 = -gain;
		    g2 = gfactor*gain;
		};

		//two separate gains
		/*
		lgain = expf(g1/amp)-1;
		hgain = expf(g2/amp)-1;
		*/
		lgain = fasterexp(g1/amp)-1;
		hgain = fasterexp(g2/amp)-1;


	}

private:
	float a0,b1;
	float lp_out;

	float lgain,hgain;

};

//#############################################
// keeps gain always <= 1
// prefered for feedback loops

class TiltFilter
{
public:
	TiltFilter()
	{
		//amp = 6/logf(2);
		//denorm = 10^-30;
		setFreq(400);

	};
	~TiltFilter(){};

	float process(float in)
	{
		lp_out = a0*in + b1*lp_out;
		return lgain*lp_out + hgain*(in - lp_out);
	}

	//-1:0 = lowpass
	//0:1 = highpass
	void setParameter(float p)
	{
		p = p*p*p*p; //TODO shaper
		if(p>=0)
		{
			//highpass
			lgain = 0;
			hgain = 1;

			setFreq((20000)*p);

		} else {
			//lowpass
			lgain = 1;
			hgain = 0;

			setFreq((20000)*(1+p));
		}
	}

private:
	void setFreq(float fc)
	{
		//filter
		const float omega = 2*M_PI*fc;
		const float n = 1/(SR3 + omega);
		a0 = 2*omega*n;
		b1 = (SR3 - omega)*n;
	}

private:
	//float amp;
	//float denorm;
	//float sr3;
	//float gfactor; //gfactor is the proportional gain
	float a0,b1;
	float lp_out;

	float lgain,hgain;

};


#endif

#endif /* TILTEQ_H_ */
