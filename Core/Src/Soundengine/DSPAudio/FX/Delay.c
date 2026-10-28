/*
 * Delay.c
 *
 *  Created on: 30.08.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "Soundengine/DSPAudio/FX/Delay.h"

void Delay_init(Delay* d)
{


	d->pendingDelayTime = 0.f;
	d->timePending = 0;
	d->oldRead = d->newRead = 0;
	d->fracReadNew = d->fracReadOld = 0;
	d->write = FX_BUFFER_SIZE/20-10;

	d->feedback = 0.7f;
	//lp.setFreq(1);
	//d->rateFilter.setFreq(CALC_FILTER_FREQ(5000));

	//lpShaper.setShape(0.9f);
	//hpShaper.setShape(-0.9f);
	d->interpStep = d->interpValue = 0;

	initOnePole(&d->tone);
	initOnePole(&d->tone2);
}

float interpolate_s16(float a, float b, float x)
{
	return a*(1-x) + b*x;
}

void Delay_calcBufferPingPong(Delay* d, uint8_t bufferIdx)
{
	const uint32_t halfsize = FX_BUFFER_SIZE>>1;

	if(bufferIdx==0)
	{
		//calc with read pointer 0
		uint32_t pos2 = d->oldRead+1;
		pos2 = pos2 & (halfsize);
		d->tmpBuffer[0] = interpolate_s16((float)FX_BUFFER[d->oldRead], (float)FX_BUFFER[pos2],d->fracReadOld)*32767.f ;
		d->tmpBuffer2[0] = interpolate_s16((float)FX_BUFFER[halfsize + d->oldRead], (float)FX_BUFFER[halfsize + pos2],d->fracReadOld)*32767.f ;
		d->oldRead++;
		d->oldRead = d->oldRead & (halfsize);
	} else
	{
		//calc with read pointetr 2
		uint32_t pos2 = d->newRead+1;
		pos2 = pos2 & (halfsize);
		d->tmpBuffer[1] = interpolate_s16((float)FX_BUFFER[d->newRead], (float)FX_BUFFER[pos2],d->fracReadNew)*32767.f;
		d->tmpBuffer2[1] = interpolate_s16((float)FX_BUFFER[halfsize + d->newRead], (float)FX_BUFFER[halfsize + pos2],d->fracReadNew)*32767.f;
		d->newRead++;
		d->newRead = d->newRead & (halfsize);
	}

}

void Delay_calcBuffer(Delay* d, uint8_t bufferIdx)
{
	if(bufferIdx==0)
	{
		//calc with read pointer 0
		uint32_t pos2 = d->oldRead+1;
		pos2 = pos2 & (FX_BUFFER_SIZE);
		d->tmpBuffer[0] = interpolate_s16((float)FX_BUFFER[d->oldRead], (float)FX_BUFFER[pos2],d->fracReadOld)*32767.f ;
		d->oldRead++;
		d->oldRead = d->oldRead & (FX_BUFFER_SIZE);
	} else
	{
		//calc with read pointetr 2
		uint32_t pos2 = d->newRead+1;
		pos2 = pos2 & (FX_BUFFER_SIZE);
		d->tmpBuffer[1] = interpolate_s16((float)FX_BUFFER[d->newRead], (float)FX_BUFFER[pos2],d->fracReadNew)*32767.f ;;
		d->newRead++;
		d->newRead = d->newRead & (FX_BUFFER_SIZE);
	}
}


void Delay_processPingPong(Delay *d, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{

	int32_t x;
	int32_t* stereoOut = (int32_t*)outL;
	int16_t* monoOut = outL;

	for(uint8_t i=0;i<size;i++)
	{

		const uint8_t idx1 = i*2;
		const uint8_t idx2 = i*2+1;


			int16_t outL, outR;


			if(d->interpValue <= 0.f) //no parameter change in progress -> no crossfade
			{
				Delay_calcBufferPingPong(d, 0);
				outL = d->tmpBuffer[0];
				outR = d->tmpBuffer2[0];
				//if(d->interpValue <= 0.f) //no parameter change in progress -> no crossfade
				{
					//lfo modulation can cause pending delay times
					if(	d->timePending )
					{
						d->timePending = 0;
						Delay_setDelayTime(d, d->pendingDelayTime);
					}
				}
				//out = fourPole.process(out,sizeParameter,variationParameter*5);
			} else
			{
				Delay_calcBufferPingPong(d, 0);
				Delay_calcBufferPingPong(d, 1);

				outL = d->tmpBuffer[0] * d->interpValue + d->tmpBuffer[1] * (1.0 - d->interpValue);
				outR = d->tmpBuffer2[0] * d->interpValue + d->tmpBuffer2[1] * (1.0 - d->interpValue);
				//out = fourPole.process(out,sizeParameter,variationParameter*200);
				d->interpValue -= d->interpStep; //decrement InterpFactor toward zero


				if (d->interpValue <= 0.0)
				{
					d->oldRead = d->newRead; //update the OldParms after crossfade finished
					d->fracReadOld = d->fracReadNew;
				}
			}

			//calc tone filter
			outL = calcOnePole(&d->tone, outL);
			outR = calcOnePole(&d->tone2, outR);

			//-------------
			//left delay line
			int16_t itg = (__QADD16(fxBuffer[idx1] , (int16_t)(outL*d->feedback)));
			float val = itg/32767.f;
			if(val > 1.f) val = 1.f;
			else if(val < -1.f) val = -1.f;
			FX_BUFFER[(FX_BUFFER_SIZE>>1) + d->write] = (__fp16)val;//(itg/32767.f);

			//right delay line
			itg = (__QADD16(fxBuffer[idx2] , (int16_t)(outR*d->feedback)));
			val = itg/32767.f;
			if(val > 1.f) val = 1.f;
			else if(val < -1.f) val = -1.f;
			FX_BUFFER[d->write++] = (__fp16)val;//(itg/32767.f);
			//*signalFeedback = out;

			d->write &= FX_BUFFER_SIZE>>1;


			//*fxBuffer = out;//dryWetMixer(out,*signalAudio,d->dryWet);
			int16_t wet =  outL;

			if(channels == 2)
			{
				int16_t wet2 =  outR;

				x = __PKHBT(__SSAT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16),16),   __SSAT(fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16),16), 16);
				//LERP dryWet
				*stereoOut = __QADD16(*stereoOut, x);
				stereoOut++;
			}
			else
			{
				int16_t tmp = __SSAT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16),16);
				*monoOut = __SSAT(*monoOut + tmp, 16);
				monoOut+=2;
			}
			//*input1 = dryWet*out + (1-dryWet)* (*input1);




	}
}

void Delay_process(Delay *d, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{
	//FX_BUFFER[0] = 0;

	int32_t x;
	int32_t* stereoOut = (int32_t*)outL;
	int16_t* monoOut = outL;
	for(uint8_t i=0;i<size;i++)
	{

		const uint8_t idx1 = i*2;
		const uint8_t idx2 = i*2+1;

	#if 0
		int16_t out;

		if(phase>=1)
		{
			phase-=1;
		}

		phase+=variationParameter*variationParameter*0.25;

		if(phase>=0.5) out=32767;
		else out=-32767;

		return out;

	#else
			int16_t out;


			if(d->interpValue <= 0.f) //no parameter change in progress -> no crossfade
			{
				Delay_calcBuffer(d, 0);
				out = d->tmpBuffer[0];
				//if(d->interpValue <= 0.f) //no parameter change in progress -> no crossfade
				{
					//lfo modulation can cause pending delay times
					if(	d->timePending )
					{
						d->timePending = 0;
						Delay_setDelayTime(d, d->pendingDelayTime);
					}
				}
				//out = fourPole.process(out,sizeParameter,variationParameter*5);
			} else
			{
				Delay_calcBuffer(d, 0);
				Delay_calcBuffer(d, 1);

				out = d->tmpBuffer[0] * d->interpValue + d->tmpBuffer[1] * (1.0 - d->interpValue);
				//out = fourPole.process(out,sizeParameter,variationParameter*200);
				d->interpValue -= d->interpStep; //decrement InterpFactor toward zero


				if (d->interpValue <= 0.0)
				{
					d->oldRead = d->newRead; //update the OldParms after crossfade finished
					d->fracReadOld = d->fracReadNew;
				}
			}

			//calc tone filter
			out = calcOnePole(&d->tone, out);

			//-------------

	#if 0
			float floatDataOversampled[2];
			float in = out/32767.f;
			float filtered[2];


			oversampler.upsample2(&in,floatDataOversampled,1);

			#if USE_SVF

				floatDataOversampled[0] = floatDataOversampled[0]*svf.getGain();
				floatDataOversampled[1] = floatDataOversampled[1]*svf.getGain();

				svf.process<SVF_LP_2>(floatDataOversampled[0]);
				svf.process<SVF_LP_2>(floatDataOversampled[1]);

				floatDataOversampled[0] = svf.clip(floatDataOversampled[0],1);
				floatDataOversampled[1] = svf.clip(floatDataOversampled[1],1);
			#else
			floatDataOversampled[0] = fourPole.process(floatDataOversampled[0],fc,variationParameter*4);
			floatDataOversampled[1] = fourPole.process(floatDataOversampled[1],fc,variationParameter*4);
			#endif

			oversampler.downsample2(floatDataOversampled,filtered,1);
			filtered[0] *= 32767;
			out = filtered[0];
			//---------



	#endif

			//we add both stereo channels together for a mono delay
			int16_t itg = (__QADD16(__QADD16(fxBuffer[idx1], fxBuffer[idx2]) , (int16_t)(out*d->feedback)));
			//int16_t itg = (__QADD16(fxBuffer[idx1] , (int16_t)(out*d->feedback)));
			float val = itg/32767.f;
			if(val > 1.f) val = 1.f;
			else if(val < -1.f) val = -1.f;
			FX_BUFFER[d->write++] = (__fp16)val;//(itg/32767.f);
			//*signalFeedback = out;

			d->write &= FX_BUFFER_SIZE;


			//*fxBuffer = out;//dryWetMixer(out,*signalAudio,d->dryWet);
			int16_t wet =  out;

			if(channels == 2)
			{
				int16_t wet2 =  out;

				x = __PKHBT(__SSAT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16),16),   __SSAT(fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16),16), 16);
				//LERP dryWet
				*stereoOut = __QADD16(*stereoOut, x);
				stereoOut++;
			}
			else
			{
				int16_t tmp = __SSAT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16),16);
				*monoOut = __SSAT(*monoOut + tmp, 16);
				monoOut+=2;
			}

			//*input1 = dryWet*out + (1-dryWet)* (*input1);
	#endif
	}
}

float interpolate(float a, float b, float x)
{
	return a*(1-x) + b*x;
}
float calcDelayTime(Delay* d, float timeCv)
{
	if(d->delayType == DELAY_NORMAL)
	{
		const float min = interpolate(LOW_MIN,HI_MIN,d->range);
		const float max = interpolate(LOW_MAX,HI_MAX,d->range);
		return interpolate(min,max,timeCv);
	}
	else
	{
		const float min = interpolate(LOW_MIN,HI_MIN,d->range);
		const float max = interpolate(LOW_MAX,HI_MAX*0.48f,d->range);
		return interpolate(min,max,timeCv);
	}

}

void Delay_setDelayTime(Delay* d, float t)
{




#if 1
	if(d->interpValue <=0)
	{
		d->delayTime = t;//rateFilter.process(t);


		//uint32_t offset = medianFilter(delayTime*SRAM_SIZE*range);
		float offset =  /*medianFilter*/(calcDelayTime(d, d->delayTime * d->range));
		//timeBuffer.write(offset);
		//offset = timeBuffer.getAverage();

		uint32_t offItg = (uint32_t)offset;
		d->fracReadNew = offset-offItg;


		//MainApplication::setLedTempo(offset/(float)(SAMPLE_RATE)*500.f);

		//newRead = medianFilter((uint32_t)(write-1-delayTime*SRAM_SIZE*range));
		d->newRead = (d->write-1-  offItg);

		//read = (uint32_t)(write-t*44100);
		if(d->delayType == DELAY_NORMAL)
		{
			d->newRead = d->newRead & (FX_BUFFER_SIZE);
		}
		else
		{
			d->newRead = d->newRead & (FX_BUFFER_SIZE>>1);
		}

		d->interpValue = 1.f;
		const float fadeSeconds = 0.01f;//100 ms
		const float fadeInSamps = REAL_FS * fadeSeconds;
		d->interpStep = 1.0f / fadeInSamps;
	}
	else
	{
		d->pendingDelayTime = t;
		d->timePending = 1;
	}
#else
	d->delayTime = t;

	d->oldRead = (uint32_t)(d->write - 1 -  (d->delayTime*d->bufferSize));
	d->oldRead = d->oldRead & (d->bufferSize);

#endif

}

void Delay_setFeedback(Delay* d, float fb)
{
	d->feedback = fb;
}
void Delay_setRange(Delay* d, float r)
{
	d->range = r;
	Delay_setDelayTime(d, d->delayTime);

}
void Delay_setFilter(Delay* d, float f)
{
	setOnePoleCoef(&d->tone, f*f);
	setOnePoleCoef(&d->tone2, f*f);
}

