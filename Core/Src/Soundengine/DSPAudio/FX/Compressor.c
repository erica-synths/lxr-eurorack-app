/*
 * Compressor.c
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

#include "Soundengine/DSPAudio/FX/Compressor.h"
#include "math.h"
#include "main.h"


enum
{
	RATIO_1,
	RATIO_2,
	RATIO_3,
	RATIO_4,
	RATIO_6,
	RATIO_8
};

static const double DC_OFFSET = 1.0E-25;

void Compressor_init(Compressor *c)
{
	c->attack = 0.1;
	c->release = 300;

	//c->filter = 0.0;
	c->env = 0.0;
	c->tresh = 3;
	c->markupGain = 1.0f;

}

// linear -> dB conversion
static double lin2dB( double lin ) {
	static const double LOG_2_DB = 8.6858896380650365530225783783321;	// 20 / ln( 10 )
	return logf( lin ) * LOG_2_DB;
}

// dB -> linear conversion
static double dB2lin( double dB ) {
	static const double DB_2_LOG = 0.11512925464970228420089957273422;	// ln( 10 ) / 20
	return expf( dB * DB_2_LOG );
}

//att/release, time in ms to reach 63% of final value
void Compressor_setAttack(Compressor *c, float a)
{
	c->attack = 0.03f + a *100.f; //[ms]
	c->attack = exp( -1000.0 / ( c->attack * REAL_FS ) );
}
void Compressor_setTreshold(Compressor *c, float t)
{
	c->tresh = -96 + t*96; //[-96:0]dB
}

void Compressor_setGain(Compressor *c, float g)
{
	c->markupGain = 1.f + g*2;
}
void Compressor_setRelease(Compressor *c, float r)
{
	c->release = 25+r*175.f; //[ms]
	c->release = exp( -1000.0 / ( c->release * REAL_FS ) );
}

void Compressor_setRatio(Compressor *c, uint8_t r)
{
	switch(r)
	{
	default:
	case RATIO_1:
		c->ratio = 1.0f;
		break;

	case RATIO_2:
		c->ratio = 0.5f;
		break;
	case RATIO_3:
		c->ratio = 1.f/3.f;
		break;
	case RATIO_4:
		c->ratio = 0.25f;
		break;
	case RATIO_6:
		c->ratio = 1.f/6.f;
		break;
	case RATIO_8:
		c->ratio = 0.125f;
		break;
	}
}

void Compressor_process(Compressor *c, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{


	int32_t x;
	int32_t* stereoOut = (int32_t*)outL;

	int16_t left[size];
	int16_t right[size];

	if(channels==2)
	{

		for(uint8_t i=0; i<size;i++)
		{
			//--- Compressor ---
			float in = 0.5 * (fxBuffer[i*2]/32768.f) + 0.5 * (fxBuffer[i*2+1]/32768.f);
			double peak = fabs(in);

			//value over thresh
			peak += DC_OFFSET;
			double peakdB = lin2dB(peak);
			double overdB = peakdB - c->tresh;
			if ( overdB < 0.0 )
				overdB = 0.0;

			// dynamic selection: attack or release?
			double  coeff = overdB > c->env ? c->attack : c->release;
			c->env = overdB + coeff * (c->env - overdB);
			//c->env = c->env + (1-coeff) * (overdB -c->env); //should be the same as above, without 1-

			double gr = c->env * (c->ratio - 1.0);
			gr = dB2lin(gr);
			//debugWatch = gr;

			left[i]  = __SSAT(fxBuffer[i*2] * gr * c->markupGain, 16);//gain;
			right[i] = __SSAT(fxBuffer[i*2+1] * gr * c->markupGain, 16);//gain;


			//--- Output transfer ---
			//TODO can be optimized quite a bit with SIMD
			const uint8_t idx1 = i*2;
			const uint8_t idx2 = i*2+1;
			//int16_t wet = __SSAT((int32_t)(left[i] * 32768.0f), 16);
			//int16_t wet2 = __SSAT((int32_t)(right[i] * 32768.0f), 16);
			x = __PKHBT(fxBuffer[idx1] + __SSAT((left[i] - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((right[i] - fxBuffer[idx2]) * dryWet,16), 16);
			//int16_t dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);
			*stereoOut = __QADD16(*stereoOut, x);
			stereoOut++;
		}
	}
	else
	{
		for(uint8_t i=0; i<size;i++)
		{
			const uint8_t idx1 = i*2;
			//--- Compressor ---
			float in = fxBuffer[idx1]/32768.f;
			double peak = fabs(in);

			//value over thresh
			peak += DC_OFFSET;
			double peakdB = lin2dB(peak);
			double overdB = peakdB - c->tresh;
			if ( overdB < 0.0 )
				overdB = 0.0;

			// dynamic selection: attack or release?
			double  coeff = overdB > c->env ? c->attack : c->release;
			c->env = overdB + coeff * (c->env - overdB);
			//c->env = c->env + (1-coeff) * (overdB -c->env); //should be the same as above, without 1-

			double gr = c->env * (c->ratio - 1.0);
			gr = dB2lin(gr);
			//debugWatch = gr;

			left[i]  = fxBuffer[idx1] * gr * c->markupGain;//gain;


			//--- Output transfer ---
			const int16_t dw = fxBuffer[idx1] + __SSAT((left[i] - fxBuffer[idx1]) * dryWet,16);
			*outL = __QADD16(*outL, dw);
			outL += 2;
		}
	}
}

