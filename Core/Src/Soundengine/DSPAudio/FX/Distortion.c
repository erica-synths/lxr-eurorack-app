/*
 * Distortion.c
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

#include "Soundengine/DSPAudio/FX/Distortion.h"
#include <math.h>

INDTCM const float table_1n914[257] = {
		-0.647479,
		-0.647111, -0.646743, -0.646375, -0.646007, -0.645639,
		-0.645271, -0.644903, -0.644535, -0.644167, -0.643799,
		-0.643431, -0.642998, -0.642508, -0.642019, -0.641529,
		-0.641039, -0.640550, -0.640060, -0.639571, -0.639081,
		-0.638591, -0.638102, -0.637604, -0.636993, -0.636382,
		-0.635771, -0.635160, -0.634549, -0.633937, -0.633326,
		-0.632715, -0.632104, -0.631493, -0.630882, -0.630198,
		-0.629465, -0.628733, -0.628000, -0.627267, -0.626535,
		-0.625802, -0.625069, -0.624337, -0.623604, -0.622871,
		-0.622123, -0.621268, -0.620414, -0.619560, -0.618706,
		-0.617852, -0.616997, -0.616143, -0.615289, -0.614435,
		-0.613581, -0.612727, -0.611791, -0.610816, -0.609840,
		-0.608864, -0.607889, -0.606913, -0.605937, -0.604961,
		-0.603986, -0.603010, -0.602034, -0.601035, -0.599937,
		-0.598840, -0.597743, -0.596646, -0.595548, -0.594451,
		-0.593354, -0.592256, -0.591159, -0.590062, -0.588965,
		-0.587735, -0.586456, -0.585177, -0.583740, -0.582106,
		-0.580472, -0.578782, -0.576793, -0.574804, -0.572815,
		-0.570515, -0.568172, -0.565828, -0.563275, -0.560576,
		-0.557877, -0.555070, -0.552016, -0.548962, -0.545901,
		-0.542492, -0.539083, -0.535674, -0.531758, -0.527172,
		-0.521927, -0.516021, -0.509456, -0.502230, -0.494344,
		-0.485269, -0.474132, -0.460956, -0.445352, -0.426263,
		-0.403305, -0.376436, -0.346270, -0.313817, -0.280021,
		-0.245520, -0.210672, -0.175659, -0.140568, -0.105442,
		-0.070300, -0.035152, -0.000001, 0.035152, 0.070301,
		0.105442, 0.140567, 0.175658, 0.210671, 0.245520,
		0.280021, 0.313817, 0.346270, 0.376436, 0.403304,
		0.426263, 0.445263, 0.460964, 0.474058, 0.484899,
		0.494282, 0.502657, 0.510037, 0.516238, 0.521415,
		0.526164, 0.530741, 0.535030, 0.539196, 0.543024,
		0.546779, 0.550148, 0.553492, 0.556400, 0.559308,
		0.561781, 0.564229, 0.566291, 0.568278, 0.570024,
		0.571681, 0.573337, 0.574994, 0.576651, 0.578308,
		0.579964, 0.581585, 0.583058, 0.584532, 0.586005,
		0.587478, 0.588951, 0.590424, 0.591898, 0.593250,
		0.594539, 0.595829, 0.597119, 0.598408, 0.599698,
		0.600988, 0.602254, 0.603360, 0.604467, 0.605573,
		0.606679, 0.607785, 0.608891, 0.609997, 0.610995,
		0.611918, 0.612840, 0.613763, 0.614685, 0.615608,
		0.616530, 0.617443, 0.618182, 0.618921, 0.619660,
		0.620399, 0.621138, 0.621877, 0.622616, 0.623260,
		0.623816, 0.624371, 0.624927, 0.625482, 0.626038,
		0.626593, 0.627149, 0.627683, 0.628217, 0.628752,
		0.629286, 0.629820, 0.630354, 0.630888, 0.631422,
		0.631957, 0.632491, 0.633025, 0.633559, 0.634093,
		0.634627, 0.635161, 0.635696, 0.636230, 0.636764,
		0.637298, 0.637832, 0.638366, 0.638901, 0.639435,
		0.639969, 0.640503, 0.641037, 0.641571, 0.642106,
		0.642640, 0.643174, 0.643708, 0.644242, 0.644776,
		0.645288, 0.645727, 0.646165, 0.646604, 0.647042,
		0.647042
		};


INDTCM const double ecc83Table[129] = {
-0.993818651334, -0.993773150866, -0.993727650399, -0.993682149931,
-0.993636649463, -0.993591148995, -0.993545648528, -0.99350014806,
-0.993454647592, -0.993409147125, -0.993363646657, -0.993256563682,
-0.99276458144, -0.991803719025, -0.990332655629, -0.988312889096,
-0.98570873291, -0.982487310607, -0.978618547622, -0.974075160572,
-0.968832643965, -0.962869254345, -0.956165991871, -0.948706579323,
-0.940477438545, -0.931467664319, -0.921668995669, -0.911075784602,
-0.899684962276, -0.887496002605, -0.874510883293, -0.860734044301,
-0.84617234375, -0.830835011248, -0.814733598658, -0.797881928296,
-0.780296038554, -0.76199412697, -0.742996490712, -0.723325464514,
-0.703005356026, -0.682062378609, -0.660524581556, -0.63842177775,
-0.615785468748, -0.592648767303, -0.569046317314, -0.545014211215,
-0.520589904785, -0.495812129405, -0.470720801731, -0.445356930816,
-0.419762522648, -0.393980482135, -0.368054512512, -0.342029012183,
-0.315948969002, -0.289859851972, -0.263807500392, -0.237838010426,
-0.211997619107, -0.186332585777, -0.160889070952, -0.135713012624,
-0.11085, -0.0863451446598, -0.0622429491607, -0.0385871730651,
-0.0154206964046, 0.00721461942549, 0.029278073338, 0.0507302687944,
0.0715332606125, 0.091650704342, 0.111048008208, 0.12969248762,
0.147553522255, 0.164602715698, 0.180814057661, 0.196164088761,
0.210632067871, 0.224200142038, 0.236853518967, 0.24858064207,
0.259373368093, 0.269227147298, 0.278141206221, 0.286118732995,
0.293167065239, 0.299297880521, 0.304527389379, 0.30887653092,
0.312371170976, 0.315042302838, 0.316926250553, 0.318064874785,
0.3188461248, 0.3196273748, 0.3204086248, 0.3211898748,
0.3219711248, 0.3227523748, 0.3235336248, 0.3243148748,
0.3250961248, 0.3258773748, 0.3266586248, 0.3274398748,
0.3282211248, 0.3290023748, 0.3297836248, 0.3305648748,
0.3313461248, 0.3321273748, 0.3329086248, 0.3336898748,
0.3344711248, 0.3352523748, 0.3360336248, 0.3368148748,
0.3375961248, 0.3383773748, 0.3391586248, 0.3399398748,
0.3407211248, 0.3415023748, 0.3422836248, 0.3430648748,
0.3438461248
 };

float clamp(float d, float min, float max) {
  const float t = d < min ? min : d;
  return t > max ? max : t;
}


static float fnms(float a, float b, float c)
{
	return c - a*b;
}

static float lerp(float a, float b, float t) {
  //return a + t * (b - a);
	//fma(a, b, c) = a*b + c
	//fnms(a, b, c) = -(a*b - c) = c - a*b
	return fma(t, b, fnms(t, a, a));

}

float diode(float in)
{

	//slew state is limited to +/- 4.2

	//if(in>1.f) in=1.f;
	//else if (in<-1.f) in=-1.f;
	in = clamp(in, -1.f, 1.f);
	float index = (in+1.f) * 128.; // [0:256]
	//float index = (in+4.2f) * 30.47f; // [0:256]

	uint16_t itg = (int16_t) index;
	float frac = index - itg;
	return lerp(table_1n914[itg], table_1n914[itg+1], frac);

}

void Distortion_init(MasterDistortion *dist)
{
	dist->distortionType = DISTORTION_TUBE;
	dist->drive = 1.0f;
	dist->p1 = 1.0f;
	dist->p2 = 0.0f;
	dist->p3 = 0.0f;

	dist->lastOut[0] = 0;
	dist->lastOut[1] = 0;

	dist->hp1_state[0] = 0.0;
	dist->hp1_state[1] = 0.0;

	dist->hp2_state[0] = 0.0;
	dist->hp2_state[1] = 0.0;

	dist->hp3_state[0] = 0.0;
	dist->hp3_state[1] = 0.0;

	dist->lp1_state[0] = 0.0;
	dist->lp1_state[1] = 0.0;

	dist->lp2_state[0] = 0.0;
	dist->lp2_state[1] = 0.0;

	dist->slewState[0] = 0.0;
	dist->slewState[1] = 0.0;

	dist->lp3_state[0] = 0.0;
	dist->lp3_state[1] = 0.0;
}

IN_ITCM_RAM void Distortion_setDrive(MasterDistortion *dist,float d)
{
	dist->drive = d;
}
IN_ITCM_RAM void Distortion_setParam1(MasterDistortion *dist,float p)
{
	dist->p1 = p;
}

IN_ITCM_RAM void Distortion_setParam2(MasterDistortion *dist,float p)
{
	dist->p2 = p;
}

IN_ITCM_RAM void Distortion_setParam3(MasterDistortion *dist,float p)
{
	dist->p3 = p;
}

float foldback(float in, float threshold)
{
  if (in>threshold || in<-threshold)
  {
    in= fabs(fabs(fmod(in - threshold, threshold*4)) - threshold*2) - threshold;
  }
  return in;
}

void Distortion_process(MasterDistortion *dist, int16_t *fxBuffer, int16_t* outL, int16_t* outR, uint16_t size, uint8_t channels, float dryWet)
{
	if(dist->distortionType == DISTORTION_TUBE)
	{
		//--- Tone Control ---
		const double coff = 0.06537948995454451662 + dist->p2 * 0.92410701734426630737;
		//--------------------

		const float outputModifier = (1.0+0.55*dist->p1) +0.55*dist->p1;

		//outL and outR are in the same array as interleaved stereo samples.
		//therefore the *out += 2 to increment it
		//we can use an int32_t instead of int_16_t to use SIMD instructions on both stereo channels
		int32_t* stereoOut = (int32_t*)outL;

		if(channels==2)
		{
			for(uint32_t i=0;i<size;i++)
			{
				const uint8_t idx1 = i*2;
				const uint8_t idx2 = i*2+1;

				int32_t input1 = ((int32_t)((fxBuffer[idx1]) * (1.f + dist->drive*4)));
				int32_t input2 = ((int32_t)((fxBuffer[idx2]) * (1.f + dist->drive*4)));

				uint8_t itg1 = (__SSAT(input1,16)>>9)+64;
				uint8_t itg2 = (__SSAT(input2,16)>>9)+64;

				float frac1 = (input1&0b0000000111111111) / 512.f;
				float frac2 = (input2&0b0000000111111111) / 512.f;

				double val1 = ecc83Table[itg1] + (ecc83Table[itg1+1]-ecc83Table[itg1]) * frac1;
				double val2 = ecc83Table[itg2] + (ecc83Table[itg2+1]-ecc83Table[itg2]) * frac2;



				//--- tone control ---
				dist->lp3_state[0] += coff * (val1 - dist->lp3_state[0]);
				dist->lp3_state[1] += coff * (val2 - dist->lp3_state[1]);
				//---

				val1 = dist->lp3_state[0] * outputModifier * dist->p3;
				val2 = dist->lp3_state[1] * outputModifier * dist->p3;



				const int16_t wet = __SSAT((int32_t)(val1 * 32768.0f), 16);
				//simd.word[0] = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);

				const int16_t wet2 = __SSAT((int32_t)(val2 * 32768.0f), 16);
				//simd.word[1] = fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16);

				const int32_t x = __PKHBT( fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16), 16);

				//add stereo samples using SIMD
				*stereoOut = __QADD16(*stereoOut, x);
				stereoOut++;
			}
		}
		else
		{
			for(uint32_t i=0;i<size;i++)
			{
				const uint8_t idx1 = i*2;
				int32_t input1 = ((int32_t)((fxBuffer[idx1]) * (1.f + dist->drive)));
				uint8_t itg1 = (__SSAT(input1,16)>>9)+64;
				float frac1 = (input1&0b0000000111111111) / 512.f;
				double val1 = ecc83Table[itg1] + (ecc83Table[itg1+1]-ecc83Table[itg1]) * frac1;


				//--- tone control ---
				dist->lp3_state[0] += coff * (val1 - dist->lp3_state[0]);
				val1 = dist->lp3_state[0] * outputModifier * dist->p3;
				//val1 = dist->lp3_state[0] * dist->p3;
				//---

				int16_t wet = __SSAT((int32_t)(val1 * 32768.0f), 16);
				int16_t dw = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);
				*outL = __QADD16(*outL, dw);
				//*outL =  __SSAT((int32_t)(val1 * 32768.0f), 16);
				outL += 2;
			}
		}
	}
	else if(dist->distortionType == DISTORTION_FOLD)
	{
		//p1 => offset
		//x*(2-abs(x))

		//--- Tone Control ---
		const double coff = 0.06537948995454451662 + dist->p2 * 0.92410701734426630737;
		//--------------------

		if(channels==2)
		{
			int32_t* stereoOut = (int32_t*)outL;
			//simdBuffer simd;

			for(uint32_t i=0;i<size;i++)
			{
				const uint8_t idx1 = i*2;
				const uint8_t idx2 = i*2+1;

				float input1 = ( (fxBuffer[idx1])  ) / 32768.f;
				float input2 = ( (fxBuffer[idx2]) ) / 32768.f ;

				float val1 = input1  * (1.f + dist->drive*4.f)  + dist->p1;
				float val2 = input2  * (1.f + dist->drive*4.f)  + dist->p1;

	#if 1

				//--- tone control ---
				dist->lp3_state[0] += coff * (val1 - dist->lp3_state[0]);
				dist->lp3_state[1] += coff * (val2 - dist->lp3_state[1]);
				//---


				val1 = foldback(dist->lp3_state[0], 1.f) * dist->p3;
				val2 = foldback(dist->lp3_state[1], 1.f) * dist->p3;
	#else

				val1 = val1*(2-fabsf(val1));
				val2 = val2*(2-fabsf(val2));
				val1 = val1*(2-fabsf(val1));
				val2 = val2*(2-fabsf(val2));

	#endif


				/*
				int16_t wet = __SSAT((int32_t)(val1 * 32768.0f), 16);
				int16_t dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);
				*outL = __QADD16(*outL, dw);
				outL += 2;

				wet = __SSAT((int32_t)(val2 * 32768.0f), 16);
				dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);
				*outR = __QADD16(*outR, dw);
				outR += 2;
				*/
				int32_t x;
				const int16_t wet = __SSAT((int32_t)(val1 * 32768.0f), 16);
				//simd.word[0] = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);

				const int16_t wet2 = __SSAT((int32_t)(val2 * 32768.0f), 16);
				//simd.word[1] = fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16);

				x = __PKHBT( fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16), 16);

				//add stereo samples using SIMD
				*stereoOut = __QADD16(*stereoOut, x);
				stereoOut++;

			}
		}
		else
		{
			for(uint32_t i=0;i<size;i++)
			{
				const uint8_t idx1 = i*2;

				float input1 = ( (fxBuffer[idx1])  ) / 32768.f;
				float val1 = input1  * (1.f + dist->drive*4.f)  + dist->p1;


				//--- tone control ---
				dist->lp3_state[0] += coff * (val1 - dist->lp3_state[0]);
				//---

				val1 = foldback(dist->lp3_state[0], 1.f) * dist->p3;

				int16_t wet = __SSAT((int32_t)(val1 * 32768.0f), 16);
				int16_t dw = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);

				*outL = __QADD16(*outL, dw);
				outL += 2;

			}
		}


	}
	else //DISTORTION_RAT
	{
		int32_t* stereoOut = (int32_t*)outL;
		int32_t x;
		//simdBuffer simd;
#if 0
		double tone =  475 + dist->p1*(32000-475);
		const double coff = 1. - expf(-2*M_PI*tone/44140);
#else
		//Tone control 475 to 32000 Hz
		const double coff = 0.06537948995454451662 + dist->p2 * 0.92410701734426630737;
#endif
		const double gain = 1 + dist->drive*2304;

		if(channels==2)
		{
			for(uint8_t i=0;i<size;i++)
			{

				const uint8_t idx1 = i*2;
				const uint8_t idx2 = i*2+1;
				//power supply 0-9V with 4.5V center => +/- 4.5V

				double signal1 = ( (fxBuffer[idx1])  ) / 32768.;
				double signal2 = ( (fxBuffer[idx2])  ) / 32768.;  //[-1:1]

				//feedback thru hp
				//dist->hp1_state[0] += 0.64116641920131193922 * ( dist->p2 * 0.125 - dist->hp1_state[0]);
				//dist->hp1_state[1] += 0.64116641920131193922 * ( dist->p2 * 0.125 - dist->hp1_state[1]);

				signal1 += dist->lastOut[0] * dist->p1 * 0.125;
				signal2 += dist->lastOut[1] * dist->p1 * 0.125;

				//guitar levels are normally in mv range!
				//seems like generally we think of a 100-300mv signal in designing distortions etc, but a big E chord can get near 1V!
				//signal1 *= 0.3;
				//signal2 *= 0.3;

				//--- Clipper Amplifier
				//input HP 7.2Hz

				/*
				Coefficient calculation:
				x = exp(-2.0*pi*freq/samplerate);
				a0 = 1.0-x;
				b1 = -x;

				for 7.2Hz
				x = 0,35883358079868806078
				a0 = 0,64116641920131193922
				b1 = -0,35883358079868806078
				 */

				//const double a0 = 0.64116641920131193922;
				//const double b1 = -0.35883358079868806078;
				dist->hp1_state[0] += 0.64116641920131193922 * (signal1 - dist->hp1_state[0]);
				dist->hp1_state[1] += 0.64116641920131193922 * (signal2 - dist->hp1_state[1]);

				//we now have lowpass, substract from input for HP
				signal1 = signal1 - dist->hp1_state[0];
				signal2 = signal2 - dist->hp1_state[1];




				//LP/HP around opAmp
				//1 pole LP   1/(2*pi*R_dist*0.0000001)  min = 16kHz = 0.1025353241628911807
				dist->lp1_state[0] += (1. - 0.1025353241628911807) * (signal1 - dist->lp1_state[0]);
				dist->lp1_state[1] += (1. - 0.1025353241628911807) * (signal2 - dist->lp1_state[1]);
				signal1 = dist->lp1_state[0];
				signal2 = dist->lp1_state[1];

				// 1 HP poles 1539Hz -> 0.80326417028080632238
				/*
				dist->hp2_state[0] += (1. - 0.80326417028080632238) * (signal1 - dist->hp2_state[0]);
				dist->hp2_state[1] += (1. - 0.80326417028080632238) * (signal2 - dist->hp2_state[1]);
				signal1 = dist->lp1_state[0] - dist->hp2_state[0];
				signal2 = dist->lp1_state[1] - dist->hp2_state[1];
				*/
				// 1 HP poles 60Hz -> 0.99149556438154282898
				dist->hp3_state[0] += (1. - 0.99149556438154282898) * (signal1 - dist->hp3_state[0]);
				dist->hp3_state[1] += (1. - 0.99149556438154282898) * (signal2 - dist->hp3_state[1]);
				signal1 = signal1 - dist->hp3_state[0];
				signal2 = signal2 - dist->hp3_state[1];

				//gain bw
				// 67dB up to 500Hz
				// 50dB @ 1k
				// ~33 @ 10k
				// ~15 @ 100k
				// -> lp @ 500Hz -> 0.93130041642945205043
				dist->lp2_state[0] += (1. - 0.93130041642945205043) * (signal1 - dist->lp2_state[0]);
				dist->lp2_state[1] += (1. -0.93130041642945205043) * (signal2 - dist->lp2_state[1]);
				signal1 = dist->lp2_state[0];
				signal2 = dist->lp2_state[1];


				//--- opamp section ---

				//voltage gain [1:2305]
				signal1 *= gain;
				signal2 *= gain;



				//slew limit 0.3V/us 0-9V  =>  ~5.3khz
				//SR = 44140Hz = 1 sample duration of 22,655... us
				// slew limit = 22.655*0.3V = 6.79655641141821477118 V/sample
				double diff = signal1 - dist->slewState[0];
				double sign = diff > 0 ? 1. : -1.;
				if(fabs(diff) > 6.79655641141821477118) diff = 6.79655641141821477118 * sign;
				dist->slewState[0] += diff;

				diff = signal2 - dist->slewState[1];
				sign = diff > 0 ? 1 : -1;
				if(fabs(diff) > 6.79655641141821477118) diff = 6.79655641141821477118 * sign;
				dist->slewState[1] += diff;

				//we hard clip here to the opamp rails, even if soft clipping would be more appropriate
				if(dist->slewState[0] > 4.2) dist->slewState[0]=4.2;
				else if(dist->slewState[0] < -4.2) dist->slewState[0] = -4.2;

				if(dist->slewState[1] > 4.2) dist->slewState[1]=4.2;
				else if(dist->slewState[1] < -4.2) dist->slewState[1] = -4.2;


				//diode clipping +/- 0.7V 1n914 use table_1n914
				signal1 = diode(dist->slewState[0]);
				signal2 = diode(dist->slewState[1]);

				//max range now [-0.647479:0.647042]
				//we ommit thios cause rat is too loud!
				//signal1 *= 1.54;
				//signal2 *= 1.54;

				//range ~ [-1:1]

				//--- Tone Control ---
				//simple lowpass
				//fMin = 475Hz
				//fMax = 32kHz

				dist->lp3_state[0] += coff * (signal1 - dist->lp3_state[0]);
				dist->lp3_state[1] += coff * (signal2 - dist->lp3_state[1]);

				dist->lastOut[0] = dist->lp3_state[0] * dist->p3;
				dist->lastOut[1] = dist->lp3_state[1] * dist->p3;


				int16_t wet = __SSAT((int32_t)(dist->lastOut[0] * 32768.0f), 16);
				//simd.word[0] = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);
				int16_t wet2 = __SSAT((int32_t)(dist->lastOut[1] * 32768.0f), 16);
				//simd.word[1] = fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16);

				x = __PKHBT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((wet2 - fxBuffer[idx2]) * dryWet,16), 16);
				//add stereo samples using SIMD
				*stereoOut = __QADD16(*stereoOut, x);
				stereoOut++;
			}
		}
		else
		{
			for(uint32_t i=0;i<size;i++)
			{

				const uint8_t idx1 = i*2;
				double signal1 = ( (fxBuffer[idx1])  ) / 32768.f;

				signal1 += dist->lastOut[0] * dist->p1 * 0.125;

				dist->hp1_state[0] += 0.64116641920131193922 * (signal1 - dist->hp1_state[0]);
				signal1 = signal1 - dist->hp1_state[0];

				//LP/HP around opAmp
				//1 pole LP   1/(2*pi*R_dist*0.0000001)  min = 16kHz = 0.1025353241628911807
				dist->lp1_state[0] += (1. - 0.1025353241628911807) * (signal1 - dist->lp1_state[0]);
				signal1 = dist->lp1_state[0];

				// 1 HP poles 60Hz -> 0.99149556438154282898
				dist->hp3_state[0] += (1. - 0.99149556438154282898) * (signal1 - dist->hp3_state[0]);
				signal1 = signal1 - dist->hp3_state[0];

				dist->lp2_state[0] += (1. - 0.93130041642945205043) * (signal1 - dist->lp2_state[0]);
				signal1 = dist->lp2_state[0];

				//voltage gain [1:2305]
				signal1 *= gain;

				//slew limit 0.3V/us 0-9V  =>  ~5.3khz
				//SR = 44140Hz = 1 sample duration of 22,655... us
				// slew limit = 22.655*0.3V = 6.79655641141821477118 V/sample
				double diff = signal1 - dist->slewState[0];
				double sign = diff > 0 ? 1. : -1.;
				if(fabs(diff) > 6.79655641141821477118) diff = 6.79655641141821477118 * sign;
				dist->slewState[0] += diff;

				//we hard clip here to the opamp rails, even if soft clipping would be more appropriate
				if(dist->slewState[0] > 4.2) dist->slewState[0]=4.2;
				else if(dist->slewState[0] < -4.2) dist->slewState[0] = -4.2;

				//diode clipping +/- 0.7V 1n914 use table_1n914
				signal1 = diode(dist->slewState[0]);

				dist->lp3_state[0] += coff * (signal1 - dist->lp3_state[0]);

				dist->lastOut[0] = dist->lp3_state[0] * dist->p3;


				int16_t wet = __SSAT((int32_t)(dist->lastOut[0] * 32768.0f), 16);
				int16_t dw = fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16);
				*outL = __QADD16(*outL, dw);
				outL += 2;
			}
		}
	}
}

