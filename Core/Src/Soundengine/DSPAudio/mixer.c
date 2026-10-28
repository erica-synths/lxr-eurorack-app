/*
 * mixer.c
 *
 *  Created on: 11.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include <Soundengine/ModMatrix_Processor.h>
#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/mixer.h"
#include "Soundengine/config.h"
#include "Soundengine/AudioCodecManager.h"
#include "Soundengine/DSPAudio/CymbalVoice.h"
#include "Soundengine/DSPAudio/DrumVoice.h"
#include "Soundengine/DSPAudio/Snare.h"
#include "Soundengine/DSPAudio/HiHat.h"
#include "Soundengine/DSPAudio/BufferTools.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "main.h"
#include "menu/menu.h"
#include "Soundengine/FxBus.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"

	uint8_t r2_Available;
	uint8_t l1_Available;
	uint8_t l2_Available;
	uint8_t r1_Available;


//-----------------------------------------------------------------------
INDTCMZ uint8_t mixer_audioRouting[6];
//-----------------------------------------------------------------------
#if USE_DECIMATOR
INDTCMZ float mixer_decimation_rate[7];		/**<sets the sample rate decimation. 0..1 = full rate*/
INDTCMZ float mixer_decimation_cnt[6];		/**<s'n'h counter for decimator*/
INDTCMZ int16_t mixer_voice_samples[6];		/**< stores the last outputted sample of the 6 voices*/
#endif
//-----------------------------------------------------------------------
void mixer_init()
{
#if USE_DECIMATOR
	int i;
	for(i=0;i<6;i++)
	{
		mixer_decimation_rate[i] 	= 1;
		mixer_decimation_cnt[i] 	= 0;
		mixer_voice_samples[i] 		= 0;
		mixer_audioRouting[i]		= 0;
	}
	mixer_decimation_rate[6] 		= 1;
#endif

	FxBus_init();
}
//-----------------------------------------------------------------------
IN_ITCM_RAM void mixer_decimateBlock(const uint8_t voiceNr, int16_t* buffer)
{
	uint8_t i;
	float phaseInc = mixer_decimation_rate[voiceNr]*mixer_decimation_rate[6];
	for(i=0;i<OUTPUT_DMA_SIZE;i++)
	{
		mixer_decimation_cnt[voiceNr] += phaseInc;
		if(mixer_decimation_cnt[voiceNr] >= 1.f)
		{
			mixer_decimation_cnt[voiceNr] -= 1.f;
			mixer_voice_samples[voiceNr] = buffer[i];

		}
		buffer[i] = mixer_voice_samples[voiceNr];
	}
}
//#pragma GCC push_options
//#pragma GCC optimize ("O0")
//-----------------------------------------------------------------------
/*IN_ITCM_RAM*/ uint8_t mixer_checkOutJackAvailable(uint8_t dest)
{
	//read input pins
	//TODO move pin reading in low priority idle loop
	/*
	 #define switch_l1_Pin GPIO_PIN_6
#define switch_l1_GPIO_Port GPIOD
#define switch_r1_Pin GPIO_PIN_7
#define switch_r1_GPIO_Port GPIOD
#define switch_l2_Pin GPIO_PIN_3
#define switch_l2_GPIO_Port GPIOB
#define switch_r2_Pin GPIO_PIN_4
#define switch_r2_GPIO_Port GPIOB
	 */


	//return MIXER_ROUTING_DAC2_STEREO;

	switch(dest)
	{
		case MIXER_ROUTING_FX_BUS:
			return MIXER_ROUTING_FX_BUS;
			break;

		case MIXER_ROUTING_DAC1_STEREO:
			if(r1_Available && l1_Available) {
				return MIXER_ROUTING_DAC1_STEREO;
			} else
			if (r2_Available && l2_Available) {
				return MIXER_ROUTING_DAC2_STEREO;
			}
			else if(r1_Available) {
				return MIXER_ROUTING_DAC1_R;
			}else if(l1_Available) {
				return MIXER_ROUTING_DAC1_L;
			} else
			if (r2_Available) {
				return MIXER_ROUTING_DAC2_R;
			}else
			if (l2_Available) {
				return MIXER_ROUTING_DAC2_L;
			}
			break;

		case MIXER_ROUTING_DAC2_STEREO:

			if (r2_Available && l2_Available) {
				return MIXER_ROUTING_DAC2_STEREO;
			} else
			if(r1_Available && l1_Available) {
				return MIXER_ROUTING_DAC1_STEREO;
			}
			else
			if (r2_Available) {
				return MIXER_ROUTING_DAC2_R;
			}else
			if (l2_Available) {
				return MIXER_ROUTING_DAC2_L;
			}else if(r1_Available) {
				return MIXER_ROUTING_DAC1_R;
			}else if(l1_Available) {
				return MIXER_ROUTING_DAC1_L;
			}
			break;

		case MIXER_ROUTING_DAC1_L:
			if(l1_Available) {
				return dest;
			} else if (r1_Available) {
				return MIXER_ROUTING_DAC1_R;
			} else if (l2_Available) {
				return MIXER_ROUTING_DAC2_L;
			} else if (r2_Available) {
				return MIXER_ROUTING_DAC2_R;
			}
			break;

		case MIXER_ROUTING_DAC1_R:
			if(r1_Available) {
				return dest;
			} else if (l1_Available) {
				return MIXER_ROUTING_DAC1_L;
			} else if (l2_Available) {
				return MIXER_ROUTING_DAC2_L;
			} else if (r2_Available) {
				return MIXER_ROUTING_DAC2_R;
			}
			break;

		case MIXER_ROUTING_DAC2_L:
			if(l2_Available) {
				return dest;
			} else if (r2_Available) {
				return MIXER_ROUTING_DAC2_R;
			} else if (l1_Available) {
				return MIXER_ROUTING_DAC1_L;
			} else if (r1_Available) {
				return MIXER_ROUTING_DAC1_R;
			}
			break;

		case MIXER_ROUTING_DAC2_R:
			if(r2_Available) {
				return dest;
			} else if (l2_Available) {
				return MIXER_ROUTING_DAC2_L;
			} else if (r1_Available) {
				return MIXER_ROUTING_DAC1_R;
			} else if (l1_Available) {
				return MIXER_ROUTING_DAC1_L;
			}
			break;
	}
	return dest;
}

//-----------------------------------------------------------------------
#if 0
static inline void mixer_moveDataToOutput(uint8_t dest, const float panL, const float panR, int16_t* data,int16_t* outL,int16_t* outR,int16_t* outL2, int16_t* outR2)
{
	//check if a cable is in the selected out
	dest = mixer_checkOutJackAvailable(dest);

	uint8_t i;
	switch(dest)
	{

	case MIXER_ROUTING_DAC1_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL2 = data[i] * panL;
			outL2 += 2;

			*outR2 = data[i] * panR;
			outR2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL = data[i] * panL;
			outL += 2;

			*outR = data[i] * panR;
			outR += 2;
		}
		break;
	case MIXER_ROUTING_DAC1_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL2 = data[i];
			outL2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC1_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR2 = data[i];
			outR2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL = data[i];
			outL += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR = data[i];
			outR += 2;
		}
		break;
	}
}
#endif
//-----------------------------------------------------------------------
#if 0
static float fastTanh(float var)
{
   if(var < -1.95f)     return -1.0f;
   else if(var > 1.95f) return  1.0f;
   else					return  4.15f*var/(4.29f+var*var);
}




double ecc83(float x, float v)
{
	/*
	double input = x -0.05*v*x +0.01*v;//(h100f(x -0.05*v*x +0.01*v))*(1.0+0.55*v) +0.55*v;
	input += 1.0; // [0:2]
	input *= 64.0;
	uint8_t itg = input;
	float frac = input-itg;
	double val;
	if(itg<127)
		val = (1.0-frac) * ecc83Table[itg] + frac * ecc83Table[itg+1];
	else val = ecc83Table[127];
	val = val *(1.0+0.55*v) +0.55*v;
	//if(val>0.999999)val =0.999999;
	return val;
	*/
}

void mixer_saturateOutputs(float gain,  int16_t* outL,int16_t* outR,int16_t* outL2, int16_t* outR2)
{
    gain = 1+gain*2;
    //float v = gain
	for(uint32_t i=0;i<OUTPUT_DMA_SIZE;i++)
	{
		/*
		*outR = __SSAT((int32_t)(ecc83(*outR/32768.0f, gain) * 32768.0f), 16); ;
		outR += 2;

		*outL =  __SSAT((int32_t)(ecc83(*outL/32768.0f, gain) * 32768.0f), 16); ;
		outL += 2;
		*/

		*outR2 =  __SSAT((int32_t)(ecc83(*outR2/32768.0f*gain, 1.) * 32768.0f), 16); ;
		outR2 += 2;

		*outL2 =  __SSAT((int32_t)(ecc83(*outL2/32768.0f*gain, 1.) * 32768.0f), 16); ;
		outL2 += 2;
	}
}
#endif
//-----------------------------------------------------------------------
IN_ITCM_RAM static inline void mixer_addDataToOutputFilteredPan(uint8_t dest, const float pan,  int16_t* data,int16_t* outL,int16_t* outR,int16_t* outL2, int16_t* outR2, int16_t *fxBuffer, OnePoleFilter* filter)
{


	//check if a cable is in the selected out
	dest = mixer_checkOutJackAvailable(dest);
	//assert_param(dest <= MIXER_ROUTING_DAC2_R);

	//TODO may be possible to optimize here using both halfwords of add for stereo mixing
	uint8_t i;
	switch(dest)
	{

	case MIXER_ROUTING_FX_BUS:
#if 0
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			fxBuffer[i] = __QADD16(fxBuffer[i],data[i]) & 0xFFFF;
		}
#else
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{

			//----
			float mixPan 	= calcOnePole(filter, pan);
			float panL = sqrtf((127-mixPan)/127.f);
			float panR = sqrtf(mixPan/127.f);
			//----
			int32_t y;
			int32_t x;
			y = *((int32_t*)fxBuffer);
			x = __PKHBT( (int16_t)(data[i] * panL), (int16_t)(data[i] * panR), 16);

			*((int32_t*)fxBuffer) = __QADD16(x, y);
			fxBuffer += 2;
		}
#endif
		break;

	case MIXER_ROUTING_DAC1_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			//----
			float mixPan 	= calcOnePole(filter, pan);
			float panL = sqrtf((127-mixPan)/127.f);
			float panR = sqrtf(mixPan/127.f);
			//----
#if 1
			int32_t y;
			int32_t x;
			y = *((int32_t*)outL2);
			//x = ((int16_t)(data[i] * panL)& 0xFFFF) | (((int16_t)(data[i] * panR)& 0xFFFF)<<16);
			x = __PKHBT( (int16_t)(data[i] * panL), (int16_t)(data[i] * panR), 16);
			*((int32_t*)outL2) = __QADD16(x, y);
			outL2 += 2;
#else


			*outL2 = __QADD16(*outL2,(int16_t)(data[i] * panL)) & 0xFFFF;
			outL2 += 2;

			*outR2 = __QADD16(*outR2,(int16_t)(data[i] * panR)) & 0xFFFF;
			outR2 += 2;
#endif
		}
		break;
	case MIXER_ROUTING_DAC2_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			//----
			float mixPan 	= calcOnePole(filter, pan);
			float panL = sqrtf((127-mixPan)/127.f);
			float panR = sqrtf(mixPan/127.f);
			//----
#if 1
			int32_t x,y;
			y = *((int32_t*)outL);
			//simdBuffer x;
			//x = ((int16_t)(data[i] * panL)& 0xFFFF) | (((int16_t)(data[i] * panR)& 0xFFFF)<<16);
			//*((int32_t*)outL) = __SADD16(x, y);
			//x.word[0] = data[i] * panL;
			//x.word[1] = data[i] * panR;
			x = __PKHBT(data[i] * panL, data[i] * panR, 16);
			*((int32_t*)outL) = __QADD16(x, y);
			outL += 2;
#else
			*outL = __QADD16(*outL,(int16_t)(data[i] * panL)) & 0xFFFF;
			outL += 2;

			*outR = __QADD16(*outR,(int16_t)(data[i] * panR)) & 0xFFFF;
			outR += 2;
#endif
		}
		break;
	case MIXER_ROUTING_DAC1_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{

			*outL2 = __QADD16(*outL2,data[i]);
			outL2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC1_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR2 = __QADD16(*outR2,data[i]) ;
			outR2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL = __QADD16(*outL,data[i]);
			outL += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR = __QADD16(*outR,data[i]);
			outR += 2;
		}
		break;

	}
}
//-----------------------------------------------------------------------
IN_ITCM_RAM static inline void mixer_addDataToOutput(uint8_t dest, const float panL, const float panR,  int16_t* data,int16_t* outL,int16_t* outR,int16_t* outL2, int16_t* outR2, int16_t *fxBuffer)
{
	//check if a cable is in the selected out
	dest = mixer_checkOutJackAvailable(dest);
	//assert_param(dest <= MIXER_ROUTING_DAC2_R);

	//TODO may be possible to optimize here using both halfwords of add for stereo mixing
	uint8_t i;
	switch(dest)
	{

	case MIXER_ROUTING_FX_BUS:
#if 0
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			fxBuffer[i] = __QADD16(fxBuffer[i],data[i]) & 0xFFFF;
		}
#else
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			int32_t y;
			int32_t x;
			y = *((int32_t*)fxBuffer);
			x = __PKHBT( (int16_t)(data[i] * panL), (int16_t)(data[i] * panR), 16);

			*((int32_t*)fxBuffer) = __QADD16(x, y);
			fxBuffer += 2;
		}
#endif
		break;

	case MIXER_ROUTING_DAC1_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
#if 1
			int32_t y;
			int32_t x;
			y = *((int32_t*)outL2);
			//x = ((int16_t)(data[i] * panL)& 0xFFFF) | (((int16_t)(data[i] * panR)& 0xFFFF)<<16);
			x = __PKHBT( (int16_t)(data[i] * panL), (int16_t)(data[i] * panR), 16);
			*((int32_t*)outL2) = __QADD16(x, y);
			outL2 += 2;
#else


			*outL2 = __QADD16(*outL2,(int16_t)(data[i] * panL)) & 0xFFFF;
			outL2 += 2;

			*outR2 = __QADD16(*outR2,(int16_t)(data[i] * panR)) & 0xFFFF;
			outR2 += 2;
#endif
		}
		break;
	case MIXER_ROUTING_DAC2_STEREO:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
#if 1
			int32_t x,y;
			y = *((int32_t*)outL);
			//simdBuffer x;
			//x = ((int16_t)(data[i] * panL)& 0xFFFF) | (((int16_t)(data[i] * panR)& 0xFFFF)<<16);
			//*((int32_t*)outL) = __SADD16(x, y);
			//x.word[0] = data[i] * panL;
			//x.word[1] = data[i] * panR;
			x = __PKHBT( (int16_t)(data[i] * panL), (int16_t)(data[i] * panR), 16);
			//x = __PKHBT(data[i] * panL, data[i] * panR, 16);
			*((int32_t*)outL) = __QADD16(x, y);
			outL += 2;
#else
			*outL = __QADD16(*outL,(int16_t)(data[i] * panL)) & 0xFFFF;
			outL += 2;

			*outR = __QADD16(*outR,(int16_t)(data[i] * panR)) & 0xFFFF;
			outR += 2;
#endif
		}
		break;
	case MIXER_ROUTING_DAC1_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL2 = __QADD16(*outL2,data[i]);
			outL2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC1_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR2 = __QADD16(*outR2,data[i]) ;
			outR2 += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_L:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outL = __QADD16(*outL,data[i]);
			outL += 2;
		}
		break;
	case MIXER_ROUTING_DAC2_R:
		for(i=0;i<OUTPUT_DMA_SIZE;i++)
		{
			*outR = __QADD16(*outR,data[i]);
			outR += 2;
		}
		break;

	}
}
//#pragma GCC pop_options
//-----------------------------------------------------------------------
IN_ITCM_RAM void mixer_calcNextSampleBlock(int16_t* output,int16_t* output2)
{
	voiceControl_processQueue();
#if !EURORACK_MODULE
	r2_Available = (switch_r2_GPIO_Port->IDR & switch_r2_Pin)>0;
	l1_Available = (switch_l1_GPIO_Port->IDR & switch_l1_Pin)>0;
	l2_Available = (switch_l2_GPIO_Port->IDR & switch_l2_Pin)>0;
	r1_Available = (switch_r1_GPIO_Port->IDR & switch_r1_Pin)>0;
#endif


#if OLD_LFO_ROUTING
	//re assign velocity modulation
	modNode_reassignVeloMod();
#endif
	//calc and dispatch LFO
	lfo_calcNextValue(&voiceArray[0].lfo);
	lfo_calcNextValue(&voiceArray[1].lfo);
	lfo_calcNextValue(&voiceArray[2].lfo);
	lfo_calcNextValue(&snareVoice.lfo);
	lfo_calcNextValue(&cymbalVoice.lfo);
	lfo_calcNextValue(&hatVoice.lfo);

	modNode_resetTargets();

	//update filter frequencies
	SVF_recalcFreq(&voiceArray[0].filter);
	SVF_recalcFreq(&voiceArray[1].filter);
	SVF_recalcFreq(&voiceArray[2].filter);
	SVF_recalcFreq(&snareVoice.filter);
	SVF_recalcFreq(&cymbalVoice.filter);
	SVF_recalcFreq(&hatVoice.filter);

	//process CV
	CV_process();

	//--- Calc async -----
	calcDrumVoiceAsync(0);
	calcDrumVoiceAsync(1);
	calcDrumVoiceAsync(2);

	Snare_calcAsync();
	Cymbal_calcAsync();
	HiHat_calcAsync();

	//calculate trigger io phase
	//trigger_tickPhaseCounter();

	//an array to store intermediate voice samples
	//befor output distribution
	int16_t sampleData[OUTPUT_DMA_SIZE];
	int16_t fxBusData[OUTPUT_DMA_SIZE*2];

	const uint8_t pos = 0;

	//Clear is necessary, otherwise we keep trash data on non routed outputs
	bufferTool_clearBuffer(output,OUTPUT_DMA_SIZE*2);
	bufferTool_clearBuffer(output2,OUTPUT_DMA_SIZE*2);

	bufferTool_clearBuffer(fxBusData,OUTPUT_DMA_SIZE*2);

	//calc voice 1
	calcDrumVoiceSyncBlock(0, sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(0,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[0],squareRootLut[127-voiceArray[0].pan] ,squareRootLut[voiceArray[0].pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else

#if 0//SMOOTH_MIXER_VOL_PAN_DRIVE
	//float mixPan 	= calcOnePole( &voiceArray[0].MixerPageFilter[1], voiceArray[0].pan);
	//mixer_addDataToOutput(mixer_audioRouting[0],sqrtf((127-mixPan)/127.f) ,sqrtf(mixPan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
	mixer_addDataToOutputFilteredPan(mixer_audioRouting[0], voiceArray[0].pan, sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData, &voiceArray[1].MixerPageFilter[0]);
#else

#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[0],voiceArray[0].triggeredPanL  ,voiceArray[0].triggeredPanR , sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[0],sqrtf((127-voiceArray[0].pan)/127.f) ,sqrtf(voiceArray[0].pan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif
#endif

#if 1

	//calc voice 2
	calcDrumVoiceSyncBlock(1, sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(1,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[1],squareRootLut[127-voiceArray[1].pan] ,squareRootLut[voiceArray[1].pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
#if 0//SMOOTH_MIXER_VOL_PAN_DRIVE
	//mixPan 	= calcOnePole( &voiceArray[1].MixerPageFilter[1], voiceArray[1].pan);
	//mixer_addDataToOutput(mixer_audioRouting[1],sqrtf((127-mixPan)/127.f) ,sqrtf(mixPan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
	mixer_addDataToOutputFilteredPan(mixer_audioRouting[1], voiceArray[1].pan, sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData, &voiceArray[1].MixerPageFilter[1]);
#else
#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[1],voiceArray[1].triggeredPanL  ,voiceArray[1].triggeredPanR , 			sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[1],sqrtf((127-voiceArray[1].pan)/127.f) ,sqrtf(voiceArray[1].pan/127.f), 	sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif
#endif
	//calc voice 3
	calcDrumVoiceSyncBlock(2, sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(2,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[2],squareRootLut[127-voiceArray[2].pan] ,squareRootLut[voiceArray[2].pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
#if 0//SMOOTH_MIXER_VOL_PAN_DRIVE

	mixer_addDataToOutputFilteredPan(mixer_audioRouting[2], voiceArray[2].pan, sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData, &voiceArray[2].MixerPageFilter[1]);
#else
#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[2],voiceArray[2].triggeredPanL  ,voiceArray[2].triggeredPanR , 			sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[2],sqrtf((127-voiceArray[2].pan)/127.f) ,sqrtf(voiceArray[2].pan/127.f), 	sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif
#endif
	//calc snare
	Snare_calcSyncBlock(sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(3,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[3],squareRootLut[127-snareVoice.pan], squareRootLut[snareVoice.pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else

#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[3],snareVoice.triggeredPanL  ,snareVoice.triggeredPanR , 			sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[3],sqrtf((127-snareVoice.pan)/127.f), sqrtf(snareVoice.pan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif
	//calc cymbal
	Cymbal_calcSyncBlock(sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(4,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[4],squareRootLut[127-cymbalVoice.pan],	squareRootLut[cymbalVoice.pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[4],cymbalVoice.triggeredPanL  ,cymbalVoice.triggeredPanR , 			sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[4],sqrtf((127-cymbalVoice.pan)/127.f),	sqrtf(cymbalVoice.pan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif


	//calc HiHat
	HiHat_calcSyncBlock(sampleData,OUTPUT_DMA_SIZE);
	//decimate voice
	mixer_decimateBlock(5,sampleData);
	//copy to selected dma buffer
#if USE_SQRT_LUT
	mixer_addDataToOutput(mixer_audioRouting[5],squareRootLut[127-hatVoice.pan],squareRootLut[hatVoice.pan], sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
#if USE_TRIGGERED_PAN
	mixer_addDataToOutput(mixer_audioRouting[5],hatVoice.triggeredPanL  ,hatVoice.triggeredPanR , 			sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#else
	mixer_addDataToOutput(mixer_audioRouting[5],sqrtf((127-hatVoice.pan)/127.f),sqrtf(hatVoice.pan/127.f), sampleData,&output[pos],&output[pos+1],&output2[pos],&output2[pos+1], fxBusData);
#endif
#endif

#endif
	FxBus_process(fxBusData,  &output[pos],&output[pos+1],&output2[pos],&output2[pos+1], OUTPUT_DMA_SIZE);
/*
	if(activeParameters[UNUSED_PAR_GLO_SAT])
	{
		mixer_saturateOutputs(activeParameters[UNUSED_PAR_GLO_SAT]/127.f, &output[pos],&output[pos+1],&output2[pos],&output2[pos+1]);
	}
	*/


}
