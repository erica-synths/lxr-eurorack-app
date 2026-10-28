/*
 * ResonantFilter.c
 *
 *  Created on: 05.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/DSPAudio/ResonantFilter.h"


// to make the linker happy. __errno seems not defined
// so libm.a won't link without this int.
//int __errno;
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_setReso(ResonantFilter* filter, float feedback)
{
	filter->q = 1-feedback;
	if(filter->q<0.1f)filter->q = 0.02f;
}
//------------------------------------------------------------------------------------
void SVF_init(ResonantFilter* filter)
{
		filter->s1 = 0;
		filter->s2 = 0;

		filter->a =filter->b = 0;

		filter->f = 0.20f;
		filter->q = 0.9f;

		filter->drive = 0.315f;

		SVF_directSetFilterValue(filter,0.25f);

#if USE_SHAPER_NONLINEARITY
		setDistortionShape(&filter->shaper, 0);
#endif
}
//------------------------------------------------------------------------------------
void SVF_reset(ResonantFilter* filter)
{
	filter->s1 = 0;
	filter->s2 = 0;

#if ENABLE_NONLINEAR_INTEGRATORS
	filter->zi = 0;	//input z^(-1)
#endif

	filter->a = filter->b = 0;
}
//------------------------------------------------------------------------------------
IN_ITCM_RAM static float fastTanh(float var)
{
   if(var < -1.95f)     return -1.0f;
   else if(var > 1.95f) return  1.0f;
   else					return  4.15f*var/(4.29f+var*var);
}
//------------------------------------------------------------------------------------
IN_ITCM_RAM float fastTan(float x)
{
#if 1
	const float x2 = x*x;
	const float A = -15*x+x*x2;
	const float B = 3*(-5+2*x2);
	return A/B;
#else
	const float A = 5*(-21*x+2*x*x*x);
	const float B = 105-45*x*x+x*x*x*x;
	return A/B;
#endif
}
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_recalcFreq(ResonantFilter* filter)
{
	filter->g  = fastTan(M_PI * filter->f );
#if USE_SHAPER_NONLINEARITY
	setDistortionShape(&filter->shaper, filter->drive);
#endif
}
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_setDrive(ResonantFilter* filter,float drive)
{
#if USE_SHAPER_NONLINEARITY
	filter->drive = drive;
	setDistortionShape(&filter->shaper, filter->drive);
#else
	filter->drive =  0.4f + (drive)*(drive)*6;
#endif

}
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_directSetFilterValue(ResonantFilter* filter, float val)
{
	filter->f = val*(0.5f*0.90f);
	filter->g  = fastTan(M_PI * filter->f );

}
//------------------------------------------------------------------------------------
//#if ENABLE_NONLINEAR_INTEGRATORS
IN_ITCM_RAM float tanhXdX(float x)
{
	const float a = x*x;
    // IIRC I got this as Pade-approx for tanh(sqrt(x))/sqrt(x)
	return ((a + 105)*a + 945) / ((15*a + 420)*a + 945);
}
//#endif
//------------------------------------------------------------------------------------
IN_ITCM_RAM float softClipTwo(float in)
{

	return in * tanhXdX(0.5*in);

//	if(in > 0.76159415595576488811945828260479f) return fastTanh(in-0.76159415595576488811945828260479f)+0.76159415595576488811945828260479f;
//	if(in < -0.76159415595576488811945828260479f) return fastTanh(in+0.76159415595576488811945828260479f)-0.76159415595576488811945828260479f;

//	return in;
}
#pragma GCC push_options
#pragma GCC optimize("O0")
#define is_nan(x) ((x) != (x))

float removeMaliciousSample(float in)
{
	if(is_nan(in))
	{
		return 0.f;
	}

	return in;

	/*
	uint32_t* floatInput = (uint32_t*)in;// *inArrayOfFloats;
    uint32_t sample = *floatInput;// *inArrayOfFloats;
    uint32_t exponent = sample & 0x7F800000;

            // exponent < 0x7F800000 is 0 if NaN or Infinity, otherwise 1
            // exponent > 0 is 0 if denormalized, otherwise 1

    int aNaN = exponent < 0x7F800000;
    int aDen = exponent > 0;


    *floatInput = sample * ( aNaN & aDen );
    *floatInput */
}
#pragma GCC pop_options
#define USE_FMAF 0
//------------------------------------------------------------------------------------
IN_ITCM_RAM void SVF_calcBlockZDF(ResonantFilter* filter, const uint8_t type, int16_t* buf, const uint8_t size)
{
	uint8_t i;
	const float f 	= filter->g;
	//fix unstable filter for high f and r settings
	const float R 	= filter->f >= 0.4499f ? 1 : filter->q;
	const float ff 	= f*f;

	if(type== FILTER_OFF) return;
	if(type == FILTER_NAIVE_2_POLE)
	{

		float f_lp2 = filter->f * 2.21f;
		for(i=0;i<size;i++)
		{
			/* alternative 2Pole LP filter to fix the kick transient problems with the nonlinear ZDF LP */

			float x = softClipTwo((buf[i]/((float)0x7fff))*filter->drive);

#if !USE_FMAF
			float q = (1-filter->q) *1.4 + (1-filter->q) / (1.0 - f_lp2);
#else
			//result = a × b + c
			float q = fmaf((1-filter->q), 1.4f, (1-filter->q) / (1.0 - f_lp2));

#endif


#if !USE_FMAF
			filter->a += f_lp2 * (   (x - filter->a)  + q * (filter->a - filter->b )   );
#else
			//result = a × b + c
			const float tmpC = fmaf(q, filter->a - filter->b, (x - filter->a));
			filter->a = fmaf(f_lp2, tmpC, filter->a);
#endif

			if(filter->a > 1) filter->a = 1;
			else if(filter->a < -1) filter->a = -1;

			//result = a × b + c
#if !USE_FMAF
			filter->b  += f_lp2 * (filter->a - filter->b );
#else
			filter->b = fmaf(f_lp2, filter->a - filter->b, filter->b);
#endif


			if(filter->b > 1) filter->b = 1;
			else if(filter->b < -1) filter->b = -1;

			int32_t tmp;
			tmp = (filter->b  * FILTER_GAIN);
			buf[i] = __SSAT(tmp,16);
		}

	} else {

		filter->s1 = removeMaliciousSample(filter->s1);
		filter->s2 = removeMaliciousSample(filter->s2);
		for(i=0;i<size;i++)
		{




	#if USE_SHAPER_NONLINEARITY
			const float x = (buf[i]/((float)0x7fff));
	#else
			const float x = softClipTwo((buf[i]/((float)0x7fff))*filter->drive);
	#endif

	#if ENABLE_NONLINEAR_INTEGRATORS
			// input with half sample delay, for non-linearities
			float ih = 0.5f * (x + filter->zi);
			filter->zi = x;
	#endif

			// evaluate the non-linear gains
			/*
			You can travially remove any saturator by setting the corresponding gain t0,...,t1 to 1. Also, you can simply scale any saturator (i.e. change clipping threshold) to 1/a*tanh(a*x) by writing
			double t1 = tanhXdX(a*s[0]);
			*/
	#if ENABLE_NONLINEAR_INTEGRATORS
			const float scale = 0.5f;
			const float t0 = tanhXdX(scale* (ih - 2*R*filter->s1 - filter->s2 ) );
			const float t1 = tanhXdX(scale* (filter->s1 ) );
	#else
			const float t0 = 1;
			const float t1 = 1;
	#endif

			// g# the denominators for solutions of individual stages

#if !USE_FMAF
			const float g0 = 1.f / (1.f + f*t0*2*R);
#else
			//result = a × b + c
			const float g0 = 1.f / fmaf(f*t0 , 2.f*R, 1.f);
#endif
			const float s1 = filter->s1;
			const float s2 = filter->s2;

			// solve feedback
			const float f1 = ff*g0*t0*t1;

#if !USE_FMAF
			float y1=(f1*x+s2+f*g0*t1*s1)/(f1+1);
#else
			const float tmpA = fmaf(f1, x, s2);
			const float tmpB = fmaf(f*g0, t1*s1, tmpA);
			const float y1 =  tmpB / (f1+1);
#endif


			// solve the remaining stages with nonlinear gain
			 const float xx = t0*(x - y1);
			 const float y0 = (softClipTwo(s1) + f*xx)*g0;

#if !USE_FMAF
			filter->s1   = softClipTwo(filter->s1) + 2*f*(xx - t0*2*R*y0);
#else
			 filter->s1 = fmaf(2*f , (xx - t0*2*R*y0), softClipTwo(filter->s1));
#endif

#if !USE_FMAF
			filter->s2   = (filter->s2)    + 2*f* t1*y0;
#else
			//result = a × b + c
			filter->s2 = fmaf(2*f , t1*y0, filter->s2);
#endif

			int32_t tmp;
			switch(type)
			{
			default:
				return;
				break;
			case FILTER_LP:
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * fastTanh( distortion_calcSampleFloat(&filter->shaper, y1));
	#else
				tmp = fastTanh(y1) * 0x7fff ;//FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
				break;

			case FILTER_HP:
			{
				const float ugb = 2*R*y0;
				const float h = x - ugb - y1;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, h);

	#else
				tmp = h * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_BP:
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, y0);
	#else
				tmp = y0 * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
				break;

			case FILTER_UNITY_BP:
			{
				const float ugb = 2*R*y0;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, ugb);
	#else
				tmp = ugb * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_NOTCH:
			{
				const float ugb = 2*R*y0;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, (x-ugb));
	#else
				tmp = (x-ugb) * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_PEAK:
			{
				const float ugb = 2*R*y0;
				const float h = x - ugb - y1;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, (y1-h));
	#else
				tmp = (y1-h) * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
			break;

			}

			buf[i] = __SSAT(buf[i],16);
		}
	}
}


//----------------------------------------------------------------------
IN_ITCM_RAM void SVF_calcBlockCheapZDF(ResonantFilter* filter, const uint8_t type, int16_t* buf, const uint8_t size)
{
	uint8_t i;
	const float f 	= filter->g;
	//fix unstable filter for high f and r settings
	const float R 	= filter->f >= 0.4499f ? 1 : filter->q;
	const float ff 	= f*f;

	if(type== FILTER_OFF) return;
	if(type == FILTER_NAIVE_2_POLE)
	{

		float f_lp2 = filter->f * 2.21f;
		for(i=0;i<size;i++)
		{
			/* alternative 2Pole LP filter to fix the kick transient problems with the nonlinear ZDF LP */

			float x = softClipTwo((buf[i]/((float)0x7fff))*filter->drive);

#if !USE_FMAF
			float q = (1-filter->q) *1.4 + (1-filter->q) / (1.0 - f_lp2);
#else
			//result = a × b + c

			float q = fmaf((1-filter->q), 1.4f, (1-filter->q) / (1.0 - f_lp2));
#endif


#if !USE_FMAF
			filter->a += f_lp2 * (   (x - filter->a)  + q * (filter->a - filter->b )   );
#else
			//result = a × b + c
			const float tmpC = fmaf(q, filter->a - filter->b, (x - filter->a));
			filter->a = fmaf(f_lp2, tmpC, filter->a);
#endif

			if(filter->a > 1) filter->a = 1;
			else if(filter->a < -1) filter->a = -1;

			//result = a × b + c
#if !USE_FMAF
			filter->b  += f_lp2 * (filter->a - filter->b );
#else
			filter->b = fmaf(f_lp2, filter->a - filter->b, filter->b);
#endif


			if(filter->b > 1) filter->b = 1;
			else if(filter->b < -1) filter->b = -1;

			int32_t tmp;
			tmp = (filter->b  * FILTER_GAIN);
			buf[i] = __SSAT(tmp,16);
		}

	} else {

		for(i=0;i<size;i++)
		{
	#if USE_SHAPER_NONLINEARITY
			const float x = (buf[i]/((float)0x7fff));
	#else
			const float x = softClipTwo((buf[i]/((float)0x7fff))*filter->drive);
	#endif

	#if 0
			// input with half sample delay, for non-linearities
			float ih = 0.5f * (x + filter->zi);
			filter->zi = x;
	#endif

			// evaluate the non-linear gains
			/*
			You can travially remove any saturator by setting the corresponding gain t0,...,t1 to 1. Also, you can simply scale any saturator (i.e. change clipping threshold) to 1/a*tanh(a*x) by writing
			double t1 = tanhXdX(a*s[0]);
			*/
	#if 0
			const float scale = 0.5f;
			const float t0 = tanhXdX(scale* (ih - 2*R*filter->s1 - filter->s2 ) );
			const float t1 = tanhXdX(scale* (filter->s1 ) );
	#else
			const float t0 = 1;
			const float t1 = 1;
	#endif

			// g# the denominators for solutions of individual stages
#if !USE_FMAF
			const float g0 = 1.f / (1.f + f*t0*2*R);
#else

			//result = a × b + c
			const float g0 = 1.f / fmaf(f*t0 , 2.f*R, 1.f);
#endif
			const float s1 = filter->s1;
			const float s2 = filter->s2;

			// solve feedback
			const float f1 = ff*g0*t0*t1;
			//float y1=(f1*x+s2+f*g0*t1*s1)/(f1+1);
			const float tmpA = fmaf(f1, x, s2);
			const float tmpB = fmaf(f*g0, t1*s1, tmpA);
			const float y1 =  tmpB / (f1+1);


			// solve the remaining stages with nonlinear gain
			 const float xx = t0*(x - y1);
			 const float y0 = (softClipTwo(s1) + f*xx)*g0;

#if !USE_FMAF
			filter->s1   = softClipTwo(filter->s1) + 2*f*(xx - t0*2*R*y0);
#else
			 filter->s1 = fmaf(2*f , (xx - t0*2*R*y0), softClipTwo(filter->s1));
#endif

#if !USE_FMAF
			filter->s2   = (filter->s2)    + 2*f* t1*y0;
#else
			//result = a × b + c
			filter->s2 = fmaf(2*f , t1*y0, filter->s2);
#endif

			int32_t tmp;
			switch(type)
			{
			default:
				return;
				break;
			case FILTER_LP:
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * fastTanh( distortion_calcSampleFloat(&filter->shaper, y1));
	#else
				tmp = fastTanh(y1) * 0x7fff ;//FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
				break;

			case FILTER_HP:
			{
				const float ugb = 2*R*y0;
				const float h = x - ugb - y1;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, h);
	#else
				tmp = h * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_BP:
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, y0);
	#else
				tmp = y0 * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
				break;

			case FILTER_UNITY_BP:
			{
				const float ugb = 2*R*y0;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, ugb);
	#else
				tmp = ugb * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_NOTCH:
			{
				const float ugb = 2*R*y0;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, (x-ugb));
	#else
				tmp = (x-ugb) * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

			case FILTER_PEAK:
			{
				const float ugb = 2*R*y0;
				const float h = x - ugb - y1;
	#if USE_SHAPER_NONLINEARITY

				buf[i] = FILTER_GAIN * distortion_calcSampleFloat(&filter->shaper, (y1-h));
	#else
				tmp = (y1-h) * FILTER_GAIN;
				buf[i] = __SSAT(tmp,16);
	#endif
			}
				break;

				}
			buf[i] = __SSAT(buf[i],16);
		}
	}
}
