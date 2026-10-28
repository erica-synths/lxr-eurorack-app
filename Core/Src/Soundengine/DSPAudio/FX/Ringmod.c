/*
 * Ringmod.c
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


#include "Soundengine/DSPAudio/FX/Ringmod.h"

void Ringmod_init(Ringmod *rm)
{
	rm->freq = 0.5f;

	rm->osc.cvFreq = -1.f;
	rm->osc.freq = 100;
	rm->osc.modNodeValue = 1;
	rm->osc.waveform = REC; //for testing init to tri,saw,rec
	rm->osc.fmMod = 0;
	rm->osc.midiFreq = 70<<8 | 64;
	rm->osc.pitchMod = 1.0f;
	osc_setFreq(&rm->osc);
}
void Ringmod_process(Ringmod *rm, int16_t *fxBuffer, int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{
	int16_t buf[size];
	int32_t x;
	int32_t* stereoOut = (int32_t*)outL;

	calcNextOscSampleBlock(&rm->osc, buf, size, 1.f, 0.3f);

	if(channels==2)
	{
		for(uint8_t i=0;i<size;i++)
		{
			const uint8_t idx1 = i*2;
			const uint8_t idx2 = i*2+1;

			//TODO could probably be simd optimized
			int32_t val1 = ((int32_t)fxBuffer[idx1]) * buf[i];
			int32_t val2 = ((int32_t)fxBuffer[idx2]) * buf[i];
			//int32_t val2 = *outR * buf[i];


			int16_t wet =  val1>>16;
			int16_t wet2 =  val2>>16;
			//int16_t dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);

			x = __PKHBT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16), 16);
			//LERP dryWet
			*stereoOut = __QADD16(*stereoOut, x);
			stereoOut++;
		}
	}
	else
	{
		for(uint8_t i=0;i<size;i++)
		{
			const uint8_t idx1 = i*2;
			int32_t val1 = ((int32_t)fxBuffer[idx1]) * buf[i];

			//*outL = val1>>16;
			//*outR = val2>>16;

			const int16_t wet = __QADD16(*outL, val1>>16);
			const int16_t dw = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);
			*outL = __QADD16(*outL, dw);
			outL += 2;
		}
	}
}
void Ringmod_setFreq(Ringmod *rm, float f)
{
	rm->osc.freq = f*880.f;
	osc_setFreq(&rm->osc);
}
void Ringmod_setWave(Ringmod *rm, uint8_t w)
{
	rm->osc.waveform = w;
}

void Ringmod_resetPhase(Ringmod* rm)
{
	if(rm->osc.waveform == SINE)
		rm->osc.phase = 1024 + ( (0x3ff<<20) - 1024);//voiceArray[voiceNr].osc.startPhase ;
	else //if(rm->osc.waveform > SINE && rm->osc.waveform <= REC)
		rm->osc.phase = (0xff<<22);

}

