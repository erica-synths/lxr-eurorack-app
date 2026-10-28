/*
 * HiHat.c
 *
 *  Created on: 18.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/HiHat.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "IO/adcPots.h"
#include "Soundengine/FxBus.h"

INDTCMZ HiHatVoice hatVoice;

//---------------------------------------------------
void HiHat_setPan(const uint8_t pan)
{
	//hatVoice.panL = squareRootLut[127-pan];
	//hatVoice.panR = squareRootLut[pan];
	hatVoice.pan = pan;
}
//---------------------------------------------------

void HiHat_init()
{
//	initOnePole(&hatVoice.FaderVolumeFilter);
//	setOnePoleCoef(&hatVoice.FaderVolumeFilter,0.1);

	SnapEg_init(&hatVoice.snapEg);
	HiHat_setPan(0);
	hatVoice.vol = 0.8f;
	hatVoice.cvFreq = 0.5f;

	hatVoice.decayClosed = 0.002f;
	hatVoice.decayOpen = 0.002f;

	//hatVoice.panModifier = 1.f;

	transient_init(&hatVoice.transGen);

	hatVoice.fmModAmount1 = 0.5f;
	hatVoice.fmModAmount2 = 0.5f;

	setDistortionShape(&hatVoice.distortion, 0.f);

	hatVoice.modOsc.freq = 440;
	hatVoice.modOsc.waveform = SINE;
	hatVoice.modOsc.fmMod = 0;
	hatVoice.modOsc.midiFreq = 70<<8 | 64 ;
	hatVoice.modOsc.pitchMod = 1.0f;
	hatVoice.modOsc.modNodeValue = 1;
	hatVoice.modOsc.cvFreq = -1.f;

	hatVoice.modOsc2.freq = 440;
	hatVoice.modOsc2.waveform = NOISE;//SINE;
	hatVoice.modOsc2.fmMod = 0;
	hatVoice.modOsc2.midiFreq = 70<<8 | 64;
	hatVoice.modOsc2.pitchMod = 1.0f;
	hatVoice.modOsc2.modNodeValue = 1;
	hatVoice.modOsc2.cvFreq = -1.f;

	hatVoice.osc.freq = 440;
	hatVoice.osc.waveform = 1;
	hatVoice.osc.fmMod = 1;
	hatVoice.osc.midiFreq = 70<<8 | 64;
	hatVoice.osc.pitchMod = 1.0f;
	hatVoice.osc.modNodeValue = 1;
	hatVoice.osc.cvFreq = -1.f;

	hatVoice.volumeMod = 1;

	slopeEg2_init(&hatVoice.oscVolEg);

	SVF_init(&hatVoice.filter);

	lfo_init(&hatVoice.lfo);
}

//---------------------------------------------------
void HiHat_updatePan()
{
	hatVoice.triggeredPanL = sqrtf((127-hatVoice.pan)/127.f);
	hatVoice.triggeredPanR = sqrtf(hatVoice.pan/127.f);
}
//---------------------------------------------------
void HiHat_setCv1VOct(float oct)
{
	hatVoice.osc.cvFreq		= oct;		// the main fm oscillator
	hatVoice.modOsc.cvFreq	= oct;		// the mod osc
	hatVoice.modOsc2.cvFreq	= oct;		// the mod osc
	hatVoice.transGen.cvFreq = oct;
}
//---------------------------------------------------
void HiHat_trigger( uint8_t vel, uint8_t isOpen, const uint8_t note)
{
	lfo_retrigger(5);

	//update velocity modulation
#if OLD_LFO_ROUTING
	modNode_updateValue(&velocityModulators[5],vel/127.f);
#else
	hatVoice.velocity = vel/127.f;
#endif
	float offset = 1;
	if(hatVoice.transGen.waveform==1) //offset mode
	{
		offset -= hatVoice.transGen.volume;
	}
	if(hatVoice.osc.waveform == SINE)
		hatVoice.osc.phase = (0x3ff<<20)*offset;//voiceArray[voiceNr].osc.startPhase ;
	else if(hatVoice.osc.waveform > SINE && hatVoice.osc.waveform <= REC)
		hatVoice.osc.phase = (0xff<<22)*offset;
	else
		hatVoice.osc.phase = 0;

	hatVoice.modOsc.phase = 0;
	hatVoice.modOsc2.phase = 0;

	osc_setBaseNote(&hatVoice.osc,note);
	osc_setBaseNote(&hatVoice.modOsc,note);
	osc_setBaseNote(&hatVoice.modOsc2,note);

	hatVoice.isOpen = isOpen;
	hatVoice.oscVolEg.decay = isOpen?hatVoice.decayOpen:hatVoice.decayClosed;

	slopeEg2_trigger(&hatVoice.oscVolEg);
	hatVoice.velo = vel/127.f;
	transient_trigger(&hatVoice.transGen);

	SnapEg_trigger(&hatVoice.snapEg);

	HiHat_updatePan();
}
//---------------------------------------------------
IN_ITCM_RAM void HiHat_calcAsync( )
{
	//calc the osc  vol eg
	hatVoice.egValueOscVol = slopeEg2_calc(&hatVoice.oscVolEg);

	//turn off trigger signal if trigger gate mode is on and volume == 0
	/*
	if(trigger_isGateModeOn())
	{
		if(!hatVoice.egValueOscVol)
		{
			if(hatVoice.isOpen)
			{
				//trigger_triggerVoice(TRIGGER_7, TRIGGER_OFF);
				voiceControl_noteOff(TRIGGER_7);
			} else {
				//trigger_triggerVoice(TRIGGER_6, TRIGGER_OFF);
				voiceControl_noteOff(TRIGGER_6);
			}
		}
	}
	*/

	//calc snap EG if transient sample 0 is activated
	if(hatVoice.transGen.waveform == 0)
	{
		const float snapVal = SnapEg_calc(&hatVoice.snapEg, hatVoice.transGen.pitch);
		hatVoice.osc.pitchMod = 1 + snapVal*hatVoice.transGen.volume;
	}

	osc_setFreq(&hatVoice.osc);
	osc_setFreq(&hatVoice.modOsc);
	osc_setFreq(&hatVoice.modOsc2);


}
//---------------------------------------------------
void HiHat_calcSyncBlock(int16_t* buf, const uint8_t size)
{
	//2 buffers for the mod oscs
	int16_t mod1[size],mod2[size];
	//calc next mod osc samples, scaled with mod amount
	calcNextOscSampleBlock(&hatVoice.modOsc,mod1,size, hatVoice.fmModAmount1, hatVoice.pwm);
	calcNextOscSampleBlock(&hatVoice.modOsc2,mod2,size,  hatVoice.fmModAmount2, hatVoice.pwm);

	//combine both mod oscs to 1 modulation signal
	bufferTool_addBuffersSaturating(mod1,mod2,size);

	calcNextOscSampleFmBlock(&hatVoice.osc,mod1,buf,size,0.5f, hatVoice.pwm) ;

#if USE_REVERB
	if(fxBus.fxType != FX_REVERB)
	{
#endif
		SVF_calcBlockZDF(&hatVoice.filter,hatVoice.filterType,buf,size);

#if USE_REVERB
	}
	else
	{
		SVF_calcBlockCheapZDF(&hatVoice.filter,hatVoice.filterType,buf,size);
	}
#endif
	//calc transient sample
	transient_calcBlock(&hatVoice.transGen,mod1,size);

	uint8_t j;
	if(hatVoice.volumeMod)
	{
		hatVoice.combinedVolume = hatVoice.vol * hatVoice.velo * hatVoice.egValueOscVol; ;
		//const float vol =

#if 1
			int32_t* x = (int32_t*)buf;
			int32_t* y = (int32_t*)mod1;

			//add filter to buffer
			for(uint8_t i=0;i<size/2;i++)
			{
				x[i] = __QADD16(x[i],y[i]);
			}

			for(j=0;j<size;j++)
			{
				//add volume
				buf[j] *=  hatVoice.combinedVolume ;//* calcOnePole( &hatVoice.FaderVolumeFilter, adc_getFaderVolume(5));
			}
#else
		for(j=0;j<size;j++)
		{
			//add filter to buffer
			buf[j] = __QADD16(buf[j],mod1[j]);
			buf[j] *= vol;
		}
#endif
	}
	else
	{
		hatVoice.combinedVolume = hatVoice.vol * hatVoice.egValueOscVol;
		//const float vol = hatVoice.combinedVolume ;
#if 1
			int32_t* x = (int32_t*)buf;
			int32_t* y = (int32_t*)mod1;

			//add filter to buffer
			for(uint8_t i=0;i<size/2;i++)
			{
				x[i] = __QADD16(x[i],y[i]);
			}

			for(j=0;j<size;j++)
			{
				//add volume
				buf[j] *=  hatVoice.combinedVolume ;//* calcOnePole( &hatVoice.FaderVolumeFilter, adc_getFaderVolume(5));
			}
#else
		for(j=0;j<size;j++)
		{
			//add filter to buffer
			buf[j] = __QADD16(buf[j],mod1[j]);

			buf[j] *= vol;
		}
#endif
	}

	calcDistBlock(&hatVoice.distortion,buf,size);
}
//---------------------------------------------------

