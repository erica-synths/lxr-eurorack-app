/*
 * DrumVoice.c
 *
 *  Created on: 03.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/DrumVoice.h"
#include "Soundengine/DSPAudio/Oscillator.h"
#include "Soundengine/DSPAudio/random.h"
#include "math.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "Soundengine/DSPAudio/BufferTools.h"
#include "Soundengine/MIDI/ParameterArray.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "IO/adcPots.h"

INDTCM static float ampSmoothValue = 0.1f;
//---------------------------------------------------
INDTCMZ DrumVoice voiceArray[NUM_DRUM_VOICES];
//---------------------------------------------------
IN_ITCM_RAM void setPan(const uint8_t voiceNr, const uint8_t pan)
{
	voiceArray[voiceNr].pan = pan;
}
//---------------------------------------------------
void drum_setPhase(const uint8_t phase, const uint8_t voiceNr)
{
	const uint32_t startPhase = (phase/127.f)*0xffffffff;
	voiceArray[voiceNr].osc.startPhase = startPhase;
	voiceArray[voiceNr].modOsc.startPhase = startPhase;
}
//---------------------------------------------------
void initDrumVoice()
{
	ampSmoothValue = 0.1f;




	int i;
	for(i=0;i<NUM_DRUM_VOICES;i++)
	{
		voiceArray[i].cvFreq = -1;
	//	initOnePole(&voiceArray[i].FaderVolumeFilter);
	//	setOnePoleCoef(&voiceArray[i].FaderVolumeFilter,0.01);

#if SMOOTH_MIXER_VOL_PAN_DRIVE
		for(uint8_t j=0;j<2;j++)
		{
			initOnePole(&voiceArray[i].MixerPageFilter[j]);
			setOnePoleCoef(&voiceArray[i].MixerPageFilter[j],0.01);
		}
		initOnePole(&voiceArray[i].distortion.DriveParamFilter);
		setOnePoleCoef(&voiceArray[i].distortion.DriveParamFilter,0.01);

#endif

		SnapEg_init(&voiceArray[i].snapEg);
		setPan(i,0.f);
		voiceArray[i].vol = 0.8f;
		//voiceArray[i].panModifier = 1.f;
		voiceArray[i].fmModAmount = 0.5f;
		transient_init(&voiceArray[i].transGen);
#if ENABLE_DRUM_SVF
		SVF_init(&voiceArray[i].filter);
		voiceArray[i].filterType = 0x01;
#endif
		lfo_init(&voiceArray[i].lfo);

		voiceArray[i].modOsc.freq = 440;
		voiceArray[i].modOsc.waveform = 1;
		voiceArray[i].modOsc.fmMod = 0;
		voiceArray[i].modOsc.midiFreq = 70<<8 | 64;
		voiceArray[i].modOsc.pitchMod = 1.0f;
		voiceArray[i].modOsc.modNodeValue = 1;
		voiceArray[i].modOsc.cvFreq = -1.f;

		voiceArray[i].volumeMod = 1;

		voiceArray[i].osc.freq = 440;
		voiceArray[i].osc.modNodeValue = 1;
		voiceArray[i].osc.waveform = TRI+i; //for testing init to tri,saw,rec
		voiceArray[i].osc.fmMod = 0;
		voiceArray[i].osc.midiFreq = 70<<8 | 64;
		voiceArray[i].osc.cvFreq = -1.f;

		DecayEg_init(&voiceArray[i].oscPitchEg);
		voiceArray[i].egPitchModAmount = 0.5f;

		slopeEg2_init(&voiceArray[i].oscVolEg);
		setDistortionShape(&voiceArray[i].distortion, 0.f);

#ifdef USE_AMP_FILTER
		initOnePole(&voiceArray[i].ampFilter);
		setOnePoleCoef(&voiceArray[i].ampFilter,ampSmoothValue);
#endif

#if ENABLE_MIX_OSC
		voiceArray[i].mixOscs = true;
#endif
		voiceArray[i].decimationCnt = 0;
		voiceArray[i].decimationRate = 1;

	}
}
//---------------------------------------------------
void Drum_updatePan(const uint8_t voiceNr)
{
	voiceArray[voiceNr].triggeredPanL = sqrtf((127-voiceArray[voiceNr].pan)/127.f);
	voiceArray[voiceNr].triggeredPanR = sqrtf(voiceArray[voiceNr].pan/127.f);
}
//---------------------------------------------------
void Drum_setCv1VOct(const uint8_t voiceNr, float oct)
{
	voiceArray[voiceNr].osc.cvFreq		= oct;		// the main fm oscillator
	voiceArray[voiceNr].modOsc.cvFreq	= oct;		// the mod osc
	voiceArray[voiceNr].transGen.cvFreq = oct;
}
//---------------------------------------------------
void Drum_trigger(const uint8_t voiceNr, const uint8_t vol, const uint8_t note)
{

	lfo_retrigger(voiceNr);

	//update velocity modulation
#if OLD_LFO_ROUTING
	modNode_updateValue(&velocityModulators[voiceNr],vol/127.f);
#else
		voiceArray[voiceNr].velocity = vol/127.f;
#endif

	//only reset phase if envelope is closed
#ifdef USE_AMP_FILTER
	if((voiceArray[voiceNr].volEgValueBlock[15]<=0.01f) || (voiceArray[voiceNr].transGen.waveform==1))
#else
		//if((voiceArray[voiceNr].ampFilterInput<=0.01f) || (voiceArray[voiceNr].transGen.waveform==1))
#endif
	{
		float offset = 1;
		if(voiceArray[voiceNr].transGen.waveform==1) //offset mode
		{
			offset -= voiceArray[voiceNr].transGen.volume;
#ifdef USE_AMP_FILTER
			setOnePoleCoef(&voiceArray[voiceNr].ampFilter,1.0f); //turn off amp filter for super snappy attack

		} else {
			setOnePoleCoef(&voiceArray[voiceNr].ampFilter,ampSmoothValue);
#endif
		}

		if(voiceArray[voiceNr].osc.waveform == SINE)
			voiceArray[voiceNr].osc.phase = 1024 + ( (0x3ff<<20) - 1024)*offset;//voiceArray[voiceNr].osc.startPhase ;
		else if(voiceArray[voiceNr].osc.waveform > SINE && voiceArray[voiceNr].osc.waveform <= REC)
			voiceArray[voiceNr].osc.phase = (0xff<<22)*offset;
		else
			voiceArray[voiceNr].osc.phase = 0;


		//reset mod osc phase
		if(voiceArray[voiceNr].modOsc.waveform == SINE)
			voiceArray[voiceNr].modOsc.phase = 1024 + ( (0x3ff<<20) - 1024);//voiceArray[voiceNr].osc.startPhase ;
		else if(voiceArray[voiceNr].modOsc.waveform > SINE && voiceArray[voiceNr].modOsc.waveform <= REC)
			voiceArray[voiceNr].modOsc.phase = (0xff<<22);
		else
			voiceArray[voiceNr].modOsc.phase = 0;

	}

	osc_setBaseNote(&voiceArray[voiceNr].osc,note);
	osc_setBaseNote(&voiceArray[voiceNr].modOsc,note);


	DecayEg_trigger(&voiceArray[voiceNr].oscPitchEg);
	slopeEg2_trigger(&voiceArray[voiceNr].oscVolEg);
	voiceArray[voiceNr].velo = vol/127.f;

	transient_trigger(&voiceArray[voiceNr].transGen);

	SnapEg_trigger(&voiceArray[voiceNr].snapEg);

	//reset filter coeffs to prevent wrong transient
	SVF_reset(&voiceArray[voiceNr].filter);

	//update pan value
	Drum_updatePan(voiceNr);
}
//---------------------------------------------------
IN_ITCM_RAM void calcDrumVoiceAsync(const uint8_t voiceNr)
{


	//add modulation eg to osc freq (1 = no change. a+eg = original freq + modulation
	const float egPitchVal = DecayEg_calc(&voiceArray[voiceNr].oscPitchEg);
	const float pitchEgValue = egPitchVal*voiceArray[voiceNr].egPitchModAmount;
	voiceArray[voiceNr].osc.pitchMod = 1+pitchEgValue;

	//calc snap EG if transient sample 0 is activated
	if(voiceArray[voiceNr].transGen.waveform == 0)
	{
		const float snapVal = SnapEg_calc(&voiceArray[voiceNr].snapEg, voiceArray[voiceNr].transGen.pitch);
		voiceArray[voiceNr].osc.pitchMod += snapVal*voiceArray[voiceNr].transGen.volume;
	}

	// fm amount with pitch eg
	voiceArray[voiceNr].osc.fmMod = voiceArray[voiceNr].fmModAmount * egPitchVal;

	//calc the osc + noise vol eg
#if (AMP_EG_SYNC==0)

	//check if in attack phase
	if( (voiceArray[voiceNr].oscVolEg.attack == 1 ) && ((voiceArray[voiceNr].oscVolEg.state == EG_A) || (voiceArray[voiceNr].oscVolEg.state == EG_REPEAT)) )
	{
			//if attack is set to 0 -> no interpolation
		voiceArray[voiceNr].ampFilterInput = slopeEg2_calc(&voiceArray[voiceNr].oscVolEg);
		voiceArray[voiceNr].lastGain = voiceArray[voiceNr].ampFilterInput;
	}
	else
	{
		voiceArray[voiceNr].lastGain = voiceArray[voiceNr].ampFilterInput;
		voiceArray[voiceNr].ampFilterInput = slopeEg2_calc(&voiceArray[voiceNr].oscVolEg);
	}

	//turn off trigger signal if trigger gate mode is on and volume == 0
	/*
	if(trigger_isGateModeOn())
	{
		if(!voiceArray[voiceNr].ampFilterInput) {
			//trigger_triggerVoice(TRIGGER_1 + voiceNr, TRIGGER_OFF);
			voiceControl_noteOff(TRIGGER_1 + voiceNr);
		}
	}
	*/
#endif

	//update osc phaseInc
	osc_setFreq(&voiceArray[voiceNr].osc);
	osc_setFreq(&voiceArray[voiceNr].modOsc);



}

//---------------------------------------------------
IN_ITCM_RAM void calcDrumVoiceSyncBlock(const uint8_t voiceNr, int16_t* buf, const uint8_t size)
{


	int16_t modBuf[size];

	//calc vol EG
#ifdef USE_AMP_FILTER
	calcOnePoleBlockFixedInput(&voiceArray[voiceNr].ampFilter, voiceArray[voiceNr].ampFilterInput,voiceArray[voiceNr].volEgValueBlock, size);
#endif

	//calc next mod osc sampleBlock - int16_t processing
	calcNextOscSampleBlock(&voiceArray[voiceNr].modOsc,modBuf,size,voiceArray[voiceNr].fmModAmount, voiceArray[voiceNr].pwm);

	if(voiceArray[voiceNr].mixOscs)
	{
		//calc main osc buffer - int16_t processing
		calcNextOscSampleBlock(&voiceArray[voiceNr].osc,buf,size, (1.f-voiceArray[voiceNr].fmModAmount), voiceArray[voiceNr].pwm);
		//add mod buffer to main osc buffer - int16_t processing
		bufferTool_addBuffersSaturating(buf,modBuf,size);
	}
	else
	{
		calcNextOscSampleFmBlock(&voiceArray[voiceNr].osc,modBuf,buf,size,1.0f, voiceArray[voiceNr].pwm);
	}

	//calc transient sample - int16_t processing
	transient_calcBlock(&voiceArray[voiceNr].transGen,modBuf,size);

	//Mix with transient buffer - int16_t processing
	bufferTool_addBuffersSaturating(buf,modBuf,size);

	//calc filter block - float processing
	SVF_calcBlockZDF(&voiceArray[voiceNr].filter,voiceArray[voiceNr].filterType,buf,size);

	//attentuate main OSCs by amp EG
#ifdef USE_AMP_FILTER
	bufferTool_multiplyWithFloatBufferDithered(&voiceArray[voiceNr].dither, buf,voiceArray[voiceNr].volEgValueBlock,size);
#else
	//bufferTool_addGainInterpolatedDoubleDithered(&voiceArray[voiceNr].dither, buf,voiceArray[voiceNr].ampFilterInput, voiceArray[voiceNr].lastGain, size);
	//float
	bufferTool_addGainInterpolated(buf,voiceArray[voiceNr].ampFilterInput, voiceArray[voiceNr].lastGain, size);

	//buf[i] = dither_process(voiceArray[voiceNr].dither, buf[i]
#endif

	//MIDI velocity - float processing
	if(voiceArray[voiceNr].volumeMod)
	{
		bufferTool_addGain(buf,voiceArray[voiceNr].velo,size);
	}
	//distortion - float processing
#if (USE_FILTER_DRIVE == 0)
#if SMOOTH_MIXER_VOL_PAN_DRIVE
	calcDistBlockFiltered(&voiceArray[voiceNr].distortion,buf,size);
#else
	calcDistBlock(&voiceArray[voiceNr].distortion,buf,size);
#endif
#endif
	//channel volume
	//bufferTool_addGain(buf,voiceArray[voiceNr].combinedVolume  ,size);

	for(int i=0; i< size; i++)
	{
#if SMOOTH_MIXER_VOL_PAN_DRIVE

		float mixVol 	= calcOnePole( &voiceArray[voiceNr].MixerPageFilter[0], voiceArray[voiceNr].vol);
		voiceArray[voiceNr].combinedVolume = mixVol ;//* calcOnePole( &voiceArray[voiceNr].FaderVolumeFilter, adc_getFaderVolume(voiceNr));
#else
		voiceArray[voiceNr].combinedVolume = voiceArray[voiceNr].vol * calcOnePole( &voiceArray[voiceNr].FaderVolumeFilter, adc_getFaderVolume(voiceNr));
#endif
		buf[i] *= voiceArray[voiceNr].combinedVolume;
	}
}
//---------------------------------------------------


