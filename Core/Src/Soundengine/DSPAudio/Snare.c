/*
 * Snare.c
 *
 *  Created on: 17.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/Snare.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "IO/adcPots.h"
#include "Soundengine/FxBus.h"
//instance of the snare voice
INDTCMZ SnareVoice snareVoice;

//---------------------------------------------------
void Snare_setPan(const uint8_t pan)
{
	//snareVoice.panL = squareRootLut[127-pan];
	//snareVoice.panR = squareRootLut[pan];
	snareVoice.pan = pan;
}
//---------------------------------------------------
void Snare_init()
{
//	initOnePole(&snareVoice.FaderVolumeFilter);
//	setOnePoleCoef(&snareVoice.FaderVolumeFilter,0.1);

	SnapEg_init(&snareVoice.snapEg);
	Snare_setPan(0);
	snareVoice.vol = 0.8f;
	snareVoice.cvFreq = 0.5f;
	//snareVoice.panModifier = 1.f;

	snareVoice.noiseOsc.freq = 440;
	snareVoice.noiseOsc.waveform = 1;
	snareVoice.noiseOsc.fmMod = 0;
	snareVoice.noiseOsc.midiFreq = 70<<8 | 64;
	snareVoice.noiseOsc.pitchMod = 1.0f;
	snareVoice.noiseOsc.modNodeValue = 1;
	snareVoice.noiseOsc.cvFreq = -1.f;

	snareVoice.osc.freq = 440;
	snareVoice.osc.waveform = 1;
	snareVoice.osc.fmMod = 0;
	snareVoice.osc.midiFreq = 70<<8 | 64;
	snareVoice.osc.modNodeValue = 1;
	snareVoice.osc.cvFreq = -1.f;

	setDistortionShape(&snareVoice.distortion, 0.f);

	snareVoice.volumeMod = 1;

	transient_init(&snareVoice.transGen);

	DecayEg_init(&snareVoice.oscPitchEg);
	snareVoice.egPitchModAmount = 0.5f;

	slopeEg2_init(&snareVoice.oscVolEg);



	SVF_init(&snareVoice.filter);

	lfo_init(&snareVoice.lfo);
}
//---------------------------------------------------
void Snare_updatePan()
{
	snareVoice.triggeredPanL = sqrtf((127-snareVoice.pan)/127.f);
	snareVoice.triggeredPanR = sqrtf(snareVoice.pan/127.f);
}
//---------------------------------------------------
void Snare_setCv1VOct(float oct)
{
	snareVoice.osc.cvFreq		= oct;
	snareVoice.noiseOsc.cvFreq	= oct;
	snareVoice.transGen.cvFreq = oct;
}
//---------------------------------------------------
void Snare_trigger(const uint8_t vel, const uint8_t note)
{
	lfo_retrigger(3);
	//update velocity modulation
#if OLD_LFO_ROUTING
	modNode_updateValue(&velocityModulators[3],vel/127.f);
#else
	snareVoice.velocity = vel/127.f;
#endif
	float offset = 1;
	if(snareVoice.transGen.waveform==1) //offset mode
	{
		offset -= snareVoice.transGen.volume;
	}
	if(snareVoice.osc.waveform == SINE)
		snareVoice.osc.phase = (0x3ff<<20)*offset;//voiceArray[voiceNr].osc.startPhase ;
	else if(snareVoice.osc.waveform > SINE && snareVoice.osc.waveform <= REC)
		snareVoice.osc.phase = (0xff<<22)*offset;
	else
		snareVoice.osc.phase = 0;

	DecayEg_trigger(&snareVoice.oscPitchEg);
	slopeEg2_trigger(&snareVoice.oscVolEg);
	snareVoice.velo = vel/127.f;

	osc_setBaseNote(&snareVoice.osc,note);
	//TODO noise muss mit transponiert werden

	transient_trigger(&snareVoice.transGen);

	SnapEg_trigger(&snareVoice.snapEg);

	Snare_updatePan();
}
//---------------------------------------------------
IN_ITCM_RAM void Snare_calcAsync()
{
	//add modulation eg to osc freq (1 = no change. a+eg = original freq + modulation
	const float egPitchVal = DecayEg_calc(&snareVoice.oscPitchEg);
	const float pitchEgValue = egPitchVal*snareVoice.egPitchModAmount;
	snareVoice.osc.pitchMod = 1+pitchEgValue;

	//calc the osc  vol eg
	snareVoice.egValueOscVol = slopeEg2_calc(&snareVoice.oscVolEg);

	//turn off trigger signal if trigger gate mode is on and volume == 0
	/*
	if(trigger_isGateModeOn())
	{
		if(!snareVoice.egValueOscVol) {
			//trigger_triggerVoice(TRIGGER_4, TRIGGER_OFF);
			//voiceControl_noteOff(TRIGGER_4);
		}
	}
	*/

	//calc snap EG if transient sample 0 is activated
	if(snareVoice.transGen.waveform == 0)
	{
		const float snapVal = SnapEg_calc(&snareVoice.snapEg, snareVoice.transGen.pitch);
		snareVoice.osc.pitchMod += snapVal*snareVoice.transGen.volume;;
	}

	osc_setFreq(&snareVoice.osc);
	osc_setFreq(&snareVoice.noiseOsc);


}
//---------------------------------------------------
void Snare_calcSyncBlock(int16_t* buf, const uint8_t size)
{
	int16_t transBuf[size];
	int16_t oscBuf[size];

	calcNoiseBlock(&snareVoice.noiseOsc,buf,size,0.9f);
#if USE_REVERB
	if(fxBus.fxType != FX_REVERB)
	{
#endif
		SVF_calcBlockZDF(&snareVoice.filter,snareVoice.filterType,buf,size);
#if USE_REVERB
	}
	else
	{
		SVF_calcBlockCheapZDF(&snareVoice.filter,snareVoice.filterType,buf,size);
	}
#endif


	//calc next osc sample
	calcNextOscSampleBlock(&snareVoice.osc,oscBuf,size,(1.f-snareVoice.mix), snareVoice.pwm);
	// apply filter to synthesized sound as well here if desired, or combine code for more efficiency
	//SVF_calcBlockZDF(&snareVoice.filter,snareVoice.filterType,transBuf,size);

	//calc transient sample
	transient_calcBlock(&snareVoice.transGen,transBuf,size);


	uint8_t j;
	if(snareVoice.volumeMod)
	{
		snareVoice.combinedVolume = snareVoice.vol * snareVoice.velo * snareVoice.egValueOscVol ;
		//const float vol = ;

		for(j=0;j<size;j++)
		{

#if 1
			//float fmaf(float a, float b, float c);
			//result = a × b + c
			float tmp = fmaf(buf[j], snareVoice.mix, oscBuf[j]);
			//add transient sample
			//tmp	= __QADD16(transBuf[j],(int16_t)tmp);
			tmp	+= transBuf[j];
			buf[j] =   snareVoice.combinedVolume  * tmp ;//* calcOnePole( &snareVoice.FaderVolumeFilter, adc_getFaderVolume(3));

#else

			//add filter to buffer
			buf[j] *= snareVoice.mix;
			buf[j] = (__QADD16(buf[j],transBuf[j])); //transBuf here is mod osc

			buf[j] *=  vol;
#endif
		}
	}
	else
	{
		snareVoice.combinedVolume = snareVoice.vol * snareVoice.egValueOscVol;
		//const float vol = snareVoice.combinedVolume * ;

		for(j=0;j<size;j++)
		{

#if 1
			float tmp = fmaf(buf[j], snareVoice.mix, oscBuf[j]);
			//add transient sample
			//tmp	= __QADD16(transBuf[j],(int16_t)tmp);
			tmp	+= transBuf[j];
			buf[j] =  snareVoice.combinedVolume * tmp ;//* calcOnePole( &snareVoice.FaderVolumeFilter, adc_getFaderVolume(3));
#else
			//add filter to buffer
			buf[j] *= snareVoice.mix;
			buf[j] = (__QADD16(buf[j],transBuf[j]));
			buf[j] *=  vol;
#endif
		}
	}

	calcDistBlock(&snareVoice.distortion,buf,size);
}
//------------------------------------------------------------------------
