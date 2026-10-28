/*
 * CymbalVoice.c
 *
 *  Created on: 16.06.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/DSPAudio/CymbalVoice.h"
#include "Soundengine/DSPAudio/squareRootLut.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/config.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "IO/adcPots.h"
#include "Soundengine/FxBus.h"

INDTCMZ CymbalVoice cymbalVoice;
//---------------------------------------------------
void Cymbal_setPan(const uint8_t pan)
{
	//cymbalVoice.panL = squareRootLut[127-pan];
	//cymbalVoice.panR = squareRootLut[pan];
	cymbalVoice.pan = pan;
}
//---------------------------------------------------

void Cymbal_init()
{

	//initOnePole(&cymbalVoice.FaderVolumeFilter);
	//setOnePoleCoef(&cymbalVoice.FaderVolumeFilter,0.1);
	cymbalVoice.cvFreq = 0.5;
	SnapEg_init(&cymbalVoice.snapEg);
	Cymbal_setPan(0);
	cymbalVoice.vol = 0.8f;

	//cymbalVoice.panModifier = 1.f;

	transient_init(&cymbalVoice.transGen);

	cymbalVoice.fmModAmount1 = 0.5f;
	cymbalVoice.fmModAmount2 = 0.5f;

	setDistortionShape(&cymbalVoice.distortion, 0.f);

	cymbalVoice.modOsc.freq = 440;
	cymbalVoice.modOsc.waveform = SINE;
	cymbalVoice.modOsc.fmMod = 0;
	cymbalVoice.modOsc.midiFreq = 70<<8 | 64;
	cymbalVoice.modOsc.pitchMod = 1.0f;
	cymbalVoice.modOsc.modNodeValue = 1;
	cymbalVoice.modOsc.cvFreq = -1.f;

	cymbalVoice.modOsc2.freq = 440;
	cymbalVoice.modOsc2.waveform = NOISE;//SINE;
	cymbalVoice.modOsc2.fmMod = 0;
	cymbalVoice.modOsc2.midiFreq = 70<<8| 64;
	cymbalVoice.modOsc2.pitchMod = 1.0f;
	cymbalVoice.modOsc2.modNodeValue = 1;
	cymbalVoice.modOsc2.cvFreq = -1.f;

	cymbalVoice.osc.freq = 440;
	cymbalVoice.osc.waveform = 1;
	cymbalVoice.osc.fmMod = 1;
	cymbalVoice.osc.midiFreq = 70<<8 | 64;
	cymbalVoice.osc.pitchMod = 1.0f;
	cymbalVoice.osc.modNodeValue = 1;
	cymbalVoice.osc.cvFreq = -1.f;

	cymbalVoice.volumeMod = 1;

	slopeEg2_init(&cymbalVoice.oscVolEg);

	SVF_init(&cymbalVoice.filter);

	lfo_init(&cymbalVoice.lfo);

}

//---------------------------------------------------
void Cymbal_updatePan()
{
	cymbalVoice.triggeredPanL = sqrtf((127-cymbalVoice.pan)/127.f);
	cymbalVoice.triggeredPanR = sqrtf(cymbalVoice.pan/127.f);
}
//---------------------------------------------------
void Cymbal_setCv1VOct(float oct)
{
	cymbalVoice.osc.cvFreq		= oct;		// the main fm oscillator
	cymbalVoice.modOsc.cvFreq	= oct;		// the mod osc
	cymbalVoice.modOsc2.cvFreq	= oct;		// the mod osc
	cymbalVoice.transGen.cvFreq = oct;
}
//---------------------------------------------------
void Cymbal_trigger( const uint8_t vel, const uint8_t note)
{
	lfo_retrigger(4);
	//update velocity modulation
#if OLD_LFO_ROUTING
	modNode_updateValue(&velocityModulators[4],vel/127.f);
#else
	cymbalVoice.velocity = vel/127.f;
#endif
	float offset = 1;
	if(cymbalVoice.transGen.waveform==1) //offset mode
	{
		offset -= cymbalVoice.transGen.volume;
	}
	if(cymbalVoice.osc.waveform == SINE)
		cymbalVoice.osc.phase = (0x3ff<<20)*offset;//voiceArray[voiceNr].osc.startPhase ;
	else if(cymbalVoice.osc.waveform > SINE && cymbalVoice.osc.waveform <= REC)
		cymbalVoice.osc.phase = (0xff<<22)*offset;
	else
		cymbalVoice.osc.phase = 0;

	cymbalVoice.modOsc.phase = 0;
	cymbalVoice.modOsc2.phase = 0;

	osc_setBaseNote(&cymbalVoice.osc,note);
	osc_setBaseNote(&cymbalVoice.modOsc,note);
	osc_setBaseNote(&cymbalVoice.modOsc2,note);

	slopeEg2_trigger(&cymbalVoice.oscVolEg);
	cymbalVoice.velo = vel/127.f;

	transient_trigger(&cymbalVoice.transGen);

	SnapEg_trigger(&cymbalVoice.snapEg);

	Cymbal_updatePan();
}
//---------------------------------------------------
IN_ITCM_RAM void Cymbal_calcAsync()
{
	//calc the osc  vol eg
	cymbalVoice.egValueOscVol = slopeEg2_calc(&cymbalVoice.oscVolEg);

	//turn off trigger signal if trigger gate mode is on and volume == 0
	/*
	if(trigger_isGateModeOn())
	{
		if(!cymbalVoice.egValueOscVol) {
			trigger_triggerVoice(TRIGGER_5, TRIGGER_OFF);
			voiceControl_noteOff(TRIGGER_5);
		}
	}
	*/

	//calc snap EG if transient sample 0 is activated
	if(cymbalVoice.transGen.waveform == 0)
	{
		const float snapVal = SnapEg_calc(&cymbalVoice.snapEg,cymbalVoice.transGen.pitch);
		cymbalVoice.osc.pitchMod = 1 + snapVal*cymbalVoice.transGen.volume;
	}

	//update osc phaseInc
	osc_setFreq(&cymbalVoice.osc);
	osc_setFreq(&cymbalVoice.modOsc);
	osc_setFreq(&cymbalVoice.modOsc2);


}
//---------------------------------------------------
void Cymbal_calcSyncBlock(int16_t* buf, const uint8_t size)
{
		int16_t mod[size];
		int16_t mod2[size];
		//calc next mod osc sample
		calcNextOscSampleBlock(&cymbalVoice.modOsc,mod,size,cymbalVoice.fmModAmount1, cymbalVoice.pwm);
		calcNextOscSampleBlock(&cymbalVoice.modOsc2,mod2,size,cymbalVoice.fmModAmount2, cymbalVoice.pwm);

		//combine both mod oscs to 1 modulation signal
		bufferTool_addBuffersSaturating(mod,mod2,size);

		calcNextOscSampleFmBlock(&cymbalVoice.osc,mod,buf,size,1.f, cymbalVoice.pwm) ;
#if USE_REVERB
		if(fxBus.fxType != FX_REVERB)
		{
#endif
			SVF_calcBlockZDF(&cymbalVoice.filter,cymbalVoice.filterType,buf,size);

#if USE_REVERB
		}
		else
		{
			SVF_calcBlockCheapZDF(&cymbalVoice.filter,cymbalVoice.filterType,buf,size);
		}
#endif

		//calc transient sample
		transient_calcBlock(&cymbalVoice.transGen,mod,size);

		uint8_t j;
		if(cymbalVoice.volumeMod)
		{
			cymbalVoice.combinedVolume = cymbalVoice.vol *cymbalVoice.velo *  cymbalVoice.egValueOscVol ;
			//const float vol = ;

#if 1
			int32_t* x = (int32_t*)buf;
			int32_t* y = (int32_t*)mod;

			//add filter to buffer
			for(uint8_t i=0;i<size/2;i++)
			{
				x[i] = __QADD16(x[i],y[i]);
			}

			for(j=0;j<size;j++)
			{
				//add volume
				buf[j] *=  cymbalVoice.combinedVolume ;//* calcOnePole( &cymbalVoice.FaderVolumeFilter, adc_getFaderVolume(4));
			}
#else
			for(j=0;j<size;j++)
			{
				//add filter to buffer
				buf[j] = (__QADD16(buf[j],mod[j])) ;
				buf[j] *=  vol;
			}
#endif
		}
		else
		{
			cymbalVoice.combinedVolume = cymbalVoice.vol * cymbalVoice.egValueOscVol;
			//const float vol = cymbalVoice.combinedVolume ;

#if 1
			int32_t* x = (int32_t*)buf;
			int32_t* y = (int32_t*)mod;

			//add filter to buffer
			for(uint8_t i=0;i<size/2;i++)
			{
				x[i] = __QADD16(x[i],y[i]);
			}

			for(j=0;j<size;j++)
			{
				//add volume
				buf[j] *=  cymbalVoice.combinedVolume ;//* calcOnePole( &cymbalVoice.FaderVolumeFilter, adc_getFaderVolume(4));
			}
#else
			for(j=0;j<size;j++)
			{
				//add filter to buffer
				buf[j] = (__QADD16(buf[j],mod[j])) ;
				buf[j] *=  vol;
			}
#endif
		}
		calcDistBlock(&cymbalVoice.distortion,buf,size);
}
//---------------------------------------------------

