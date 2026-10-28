/*
 * Reverb.c
 *
 *  Created on: 31.08.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "Soundengine/DSPAudio/FX/Reverb.h"
#include "Soundengine/DSPAudio/FX/FxRamBuffer.h"



static const float earlyTaps[] =
{
		12,
		24,
		38,
		50
};

void Reverb_init(Reverb *r)
{

	TiltEq_init(&r->eq);
	TiltEq_init(&r->eqFb);

	SineCosine_init(&r->lfo);
	r->reverbBufferSram = (__fp16*)FX_BUFFER;

	r->earlyVolume = 1.0f;


	setOnePoleCoef(&r->lpFeedback, 0.5f);
	setOnePoleCoef(&r->lpImpulse, 0.5f);

		//feedbackShaper.setShape(0.7f);
	r->lastOut = r->lastOut2 = 0;
	SineCosine_setFreq(&r->lfo, 1.31f);
	/*
	AllpassDelay apDelay[2];
		AllpassModDelay apModDelay[2];
		*/
#if SPIN_SEMI
		for(int i=0;i<4;i++)
		{
			AllpassDelay_init(&r->apDelay[i][0]);
			AllpassDelay_init(&r->apDelay[i][1]);
			AllpassDelay_setBuffer(&r->apDelay[i][0], &r->reverbBufferSram[i*2*(MAX_ALLPASS_SIZE)]);
			AllpassDelay_setBuffer(&r->apDelay[i][1], &r->reverbBufferSram[(i*2+1)*(MAX_ALLPASS_SIZE)]);
			//apDelay[i][0].setBuffer(&reverbBufferSram[i*2*(MAX_ALLPASS_SIZE)]);
			//apDelay[i][1].setBuffer(&reverbBufferSram[(i*2+1)*(MAX_ALLPASS_SIZE)]);
		}

		// 8 * MAX_ALLPASS_SIZE occupied

		for(int i=0;i<4;i++)
		{
			AllpassDelay_init(&r->inputAllpass[i]);
			//TODO could have way less memory -> max delay is 379 samples
			//inputAllpass[i].setBuffer(&reverbBufferSram[8*MAX_ALLPASS_SIZE + i*(MAX_ALLPASS_SIZE)]);
			AllpassDelay_setBuffer(&r->inputAllpass[i], &r->reverbBufferSram[8*MAX_ALLPASS_SIZE + i*(MAX_ALLPASS_SIZE)]);

		}
#else
		for(int i=0;i<2;i++)
		{
			AllpassDelay_init(&r->apDelay[i]);
			AllpassModDelay_init(&r->apModDelay[i]);

			AllpassDelay_setBuffer(&r->apDelay[i], &r->reverbBufferSram[i*2*(MAX_ALLPASS_SIZE)]);
			AllpassModDelay_setBuffer(&r->apModDelay[i], &r->reverbBufferSram[(i*2+1)*(MAX_ALLPASS_SIZE)]);
		}

		// 4 * MAX_ALLPASS_SIZE occupied

		for(int i=0;i<4;i++)
		{
			AllpassDelay_init(&r->inputAllpass[i]);
			//TODO could have way less memory -> max delay is 379 samples
			//inputAllpass[i].setBuffer(&reverbBufferSram[8*MAX_ALLPASS_SIZE + i*(MAX_ALLPASS_SIZE)]);
			AllpassDelay_setBuffer(&r->inputAllpass[i], &r->reverbBufferSram[4*MAX_ALLPASS_SIZE + i*(MAX_ALLPASS_SIZE)]);

		}
#endif


#if SPIN_SEMI
		// 12 * MAX_ALLPASS_SIZE occupied
		for(int i=0;i<2;i++)
		{
			VerbDelay_init(&r->delay[i]);
			VerbDelay_setBuffer(&r->delay[i], &r->reverbBufferSram[12 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
			//delay[i].setBuffer(&reverbBufferSram[12 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
		}
		for(int i=0;i<2;i++)
		{
			VerbModDelay_init(&r->modDelay[i]);
			//modDelay[i].setBuffer(&reverbBufferSram[14 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
			VerbModDelay_setBuffer(&r->modDelay[i], &r->reverbBufferSram[14 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
		}

		VerbDelay_setBuffer(&r->earlyReflectionDelay, &r->reverbBufferSram[16 * MAX_ALLPASS_SIZE]);
#else
		// 8 * MAX_ALLPASS_SIZE occupied
		for(int i=0;i<4;i++)
		{
			VerbDelay_init(&r->delay[i]);
			VerbDelay_setBuffer(&r->delay[i], &r->reverbBufferSram[8 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
			//delay[i].setBuffer(&reverbBufferSram[12 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
		}



		VerbDelay_init(&r->earlyReflectionDelay);
		VerbDelay_setBuffer(&r->earlyReflectionDelay, &r->reverbBufferSram[12 * MAX_ALLPASS_SIZE]);

		VerbDelay_init(&r->preDelay);
		VerbDelay_setBuffer(&r->preDelay, &r->reverbBufferSram[13 * MAX_ALLPASS_SIZE]);
#endif



		VerbDelay_setLength(&r->earlyReflectionDelay , 2000);
		//44 samples = 1ms, 2030 = 46ms
		VerbDelay_setLength(&r->preDelay, 2031);


		AllpassDelay_setDelay(&r->inputAllpass[0], 142);
		AllpassDelay_setDelay(&r->inputAllpass[1], 107);
		AllpassDelay_setDelay(&r->inputAllpass[2], 379);
		AllpassDelay_setDelay(&r->inputAllpass[3], 277);

		VerbDelay_setLength(&r->delay[0], 2047);
		VerbDelay_setLength(&r->delay[1], 1913); //todo wieso habe ich nur halbe buffer size? größer 4096 geht nicht
	//	delay[1].setLength(3090);

#if SPIN_SEMI
		VerbModDelay_setLength(&r->modDelay[0], 2047);
		VerbModDelay_setLength(&r->modDelay[1], 1999);

		AllpassDelay_setDelay(&r->apDelay[0][0], 672);
		AllpassDelay_setDelay(&r->apDelay[0][1], 757);

		AllpassDelay_setDelay(&r->apDelay[1][0], 1800);
		AllpassDelay_setDelay(&r->apDelay[1][1], 743);

		AllpassDelay_setDelay(&r->apDelay[2][0], 908);
		AllpassDelay_setDelay(&r->apDelay[2][1], 523);

		AllpassDelay_setDelay(&r->apDelay[3][0], 2039);
		AllpassDelay_setDelay(&r->apDelay[3][1], 877);

		VerbModDelay_setModDepth(&r->modDelay[0], 170);
		VerbModDelay_setModDepth(&r->modDelay[1], 170);

/*
		for(int i=0;i<4;i++)
		{
				AllpassDelay_setCoefficient(&r->apDelay[i][0], INITIAL_ALLPASS_COFF);
				AllpassDelay_setCoefficient(&r->apDelay[i][1], INITIAL_ALLPASS_COFF);
		}

		*/

		AllpassDelay_setCoefficient(&r->apDelay[i][0], 0.7);
		AllpassDelay_setCoefficient(&r->apDelay[i][1], 0.7);

		AllpassDelay_setCoefficient(&r->apDelay[i][0], 0.7);
		AllpassDelay_setCoefficient(&r->apDelay[i][1], 0.7);

		AllpassDelay_setCoefficient(&r->apDelay[i][0], 0.5);
		AllpassDelay_setCoefficient(&r->apDelay[i][1], 0.5);

		AllpassDelay_setCoefficient(&r->apDelay[i][0], 0.5);
		AllpassDelay_setCoefficient(&r->apDelay[i][1], 0.5);
	    //AllpassDelay_setCoefficient(&r->apDelay[2][1], -INITIAL_ALLPASS_COFF);
#else
		VerbDelay_setLength(&r->delay[2], 1720);
		VerbDelay_setLength(&r->delay[3], 1163); //todo wieso habe ich nur halbe buffer size? größer 4096 geht nicht

		AllpassDelay_setDelay(&r->apDelay[0], 1800);
		AllpassDelay_setDelay(&r->apDelay[1], 2039);

		AllpassModDelay_setDelay(&r->apModDelay[0], 672);
		AllpassModDelay_setDelay(&r->apModDelay[1], 908);
#endif

	    AllpassDelay_setCoefficient(&r->inputAllpass[0], 0.75);
	    AllpassDelay_setCoefficient(&r->inputAllpass[1], 0.75);
	    AllpassDelay_setCoefficient(&r->inputAllpass[2], 0.625);
	    AllpassDelay_setCoefficient(&r->inputAllpass[3], 0.625);

}




//void Reverb_process(Reverb *r, int16_t* input1, int16_t* input2)
void Reverb_process(Reverb *r, int16_t *fxBuffer,int16_t* outL, int16_t* outR, uint16_t size,  uint8_t channels, float dryWet)
{
	int32_t x;
	int32_t* stereoOut = (int32_t*)outL;

#if !HIGH_RES_SINE_COS
	 SineCosine_calc(&r->lfo);
#if SPIN_SEMI
	 AllpassModDelay_setModValue(&r->modDelay[1], SineCosine_getSine(&r->lfo));
	 AllpassModDelay_setModValue(&r->modDelay[0], SineCosine_getCosine(&r->lfo));
#else
	 VerbModDelay_setModValue(&r->apModDelay[1].delay, SineCosine_getSine(&r->lfo));
	 VerbModDelay_setModValue(&r->apModDelay[0].delay, SineCosine_getCosine(&r->lfo));
#endif
#endif
	for(uint8_t i=0; i<size;i++)
	{
		const uint8_t idx1 = i*2;
		const uint8_t idx2 = i*2+1;

		const int16_t mix = (__QADD16(fxBuffer[idx1]/2, fxBuffer[idx2]/2));
		float monoInput = mix/32768.f;

		monoInput = VerbDelay_process(&r->preDelay, monoInput);

		float tmp;
		float in, in2, out, out2;
#if USE_OUT_2
		float out2;
#endif

#if HIGH_RES_SINE_COS
		 SineCosine_calc(&r->lfo);
		 VerbModDelay_setModValue(&r->modDelay[1], SineCosine_getSine(&r->lfo));
		 VerbModDelay_setModValue(&r->modDelay[0], SineCosine_getCosine(&r->lfo));
#endif

		 //in = 0.5 * calcOnePole(&r->lpImpulse, AllpassDelay_process(&r->inputAllpass[0],  AllpassDelay_process(&r->inputAllpass[1], AllpassDelay_process(&r->inputAllpass[2], AllpassDelay_process(&r->inputAllpass[3], (monoInput))))));

		 	 //############## early reverb ####################

		 float early = 0;
		 //float vol = 1;//0.25f;
		 VerbDelay_process(&r->earlyReflectionDelay, monoInput);
		 /*
		 for(int j=0;j<EARLY_TAPS;j++)
		 {
		   early += VerbDelay_ReadTap(&r->earlyReflectionDelay, earlyTaps[j]) * vol ;
		   vol *= 0.5f;
		 }
		 */
		 early += VerbDelay_ReadTap(&r->earlyReflectionDelay, earlyTaps[0]) ;
		 early += VerbDelay_ReadTap(&r->earlyReflectionDelay, earlyTaps[1]) * 0.5f ;
		 early += VerbDelay_ReadTap(&r->earlyReflectionDelay, earlyTaps[2]) * 0.125f ;
		 early += VerbDelay_ReadTap(&r->earlyReflectionDelay, earlyTaps[3]) * 0.25f ;

		 monoInput = 0.5 * TiltEq_process(&r->eq, monoInput);//calcOnePole(&r->lpImpulse, monoInput);
		 in  = AllpassDelay_process(&r->inputAllpass[0], AllpassDelay_process(&r->inputAllpass[1], (monoInput)));
		 in2 = AllpassDelay_process(&r->inputAllpass[2], AllpassDelay_process(&r->inputAllpass[3], (monoInput)));



		 //in = 0.5*lpImpulse.process(inputAllpass[0].process(inputAllpass[1].process(inputAllpass[2].process(inputAllpass[3].process((*input1)<<16)))));







		 //############## Late reverb ####################

#if SPIN_SEMI

		    tmp = AllpassDelay_process(&r->apDelay[0][0], (in + r->lastOut));
#if !USE_LOWER_DENSITY
		    tmp = AllpassDelay_process(&r->apDelay[0][1], tmp);
#endif
		    tmp = VerbDelay_process(&r->delay[0], tmp);

		    out = tmp;


		    //out = __QSUB16((int16_t)(VerbDelay_ReadTap(&r->delay[0], 75)*0.83f),out);
		    //out = __QADD16((int16_t)(VerbDelay_ReadTap(&r->delay[0], 33)*0.2f),out);
		    out = (VerbDelay_ReadTap(&r->delay[0], 75)*0.83f) - out;
		    out = ((VerbDelay_ReadTap(&r->delay[0], 33)*0.2f) + out);
#if USE_OUT_2
		    out2 = tmp * 0.1;
#endif


		    tmp = AllpassDelay_process(&r->apDelay[1][0], tmp*r->feedback);
#if !USE_LOWER_DENSITY
		    tmp = AllpassDelay_process(&r->apDelay[1][1], tmp);
#endif
		    tmp = VerbModDelay_process(&r->modDelay[0], tmp);

		     //out = __QADD16((int16_t)(tmp*0.2f),out);
		    out = ((tmp*0.67) + out);



#if USE_OUT_2
		     out2 = ((tmp*0.2) + out2);
#endif


		    tmp = AllpassDelay_process(&r->apDelay[2][0], (in + tmp*r->feedback));
#if !USE_LOWER_DENSITY
		    tmp = AllpassDelay_process(&r->apDelay[2][1], tmp);
#endif
		    tmp = VerbDelay_process(&r->delay[1], tmp);
		    out = ((tmp*0.7f)+out);;

#if USE_OUT_2
		    out2 = (tmp * 0.5) + out2;
		    out2 = (VerbDelay_ReadTap(&r->delay[1], 33)*0.7) +out2;
#endif



		    tmp = AllpassDelay_process(&r->apDelay[3][0], tmp*r->feedback);
#if !USE_LOWER_DENSITY
		    tmp = AllpassDelay_process(&r->apDelay[3][1], tmp);
#endif
		    tmp = VerbModDelay_process(&r->modDelay[1], tmp);

		    r->lastOut = calcOnePole(&r->lpFeedback, tmp*r->feedback);

		    out = ((tmp*0.1f) + out);

#if USE_OUT_2
		    out2 = ((tmp) + out2);;
		    out2 = ((VerbModDelay_interpolatedReadTap(&r->modDelay[1],17)*0.2) + out2);
#endif


		   // output[0][i] = interpolate(input[0][i],output[0][i],dryWet) + early * earlyMix;
		   // output[1][i] = interpolate(input[1][i],output[1][i],dryWet) + early * earlyMix;
		  // return  dryWet*(__QADD16(out*0.5f,out2*0.5f) ) + (1-dryWet)*input;;
		    //return  (((revItg)(dryWet*(add(out , out2>>1) )))>>16) + (1-dryWet)*input;;
#if USE_OUT_2
		    out = (out + out2/2);
#endif

#if 0
		  //  r->lastOut = calcOnePole(&r->lpFeedback, tmp*r->feedback);
#else
		  //  *input1 =  dryWetMixer(out,*input1,dryWet);


		   // return  (((revItg)(dryWet*(add(out , out2) )))>>16) + (1-dryWet)*input;;

	//	   return  dryWet*out + (1-dryWet)*input;
#endif
#else




		    //loop 1
		    // in1 + loop 2 out ->apMod[0]->del[0]-> * fb -> ap[0] -> del[1] * fb --> loop 2
		    tmp = AllpassModDelay_process(&r->apModDelay[0], (in + r->lastOut2));
		    tmp = VerbDelay_process(&r->delay[0], tmp);

		    tmp *= r->feedback;

		    tmp = AllpassDelay_process(&r->apDelay[1], (tmp));
		    tmp = VerbDelay_process(&r->delay[1], tmp);

		     r->lastOut = calcOnePole(&r->lpFeedback, tmp*r->feedback);





		    //r->lastOut = tmp * r->feedback;

		    //loop 2
		    // in2 + loop 1 out ->apMod[1]->del[2]-> * fb -> ap[1] -> del[3] * fb --> loop 1
		    tmp = AllpassModDelay_process(&r->apModDelay[1], (in2 + r->lastOut));
			tmp = VerbDelay_process(&r->delay[2], tmp);

			tmp *= r->feedback;

			tmp = AllpassDelay_process(&r->apDelay[1], (tmp));
			tmp = VerbDelay_process(&r->delay[3], tmp);

			r->lastOut2 = tmp * r->feedback;

		    /*/********* left output, all wet *********
		     accumulator = 0.6 X node48_54[266]
		     accumulator += 0.6 x node48_54[2974]
		     accumulator -= 0.6 X node55_59[1913]
		     accumulator += 0.6 X node59_63[1996]
		     accumulator -= 0.6 X node24_30[1990]
		     accumulator -= 0.6 x node31_33[187]
		     YL = accumulator - 0.6 X node33_39[1066]

		     ********* right output, all wet *********
		     ********* accumulator = 0.6 X node24_30[353]
		     ********* accumulator += 0.6 X node24_30[3627]
		     ********* accumulator -= 0.6 X node31_33[1228]
		     ********* accumulator += 0.6 X node33_39[2673]
		     ********* accumulator -= 0.6 X node48_54[2111]
		     ********* accumulator -= 0.6 X node55_59[335]
		     ********* YR = accumulator - 0.6 X node59_63[121] */
			/*
			delay[0], 2047);
			delay[1], 1913);
			delay[2], 1720);
			delay[3], 1163);

			apDelay[0], 1800);
			apDelay[1], 2039);

			apModDelay[0], 672);
			apModDelay[1], 908);
			*/



		    //read output taps
			out2  = VerbDelay_ReadTapSample(&r->delay[2], 266) * 0.6;
			//out2 += VerbDelay_ReadTapSample(&r->delay[2], 2046) * 0.6;
			out2 -= VerbDelay_ReadTapSample(&r->apDelay[1].delay, 1913) * 0.6;
			out2 += VerbDelay_ReadTapSample(&r->delay[3], 996) * 0.6;
			//out2 -= VerbDelay_ReadTapSample(&r->delay[0], 1990) * 0.6;
			out2 -= VerbDelay_ReadTapSample(&r->apDelay[0].delay, 187) * 0.6;
			out2 -= VerbDelay_ReadTapSample(&r->delay[1], 1066) * 0.6;

			out= TiltEq_process(&r->eqFb,fmaf(early, r->earlyVolume, out2));

#endif
			int16_t wet =  __SSAT(out*32768, 16);
			//int16_t wet2 =  __SSAT(out*32768, 16);
			//int16_t dw = fxBuffer[i] + __SSAT((wet - fxBuffer[i]) * dryWet,16);

			x = __PKHBT(fxBuffer[idx1] + __SSAT((wet - fxBuffer[idx1]) * dryWet,16), fxBuffer[idx2] + __SSAT((wet - fxBuffer[idx2]) * dryWet,16), 16);
			//LERP dryWet
			*stereoOut = __QADD16(*stereoOut, x);
			stereoOut++;
	}
}
IN_ITCM_RAM void Reverb_setDelayTime(Reverb *r, float t)
{
#if SPIN_SEMI
	if(r->delayTime != t)
	{
		r->delayTime = t;

		for(int i=0;i<2;i++)
		{
			VerbDelay_setRoomSize(&r->delay[i], t);
		}
		for(int i=0;i<2;i++)
		{
			VerbModDelay_setRoomSize(&r->modDelay[i], t);
		}

		VerbDelay_setRoomSize(&r->earlyReflectionDelay, t);
	}
#else
	/*
	  	//44 samples = 1ms, 2030 = 46ms
		VerbDelay_setLength(&r->preDelay, 2031);
	 */
	float sz = 0.02166f + t*0.97784342688330871492f;
	VerbDelay_setRoomSize(&r->preDelay, sz);
#endif
}

IN_ITCM_RAM void Reverb_setFeedback(Reverb *r, float fb)
{
	r->feedback = fb;//feedbackShaper.shape(fb);
}

IN_ITCM_RAM void Reverb_setComposition(Reverb *r, float c)
{
	TiltEq_setParameter(&r->eq, -1+2*c);
	TiltEq_setParameter(&r->eqFb, -1+2*c);
	//compositionParameter = c;
	setOnePoleCoef(&r->lpFeedback, c);
//	setOnePoleCoef(&r->lpImpulse, c);
}

IN_ITCM_RAM void Reverb_setVariation(Reverb *r, float v)
{
#if SPIN_SEMI
	//variationParameter = v;
	for(int i=0;i<4;i++)
	{
		const float scale = (i+7)/10.f;
		AllpassDelay_setCoefficient(&r->apDelay[i][0], v*scale);
		AllpassDelay_setCoefficient(&r->apDelay[i][1], v*scale*0.8);
		//AllpassDelay_setCoefficient(&r->inputAllpass[i], v*0.9*scale);
	}
#else
	r->earlyVolume = v;
#endif
}


#if 0

static const float earlyTaps[] =
{
		12,
		24,
		38,
		50
};

AllpassLoopVerb::AllpassLoopVerb()
{
	reverbBufferSram = (revItg*)sramBuffer;



	feedbackShaper.setShape(0.7f);
	lastOut = 0;
	lfo.setFreq(1.31f);  //1.18hz

	for(int i=0;i<4;i++)
	{
		apDelay[i][0].setBuffer(&reverbBufferSram[i*2*(MAX_ALLPASS_SIZE)]);
		apDelay[i][1].setBuffer(&reverbBufferSram[(i*2+1)*(MAX_ALLPASS_SIZE)]);
	}
	// 8 * MAX_ALLPASS_SIZE occupied

	for(int i=0;i<4;i++)
	{
		//TODO could have way less memory -> max delay is 379 samples
		inputAllpass[i].setBuffer(&reverbBufferSram[8*MAX_ALLPASS_SIZE + i*(MAX_ALLPASS_SIZE)]);
	}
	// 12 * MAX_ALLPASS_SIZE occupied
	for(int i=0;i<2;i++)
	{
		delay[i].setBuffer(&reverbBufferSram[12 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
	}
	for(int i=0;i<2;i++)
	{
		modDelay[i].setBuffer(&reverbBufferSram[14 * MAX_ALLPASS_SIZE + i*MAX_DELAY_SIZE]);
	}

	earlyReflectionDelay.setBuffer(&reverbBufferSram[16 * MAX_ALLPASS_SIZE]);
	earlyReflectionDelay.setLength(4000);

	inputAllpass[0].setDelay(142);
	inputAllpass[1].setDelay(107);
	inputAllpass[2].setDelay(379);
	inputAllpass[3].setDelay(277);

	delay[0].setLength(1283);
	delay[1].setLength(1109); //todo wieso habe ich nur halbe buffer size? größer 4096 geht nicht
//	delay[1].setLength(3090);

	modDelay[0].setLength(2393);
	modDelay[1].setLength(5281);

	apDelay[0][0].setDelay(853);
	apDelay[0][1].setDelay(757);

	apDelay[1][0].setDelay(947);
	apDelay[1][1].setDelay(743);

	apDelay[2][0].setDelay(953);
	apDelay[2][1].setDelay(523);

	apDelay[3][0].setDelay(197);
	apDelay[3][1].setDelay(877);

	modDelay[0].setModDepth(170);
    modDelay[1].setModDepth(170);

	for(int i=0;i<4;i++)
	{
			apDelay[i][0].setCoefficient(INITIAL_ALLPASS_COFF);
			apDelay[i][1].setCoefficient(INITIAL_ALLPASS_COFF);
	}
    apDelay[2][1].setCoefficient(-INITIAL_ALLPASS_COFF);

    inputAllpass[2].setCoefficient(-0.5);

}

void AllpassLoopVerb::setDelayTime(float t)
{
	if(delayTime != t)
	{
		delayTime = t;

		for(int i=0;i<2;i++)
		{
			delay[i].setRoomSize(t);
		}
		for(int i=0;i<2;i++)
		{
			modDelay[i].setRoomSize(t);
		}

		earlyReflectionDelay.setRoomSize(t);
	}
}

void AllpassLoopVerb::setComposition(float c)
{
	compositionParameter = c;
	lpFeedback.setFreq(c);
	lpImpulse.setFreq(c);
}

void AllpassLoopVerb::setVariation(float v)
{
	variationParameter = v;
	for(int i=0;i<4;i++)
		{
			apDelay[i][0].setCoefficient(v);
			apDelay[i][1].setCoefficient(v);
			inputAllpass[i].setCoefficient(v);
		}
}

void AllpassLoopVerb::setFeedback(float fb)
{
	feedback = feedbackShaper.shape(fb);
}


void AllpassLoopVerb::process(int16_t* input1, int16_t* input2)
{
	revItg tmp;
	revItg in, out, out2;

	 lfo.calc();
	 modDelay[1].setModValue(lfo.getSine());
	 modDelay[0].setModValue(lfo.getCosine());



	 in = 0.5*lpImpulse.process(inputAllpass[0].process(inputAllpass[1].process(inputAllpass[2].process(inputAllpass[3].process((*input1)<<16)))));


	 	 //############## early reverb ####################

	 float early = 0;
	 float vol = 0.25f;
	 earlyReflectionDelay.process(*input1);
	 for(int j=0;j<EARLY_TAPS;j++)
	 {
	   early += earlyReflectionDelay.ReadTap(earlyTaps[j]) * vol ;
	   vol *= 0.5f;
	 }


	 //############## Late reverb ####################

	    tmp = apDelay[0][0].process(in + lastOut);
	    tmp = apDelay[0][1].process(tmp);

	    tmp = delay[0].process(tmp);

	    out = tmp;

	    out = sub((int16_t)(delay[0].ReadTap(75)*0.83f),out);
	    out = add((int16_t)(delay[0].ReadTap(33)*0.2f),out);

	    out2 = tmp * 0.1;



	    tmp = apDelay[1][0].process(tmp*feedback);
	    tmp = apDelay[1][1].process(tmp);
	    tmp = modDelay[0].process(tmp);

	     out = add((int16_t)(tmp*0.2f),out);


	     out2 = add((int16_t)(tmp*0.67),out2);

	    tmp = apDelay[2][0].process(in + tmp*feedback);
	    tmp = apDelay[2][1].process(tmp);
	    tmp = delay[1].process(tmp);

	    out2 = add((int16_t)(tmp * 0.5),out2);;
	    out2 = sub((int16_t)(delay[2].ReadTap(33)*0.7),out2);;

	    out = add((int16_t)(tmp*0.7f),out);;


	    tmp = apDelay[3][0].process(tmp*feedback);
	    tmp = apDelay[3][1].process(tmp);
	    tmp = modDelay[1].process(tmp);

	    lastOut = lpFeedback.process(tmp*feedback);

	    out = add((int16_t)(tmp*0.1f),out);


	    out2 = add((int16_t)(tmp),out2);;
	    out2 = add((int16_t)(modDelay[1].interpolatedReadTap(17)*0.2),out2);



	   // output[0][i] = interpolate(input[0][i],output[0][i],dryWet) + early * earlyMix;
	   // output[1][i] = interpolate(input[1][i],output[1][i],dryWet) + early * earlyMix;
	  // return  dryWet*(__QADD16(out*0.5f,out2*0.5f) ) + (1-dryWet)*input;;
	    //return  (((revItg)(dryWet*(add(out , out2>>1) )))>>16) + (1-dryWet)*input;;

	    out = (add(out , out2>>1)>>16);

	    *input1 =  dryWetMixer(out,*input1,dryWet);


	   // return  (((revItg)(dryWet*(add(out , out2) )))>>16) + (1-dryWet)*input;;

//	   return  dryWet*out + (1-dryWet)*input;

}

#endif
