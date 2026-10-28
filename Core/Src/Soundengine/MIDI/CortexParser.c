/*
 * frontPanelParser.c
 *
 *  Created on: 27.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "menu/menu.h"
#include <Soundengine/MIDI/CortexParser.h>
#include <Soundengine/TriggerProcessor.h>
#include "Soundengine/MIDI/MidiMessages.h"
#include "Soundengine/MIDI/MidiParser.h"
//#include "Soundengine/Sequencer/sequencer.h"
#include "Soundengine/MIDI/Uart.h"
//#include "SD_Manager.h"
//#include "Soundengine/Sequencer/EuklidGenerator.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/mixer.h"

#include "Soundengine/DSPAudio/DrumVoice.h"
#include "Soundengine/DSPAudio/CymbalVoice.h"
#include "Soundengine/DSPAudio/HiHat.h"
#include "Soundengine/DSPAudio/Snare.h"
//#include "SomGenerator.h"
#include "ledHandler.h"

void frontParser_handleMidiMessage( MidiMsg msg);
//static void frontParser_handleSysexData(unsigned char data);
//static void frontParser_handleSeqCC();

//a counter for the received bytes
//each message is made up from 3 bytes (status 0xb0, parameter nr and parameter value)
 uint8_t frontParser_rxCnt=0;

//the currently selected parameter
 //MidiMsg cortex_midiMsg;

//the currently active track on the fron panel (shown on the seq led buttons)
 uint8_t frontParser_activeFrontTrack=0;

//used to store the sysex mode and to block all other uart messages while sysexActive != 0
 uint8_t frontParser_sysexActive=0;
/** used to collect 2 7 bit messages and combine them to a 14 bit message*/
 uint16_t frontParser_twoByteData=0;

 //uint8_t frontParser_sysexBuffer[7];

/** used to count incoming sequencer step data packages*/
 uint16_t frontParser_sysexSeqStepNr=0;

 uint8_t frontParser_activeTrack=0;	/** the active track on the Frontpanel. track specific messages refer to the track selected with this command*/
uint8_t frontParser_shownPattern = 0;
 uint8_t frontParser_activeStep=0;

//------------------------------------------------------
/**send all active step numbers to frontpanel to light up corresponding LEDs*/
 /*
void frontParser_updateTrackLeds(const uint8_t trackNr, uint8_t patternNr)
{
	if(trackNr<=6)
	{

		frontParser_activeFrontTrack = trackNr;

		int i=0;

		for(;i<16;i++)
		{
			if(seq_isMainStepActive(trackNr,i,patternNr))
			{
				uart_sendFrontpanelByte(FRONT_STEP_LED_STATUS_BYTE);
				uart_sendFrontpanelByte(FRONT_LED_SEQ_BUTTON);
				uart_sendFrontpanelByte(i*8);
			}
		}

		uint8_t start = (frontParser_activeStep/8)*8;
		for(i=start;i<(start+8);i++) //only send visible substeps
		{
			if(seq_isStepActive(trackNr,i,patternNr))
			{
				uart_sendFrontpanelByte(FRONT_STEP_LED_STATUS_BYTE);
				uart_sendFrontpanelByte(FRONT_LED_SEQ_SUB_STEP);
				uart_sendFrontpanelByte(i);
			}
		}
	}
}
*/
//------------------------------------------------------
#if 0
void frontParser_parseUartData(unsigned char data)
{

	//TODO der ganze sysex kram kann sicher noch optimiert werden
	//das das nicht andauernd abgefragt werden muss

	// if high bit is set, a new message starts
	if(data&0x80)
	{
		//reset the byte counter
		frontParser_rxCnt = 0;
		cortex_midiMsg.status = data;
		if(data==SYSEX_START)
		{
			frontParser_sysexActive = SYSEX_ACTIVE_MODE_NONE;
			uart_clearFrontFifo();

			//send SYSEX_START as ACK
			uart_sendFrontpanelSysExByte(SYSEX_START);
		}
		else
		{
			frontParser_sysexActive = SYSEX_INACTIVE;
		}
	}
	else // data byte
	{
		if(cortex_midiMsg.status == SYSEX_START)
		{
			frontParser_handleSysexData(data);
		}
		else if(frontParser_rxCnt==0) //normal operation
		{
			//parameter nr
			cortex_midiMsg.data1 = data;
			frontParser_rxCnt++;
		}
		else
		{
			//parameter value
			cortex_midiMsg.data2 = data;
			//message received. process the midi message
			frontParser_handleMidiMessage();
		}
	}
};
#endif

//------------------------------------------------------
// This is called when we are in sysex mode and are receiving the sysex bytes
#if 0
static void frontParser_handleSysexData(unsigned char data)
{
	//SYSEX
	// this is not a correct sysex implementation.
	// in the front panel communication it is used to send sequencer data for preset saving
	// while the preset saving is active, no other data has to be send over the uart!!!
	// so when the SYSEX_START is received, all other communication is stopped until theSYSEX_END message is received
	// no abort and no manufacturer id is supported

	//we have received sysex data
	//first we receive a mode message
	//SYSEX_REQUEST_STEP_DATA or SYSEX_SEND_STEP_DATA
	//then the corresponding data

	switch(frontParser_sysexActive) {
	/*
	case SYSEX_REQUEST_PATTERN_DATA:
		//1 byte = pattern nr
		//send back next and repeat
		uart_sendFrontpanelSysExByte(seq_patternSet.seq_patternSettings[data].nextPattern);
		uart_sendFrontpanelSysExByte(seq_patternSet.seq_patternSettings[data].changeBar);
		break;
		*/
		#if 0
	case SYSEX_REQUEST_MAIN_STEP_DATA:
		//we expect a 2 byte message containing a step nr
		//which tells us which main step data to send back
		frontParser_rxCnt++;
		if(frontParser_rxCnt == 1)
		{
			//first nibble received -> upper nibble
			frontParser_twoByteData = data<<7;
		}
		else
		{
			//second nibble -> lower nibble
			frontParser_twoByteData |= data&0x7f;
			//reset rxCount for next 2 nibble message
			frontParser_rxCnt = 0;
			//we have received a complete 2 nibble step nr
			//send data back to front
			seq_sendMainStepInfoToFront(frontParser_twoByteData);
		}
		break;
#endif

#if !EURORACK_MODULE
	case SYSEX_REQUEST_STEP_DATA:
		//we expect a 2 byte message containing a step nr
		//which tells us which step data to send back
		frontParser_rxCnt++;

		if(frontParser_rxCnt == 1)
		{
			//first nibble received -> upper nibble
			frontParser_twoByteData = data<<7;
		}
		else
		{
			//second nibble -> lower nibble
			frontParser_twoByteData |= data&0x7f;
			//reset rxCount for next 2 nibble message
			frontParser_rxCnt = 0;
			//we have received a complete 2 nibble step nr
			//send data back to front
			seq_sendStepInfoToFront(frontParser_twoByteData);
		}

		break;
#endif
#if 0
	case SYSEX_RECEIVE_MAIN_STEP_DATA:
		if(frontParser_rxCnt<2)
		{
			frontParser_sysexBuffer[frontParser_rxCnt++] = data;
		}
		else
		{
			frontParser_sysexBuffer[frontParser_rxCnt++] = data;

			//calculate the step pattern and track indices
			const uint8_t currentPattern	= frontParser_sysexSeqStepNr / 7;
			const uint8_t currentTrack  	= frontParser_sysexSeqStepNr - currentPattern*7;

			uint16_t mainStepData = frontParser_sysexBuffer[0] | frontParser_sysexBuffer[1]<<7 | frontParser_sysexBuffer[2]<<14;

			//first load into inactive track
			PatternSet* patternSet = &seq_patternSet;

			if( (currentPattern == seq_activePattern) && seq_isRunning() )
			{
				seq_tmpPattern.seq_mainSteps[currentTrack] = mainStepData;
			} else {
				patternSet->seq_mainSteps[currentPattern][currentTrack] = mainStepData;
			}


			//inc the step counter
			frontParser_sysexSeqStepNr++;
			frontParser_rxCnt = 0;

			//signal new pattern
			// we do this below after receiving pattern length data
			//if( seq_isRunning()) {
			//	seq_newPatternAvailable = 1;
			//}
		}
		break;
#endif
#if !EURORACK_MODULE
	case SYSEX_RECEIVE_PAT_LEN_DATA:
		// same as above but we are receiving length data for each pattern
		{
			//calculate the step pattern and track indices
			const uint8_t currentPattern	= frontParser_sysexSeqStepNr / 7;
			const uint8_t currentTrack  	= frontParser_sysexSeqStepNr - currentPattern*7;


/*
			if( (currentPattern == seq_activePattern) && seq_isRunning() )
			{
				seq_tmpPattern.length[currentTrack] = data;
			} else {
				*/

#if USE_PATTERN_EDIT_BUFFER
			if( (currentPattern == seq_activePattern) )
			{
				patternBackupBuffer.length[currentTrack]=data;
			}
			else
			{
				seq_patterns.length[currentPattern][currentTrack] = data;
			}
#else
				//first load into inactive track
				PatternSet* patternSet = &seq_patternSet;
				patternSet->length[currentPattern][currentTrack] = data;
#endif
	//		}


			//inc the step counter
			frontParser_sysexSeqStepNr++;
			frontParser_rxCnt = 0;

			// signal new pattern after receiving all the data
			/*
			if( seq_isRunning() && (frontParser_sysexSeqStepNr == NUM_TRACKS*NUM_PATTERN)) {
				seq_newPatternAvailable = 1;
			}
			*/
		}
		break;


	case SYSEX_RECEIVE_STEP_DATA:
		// we expect a bunch of 8 byte sysex message containing new step data for the sequencer
		// beginning with step 0 up to NUMBER_STEPS*NUM_TRACKS*NUM_PATTERN = 128*7*8 = 7168 steps
		if(frontParser_rxCnt<7)
		{
			frontParser_sysexBuffer[frontParser_rxCnt++] = data;
		}
		else
		{
			//now we have to distribute the MSBs to the sysex data
			uint8_t i;
			for(i=0;i<7;i++)
			{
				frontParser_sysexBuffer[i] |= ((data&(1<<i))<<(7-i));

			}

			//calculate the step pattern and track indices
			const uint8_t absPat 	= frontParser_sysexSeqStepNr/128;
			const uint8_t currentTrack 		= absPat / 8;
			const uint8_t currentPattern 	= absPat - currentTrack*8;
			const uint8_t currentStep		= frontParser_sysexSeqStepNr - absPat*128;

#if USE_PATTERN_EDIT_BUFFER
			if( (currentPattern == seq_activePattern) )
			{


				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].volume 	= frontParser_sysexBuffer[0];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].prob 	= frontParser_sysexBuffer[1];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].note 	= frontParser_sysexBuffer[2];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].param1Nr = frontParser_sysexBuffer[3];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].param1Val 	= frontParser_sysexBuffer[4];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].param2Nr 	= frontParser_sysexBuffer[5];
				patternBackupBuffer.seq_subStepPattern[currentTrack][currentStep].param2Val 	= frontParser_sysexBuffer[6];
			}
			else
			{
				PatternSet* patternSet = &seq_patterns;
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].volume 	= frontParser_sysexBuffer[0];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].prob 	= frontParser_sysexBuffer[1];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].note 	= frontParser_sysexBuffer[2];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param1Nr = frontParser_sysexBuffer[3];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param1Val 	= frontParser_sysexBuffer[4];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param2Nr 	= frontParser_sysexBuffer[5];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param2Val 	= frontParser_sysexBuffer[6];
			}
#else

			PatternSet* patternSet = &seq_patternSet;

			//do not overwrite playing pattern
			/*
			if( (currentPattern == seq_activePattern)   && seq_isRunning())
			{
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].volume 	= frontParser_sysexBuffer[0];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].prob 	= frontParser_sysexBuffer[1];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].note 	= frontParser_sysexBuffer[2];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].param1Nr = frontParser_sysexBuffer[3];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].param1Val 	= frontParser_sysexBuffer[4];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].param2Nr 	= frontParser_sysexBuffer[5];
				seq_tmpPattern.seq_subStepPattern[currentTrack][currentStep].param2Val 	= frontParser_sysexBuffer[6];
			} else {
*/
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].volume 	= frontParser_sysexBuffer[0];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].prob 	= frontParser_sysexBuffer[1];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].note 	= frontParser_sysexBuffer[2];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param1Nr = frontParser_sysexBuffer[3];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param1Val 	= frontParser_sysexBuffer[4];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param2Nr 	= frontParser_sysexBuffer[5];
				patternSet->seq_subStepPattern[currentPattern][currentTrack][currentStep].param2Val 	= frontParser_sysexBuffer[6];
			//}
#endif
			//signal that a new data chunk is available
			//frontParser_newSeqDataAvailable = 1;
			//reset receive counter for next chunk
			frontParser_rxCnt = 0;

			//inc the step counter
			frontParser_sysexSeqStepNr++;
		}
		break;
#endif
	default:
	case SYSEX_ACTIVE_MODE_NONE:
		//we received a mode message -> set the active mode
		frontParser_sysexActive = data;
		frontParser_sysexSeqStepNr = 0;
		frontParser_rxCnt = 0;
		break;
	}
}
#endif
//------------------------------------------------------
// This is called when we've received a full midi message
void frontParser_handleMidiMessage( MidiMsg msg)
{

	switch(msg.status)
	{
	//SEQ MESSAGES
	case FRONT_SEQ_CC: // cortex_midiMsg.status
		//frontParser_handleSeqCC();
		break;

	case SAMPLE_CC:
		switch(msg.data1)
		{

		case FRONT_SAMPLE_START_UPLOAD:
			//seq_setRunning(0);
			//sampleMemory_init();
			//sampleMemory_loadSamples();
			//FLASH_Lock();

			//uart_sendFrontpanelByte(ACK);
		break;

		case FRONT_SAMPLE_COUNT:
			//uart_sendFrontpanelByte(SAMPLE_CC);
			//uart_sendFrontpanelByte(FRONT_SAMPLE_COUNT);
			//uart_sendFrontpanelByte(sampleMemory_getNumSamples());
			break;

		default:
			break;
		}
		break;

	//MIDI SYNTH MESSAGES
	case MIDI_CC: //cortex_midiMsg.status
		// this is for parameters below 128
		// fix offset between front an cortex
		// front params start at 1, cortex at 2 (because of midi in mod wheel==0x1
		// correct parameter number offset
		msg.data1 += 1;

		//because hh slope on front is 127 and on cortex is 0 wrap data1 at 127
		msg.data1 &= 0x7f;

		//only perform the parameter change if it is not a param lock
#if !EURORACK_MODULE
		if(!seq_isAutomationStepArmed())
#endif
		{
			midiParser_ccHandler(msg,1, 0, 0.f);
		}

		//record automation if record is turned on
#if !EURORACK_MODULE
		seq_recordAutomation(frontParser_activeTrack, cortex_midiMsg.data1, cortex_midiMsg.data2);
#endif
		break;

	//CC2 above 127
	case FRONT_CC_3:

	case FRONT_CC_2: // cortex_midiMsg.status
		{
			//only perform the parameter change if it is not a param lock
#if !EURORACK_MODULE
			if(!seq_isAutomationStepArmed())
#endif
			{
				midiParser_ccHandler(msg,1, 0, 0.f);
			}

			//record automation if record is turned on
#if !EURORACK_MODULE
			seq_recordAutomation(frontParser_activeTrack, cortex_midiMsg.data1+128, cortex_midiMsg.data2);
#endif
		}
		break;

#if OLD_LFO_ROUTING
	case FRONT_CC_LFO_TARGET: //cortex_midiMsg.status
	{
		uint8_t upper = msg.data1;
		uint8_t lower = msg.data2;
		// **PATROT note that the only valid values for the following are listed in
		// the modTargets array in the AVR code
		uint8_t value = ((upper&0x01)<<7) | lower;

		uint8_t lfoNr = (upper&0xfe)>>1;

		switch(lfoNr)
		{
		case 0:
		case 1:
		case 2:	modNode_setDestination(&voiceArray[lfoNr].lfo.modTarget, value);break;

		case 3:	modNode_setDestination(&snareVoice.lfo.modTarget,value);		break;
		case 4:	modNode_setDestination(&cymbalVoice.lfo.modTarget, value);		break;
		case 5:	modNode_setDestination(&hatVoice.lfo.modTarget, value);			break;
		default:
			break;
		}

	}
		break; // case FRONT_CC_LFO_TARGET
#endif
#if !EURORACK_MODULE
	case FRONT_SET_P1_DEST: { // cortex_midiMsg.status
		// **AUTOM add 1 to the value as our cortex parameters are off by 1 for the lower 127 params
			uint8_t hi = cortex_midiMsg.data1;
			uint8_t lo = cortex_midiMsg.data2;
			uint8_t val = (hi<<7)|lo;
			if(val && val < 128 )
				val++;


#if USE_PATTERN_EDIT_BUFFER
			patternBackupBuffer.seq_subStepPattern[frontParser_activeTrack][seq_selectedStep].param1Nr = val;

#else

			seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
			                                 [frontParser_activeTrack]
			                                 [seq_selectedStep].param1Nr = val;
#endif
		}
		break;
	case FRONT_SET_P2_DEST: { // cortex_midiMsg.status
		// **AUTOM same here
			uint8_t hi = cortex_midiMsg.data1;
			uint8_t lo = cortex_midiMsg.data2;
			uint8_t val = (hi<<7)|lo;
			if(val && val < 128 )
				val++;

#if USE_PATTERN_EDIT_BUFFER
			if( (currentPattern == seq_activePattern) )
			{
				patternBackupBuffer.seq_subStepPattern[frontParser_activeTrack][seq_selectedStep].param2Nr = val;
			}
			else
			{
				seq_patterns.seq_subStepPattern[frontParser_shownPattern]
				                                 [frontParser_activeTrack]
				                                 [seq_selectedStep].param2Nr = val;
			}

#else
			seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
			                                 [frontParser_activeTrack]
			                                 [seq_selectedStep].param2Nr = val;
#endif
		}
		break;
	case FRONT_SET_P1_VAL: { // cortex_midiMsg.status
			uint8_t stepNr = cortex_midiMsg.data1;
			uint8_t value = cortex_midiMsg.data2;

#if USE_PATTERN_EDIT_BUFFER
			if( (currentPattern == seq_activePattern) )
			{
				patternBackupBuffer.seq_subStepPattern[frontParser_activeTrack][stepNr].param1Val = value;
			}
			else
			{
				seq_patterns.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][stepNr].param1Val = value;
			}

#else
			seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][stepNr].param1Val = value;
#endif
		}
		break;
	case FRONT_SET_P2_VAL: { // cortex_midiMsg.status
			uint8_t stepNr = cortex_midiMsg.data1;
			uint8_t value = cortex_midiMsg.data2;

#if USE_PATTERN_EDIT_BUFFER
			if( (currentPattern == seq_activePattern) )
			{
				patternBackupBuffer.seq_subStepPattern[frontParser_activeTrack][stepNr].param2Val = value;
			}
			else
			{
				seq_patterns.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][stepNr].param2Val = value;
			}

#else
			seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][stepNr].param2Val = value;
#endif

		}
		break;

	case FRONT_ARM_AUTOMATION_STEP: // cortex_midiMsg.status
		{
			const uint8_t stepNr 	= cortex_midiMsg.data1;
			const uint8_t onOff 	= cortex_midiMsg.data2 & 0x40;
			const uint8_t trackNr 	=  cortex_midiMsg.data2 & 0x3f;

			seq_armAutomationStep(stepNr,trackNr,onOff);

		}
		break;
#endif
/*
	case FRONT_MAIN_STEP_CC: // cortex_midiMsg.status
		{
			//data 1 = track und pattern nr
			//data 2 = step nr
			uint8_t voiceNr 	= cortex_midiMsg.data1 >> 4;
			uint8_t patternNr 	= cortex_midiMsg.data1 & 0x7;
			uint8_t stepNr 		= cortex_midiMsg.data2;


			//toggle the step in the seq
			seq_toggleMainStep(voiceNr, stepNr, patternNr);

			//if step active send led on message to front
			if(seq_isMainStepActive(voiceNr, stepNr, patternNr))
			{
#if 0
				uart_sendFrontpanelByte(FRONT_STEP_LED_STATUS_BYTE);
				uart_sendFrontpanelByte(FRONT_LED_SEQ_BUTTON);
				uart_sendFrontpanelByte(stepNr*8);
#else
				led_setValue(1,led_idx2StepLed(stepNr));
#endif
			}
		}
		break;
		*/
/*
	case FRONT_STEP_CC: // cortex_midiMsg.status
		{
			//data 1 = track und pattern nr
			//data 2 = step nr
			uint8_t voiceNr 	= cortex_midiMsg.data1 >> 4;
			uint8_t patternNr 	= cortex_midiMsg.data1 & 0x7;
			uint8_t stepNr 		= cortex_midiMsg.data2;

			//toggle the step in the seq
			seq_toggleStep(voiceNr, stepNr, patternNr);

		}
		break;
		*/
#if OLD_LFO_ROUTING
	case FRONT_CC_VELO_TARGET: // cortex_midiMsg.status
		{
			uint8_t upper = msg.data1;
			uint8_t lower = msg.data2;
			// **PATROT note that the only valid values for the following are listed in
			// the modTargets array in the AVR code
			uint8_t value = ((upper&0x01)<<7) | lower;
			uint8_t velModNr = (upper&0xfe)>>1;
			modNode_setDestination(&velocityModulators[velModNr], value);
		}
		break;
#endif
	//VOICE option Messages
	case VOICE_CC: // cortex_midiMsg.status
		break;

	//BPM MESSAGE
#if !EURORACK_MODULE
	case FRONT_SET_BPM: { // cortex_midiMsg.status
			uint16_t bpm = cortex_midiMsg.data1 |(uint16_t)(cortex_midiMsg.data2<<7);
			if(bpm == 0) {
				seq_setExtSync(1);
			}
			else {
				seq_setBpm(bpm);

				seq_setExtSync(0);
			}
		}
		break;
#endif

	//LED MESSAGES
		/*
	case FRONT_STEP_LED_STATUS_BYTE: // cortex_midiMsg.status
		switch(cortex_midiMsg.data1)
		{
			//send all active step numbers to frontpanel to light up corresponding LEDs
			case FRONT_LED_QUERY_SEQ_TRACK:
			{
				uint8_t trackNr = cortex_midiMsg.data2 >> 4;
				uint8_t patternNr = cortex_midiMsg.data2 & 0x7;

				frontParser_updateTrackLeds(trackNr, patternNr);

				//send track length back
				uart_sendFrontpanelByte(FRONT_SEQ_CC);
				uart_sendFrontpanelByte(FRONT_SEQ_TRACK_LENGTH);
				uart_sendFrontpanelByte(seq_getTrackLength(trackNr));

				// **PATROT send track rotation value back
				uart_sendFrontpanelByte(FRONT_SEQ_CC);
				uart_sendFrontpanelByte(FRONT_SEQ_TRACK_ROTATION);
				uart_sendFrontpanelByte(seq_getTrackRotation(trackNr));

			}
			break;

		default:
			break;
		}
		break;
		*/
	} // cortex_midiMsg.status
}

//------------------------------------------------------
// Sequencer message handler
// This is called when we have received a message with status FRONT_SEQ_CC
#if !EURORACK_MODULE
static void frontParser_handleSeqCC()
{
	switch(cortex_midiMsg.data1)
	{
/*
	case FRONT_SEQ_REQUEST_PATTERN_PARAMS:
		// send back bar change and next pattern params from requested pattern
		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_SET_PAT_BEAT);
		uart_sendFrontpanelByte(seq_patternSet.seq_patternSettings[frontParser_shownPattern].changeBar);

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_SET_PAT_NEXT);
		uart_sendFrontpanelByte(seq_patternSet.seq_patternSettings[frontParser_shownPattern].nextPattern);


		break;
	case FRONT_SEQ_SET_PAT_BEAT:
		seq_patternSet.seq_patternSettings[frontParser_shownPattern].changeBar = cortex_midiMsg.data2;
		break;
	case FRONT_SEQ_SET_PAT_NEXT:
		seq_patternSet.seq_patternSettings[frontParser_shownPattern].nextPattern = cortex_midiMsg.data2;
		break;
*/

	case FRONT_SEQ_REC_ON_OFF:
		seq_setRecordingMode(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_ERASE_ON_OFF:
		seq_setErasingMode(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_NOTE:
		seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].note = cortex_midiMsg.data2;
		break;

	case FRONT_SEQ_VOLUME:
		seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].volume &= ~(0x7f);
		seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].volume |= (cortex_midiMsg.data2&0x7f);
		break;

	case FRONT_SEQ_FLAM:
		seq_patternSet.seq_stepTiming[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].flam = (cortex_midiMsg.data2);//seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].prob = (cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_SHIFT:
		seq_patternSet.seq_stepTiming[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].shift = (cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_PROB:

		seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][frontParser_activeStep].prob = (cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_EUKLID_LENGTH:
		{
			uint8_t length 	= cortex_midiMsg.data2 >> 3;
			length += 1;
			euklid_setLength(frontParser_activeTrack, length);
		}
		break;

	case FRONT_SEQ_EUKLID_STEPS:
		{
			uint8_t steps 	= cortex_midiMsg.data2 >> 3;
			steps += 1;
			uint8_t pattern = cortex_midiMsg.data2 & 0x7;

			euklid_setSteps(frontParser_activeTrack,steps,pattern);
			led_updateSeqLeds();
			//frontParser_updateTrackLeds(frontParser_activeTrack, pattern);
		}
		break;

	case FRONT_SEQ_EUKLID_ROTATION:
		{
			uint8_t rotation = cortex_midiMsg.data2 >> 3;
			//rotation += 1;
			uint8_t pattern = cortex_midiMsg.data2 & 0x7;

			euklid_setRotation(frontParser_activeTrack,rotation,pattern);
			//frontParser_updateTrackLeds(frontParser_activeTrack, pattern);
			led_updateSeqLeds();
		}
		break;

	case FRONT_SEQ_CLEAR_TRACK: {
			seq_clearTrack(cortex_midiMsg.data2, frontParser_shownPattern);
		}
		break;

/*
	case FRONT_SEQ_CLEAR_PATTERN:
		 seq_clearPattern(cortex_midiMsg.data2);
		break;
		*/

	case FRONT_SEQ_POSX:
		//som_setX(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_POSY:
		//som_setY(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_FLUX:
		//som_setFlux(cortex_midiMsg.data2/127.f);
		break;

	case FRONT_SEQ_SOM_FREQ:
		//som_setFreq(cortex_midiMsg.data2,frontParser_activeTrack);
		break;
#if !EURORACK_MODULE
	case FRONT_SEQ_MIDI_CHAN:
	{
		uint8_t voice = cortex_midiMsg.data2 >> 4;
		uint8_t channel = cortex_midiMsg.data2 & 0x0f;
		// if midi channel changed, and a note was playing on old channel, turn it off
		if(voice < 7 && midi_MidiChannels[voice] != channel)
			voiceControl_noteOff(voice);

		midi_MidiChannels[voice] = channel;
	}
		break;

	// not used anymore
	//case FRONT_SEQ_MIDI_MODE:
	//	midi_mode = cortex_midiMsg.data2;
	//	break;


	//voice nr (0xf0) + autom track nr (0x0f)
	case FRONT_SEQ_CLEAR_AUTOM:
	{
		const uint8_t voice 		= cortex_midiMsg.data2 >> 4;
		const uint8_t automTrack 	= cortex_midiMsg.data2 &  0x0f;
		seq_clearAutomation(voice, frontParser_shownPattern, automTrack);
	}
		break;

	case FRONT_SEQ_COPY_TRACK:
		{
			const uint8_t src = cortex_midiMsg.data2>>4;
			const uint8_t dst = cortex_midiMsg.data2&0xf;
			seq_copyTrack(src,dst,frontParser_shownPattern);
		}
		break;

	case FRONT_SEQ_COPY_PATTERN:
		{
			const uint8_t src = cortex_midiMsg.data2>>4;
			const uint8_t dst = cortex_midiMsg.data2&0xf;
			seq_copyPattern(src,dst);
		}
		break;

	case FRONT_SEQ_TRACK_LENGTH:
		seq_setTrackLength(frontParser_activeTrack,cortex_midiMsg.data2);
		break;
/*
	case FRONT_SEQ_TRACK_ROTATION: // **PATROT handle incoming track rotation. apply to active track
		seq_setTrackRotation(frontParser_activeTrack,cortex_midiMsg.data2);
		break;
		*/

	case FRONT_SEQ_SHUFFLE:
		seq_setShuffle(cortex_midiMsg.data2/127.f);
		break;

	case FRONT_SEQ_SELECT_ACTIVE_STEP:
		seq_selectedStep = cortex_midiMsg.data2;
		break;

	case FRONT_SEQ_SET_AUTOM_TRACK:
		seq_setActiveAutomationTrack(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_SET_QUANT:
			seq_setQuantisation(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_REQUEST_EUKLID_PARAMS:

//	case SEQ_EUKLID_LENGTH:
		activeParameters[PAR_EUKLID_LENGTH] = euklid_getLength(cortex_midiMsg.data2);//frontParser_midiMsg.data2;
//		menu_repaint();
//		break;

//	case SEQ_EUKLID_STEPS:
		activeParameters[PAR_EUKLID_STEPS] = euklid_getSteps(cortex_midiMsg.data2);//frontParser_midiMsg.data2;
//		menu_repaint();
//		break;

//	case SEQ_EUKLID_ROTATION:
		activeParameters[PAR_EUKLID_ROTATION] = euklid_getRotation(cortex_midiMsg.data2);// frontParser_midiMsg.data2;
		menu_repaint();

/*
		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_EUKLID_LENGTH);
		uart_sendFrontpanelByte(euklid_getLength(cortex_midiMsg.data2));

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_EUKLID_STEPS);
		uart_sendFrontpanelByte(euklid_getSteps(cortex_midiMsg.data2));

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_EUKLID_ROTATION);
		uart_sendFrontpanelByte(euklid_getRotation(cortex_midiMsg.data2));
		*/
		break;

	case FRONT_SEQ_SET_SHOWN_PATTERN:
		frontParser_shownPattern = cortex_midiMsg.data2;
		break;
#endif
	case FRONT_SEQ_SET_ACTIVE_TRACK:
		frontParser_activeTrack = cortex_midiMsg.data2;
		break;
#if !EURORACK_MODULE
	case FRONT_SEQ_REQUEST_STEP_PARAMS:{

		const uint8_t data2 = cortex_midiMsg.data2;

		/* send back probability, volume and note nr*/
		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_VOLUME);
		uart_sendFrontpanelByte(seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][cortex_midiMsg.data2].volume&STEP_VOLUME_MASK);

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_NOTE);
		uart_sendFrontpanelByte(seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][cortex_midiMsg.data2].note);

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_PROB);
		uart_sendFrontpanelByte(seq_patternSet.seq_subStepPattern[frontParser_shownPattern][frontParser_activeTrack][cortex_midiMsg.data2].prob);


		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_FLAM);
		uart_sendFrontpanelByte(seq_patternSet.seq_stepTiming[frontParser_shownPattern][frontParser_activeTrack][cortex_midiMsg.data2].flam);

		uart_sendFrontpanelByte(FRONT_SEQ_CC);
		uart_sendFrontpanelByte(FRONT_SEQ_SHIFT);
		uart_sendFrontpanelByte(seq_patternSet.seq_stepTiming[frontParser_shownPattern][frontParser_activeTrack][cortex_midiMsg.data2].shift);



		//send back automation params
		// **AUTOM subtract one for differing offsets when parameter is < 128
		uint8_t hi,lo;
		uint8_t dest = seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
														[frontParser_activeTrack]
														[data2].param1Nr;
		if(dest < 128 && dest)
			dest--;
		hi = dest>>7;
		lo = dest&0x7f;
		uart_sendFrontpanelByte(FRONT_SET_P1_DEST);
		uart_sendFrontpanelByte(hi);
		uart_sendFrontpanelByte(lo);

		uint8_t val = seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
													   [frontParser_activeTrack]
													   [data2].param1Val;
		hi = val>>7;
		lo = val&0x7f;
		uart_sendFrontpanelByte(FRONT_SET_P1_VAL);
		uart_sendFrontpanelByte(hi);
		uart_sendFrontpanelByte(lo);
		// **AUTOM subtract one for differing offsets
		dest = seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
												[frontParser_activeTrack]
												[data2].param2Nr;
		if(dest < 128 && dest)
			dest--;
		hi = dest>>7;
		lo = dest&0x7f;
		uart_sendFrontpanelByte(FRONT_SET_P2_DEST);
		uart_sendFrontpanelByte(hi);
		uart_sendFrontpanelByte(lo);

		val = seq_patternSet.seq_subStepPattern[frontParser_shownPattern]
											   [frontParser_activeTrack]
											   [data2].param2Val;
		hi = val>>7;
		lo = val&0x7f;
		uart_sendFrontpanelByte(FRONT_SET_P2_VAL);
		uart_sendFrontpanelByte(hi);
		uart_sendFrontpanelByte(lo);



		frontParser_activeStep = cortex_midiMsg.data2;
	}
		break;

	case FRONT_SEQ_ROLL_RATE:
		seq_setRollRate(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_ROLL_ON_OFF:
	{

		const uint8_t onOff = cortex_midiMsg.data2 >> 4;
		const uint8_t voice = cortex_midiMsg.data2 & 0xf;
		seq_setRoll(voice,onOff);
	}
		break;

	case FRONT_SEQ_CHANGE_PAT:
		//switch to one of the 8 patterns on the next pattern start
		seq_setNextPattern(cortex_midiMsg.data2);
		break;


	case FRONT_SEQ_RUN_STOP:
		if(cortex_midiMsg.data2 == 0)
		{
			seq_resetPosition();
		}
		seq_setRunning(cortex_midiMsg.data2);
		break;

	case FRONT_SEQ_MUTE_TRACK:
		seq_setMute(cortex_midiMsg.data2,1);
		break;

	case FRONT_SEQ_UNMUTE_TRACK:
		seq_setMute(cortex_midiMsg.data2,0);
		break;
#endif
	case FRONT_SEQ_MIDI_ROUTING:
		midiParser_setRouting(cortex_midiMsg.data2);
		break;
	case FRONT_SEQ_MIDI_TX_FILTER:
	case FRONT_SEQ_MIDI_RX_FILTER:
		midiParser_setFilter(cortex_midiMsg.data1==FRONT_SEQ_MIDI_TX_FILTER, cortex_midiMsg.data2);
		break;
#if !EURORACK_MODULE
	case FRONT_SEQ_BAR_RESET_MODE:
		// a setting of 0 is default (keep track of bars in song), a setting of 1 is
		// to reset the bar counter when a manual pattern change occurs
		seq_resetBarOnPatternChange = cortex_midiMsg.data2;
		break;
#endif
	case FRONT_SEQ_TRIGGER_IN_PPQ:
		switch(cortex_midiMsg.data2)
		{
		case 0:
		default:
			trigger_prescalerClockInput = PRE_4_PPQ_IN;
			break;


		case 1:
			/*
			trigger_prescalerClockInput = PRE_8_PPQ_IN;
			break;

		case 2:

			trigger_prescalerClockInput = PRE_16_PPQ_IN;
			break;
		case 3:
			*/
			trigger_prescalerClockInput = PRE_24_PPQ_IN;
			break;

		}
		break;
	case FRONT_SEQ_TRIGGER_OUT1_PPQ:
		switch(cortex_midiMsg.data2)
		{
			case PRE_2_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_2_PPQ_OUT;
							break;

			default:
			case PRE_4_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_4_PPQ_OUT;
							break;
			case PRE_8_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_8_PPQ_OUT;
							break;
			case PRE_16_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_16_PPQ_OUT;
							break;
			case PRE_24_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_24_PPQ_OUT;
							break;
			case PRE_48_PPQ_OUT:
				trigger_dividerClockOut1 = PRE_48_PPQ_OUT;
							break;
		}

		break;

	case FRONT_SEQ_TRIGGER_OUT2_PPQ:
		/*
		switch(cortex_midiMsg.data2)
		{
		case 0:
			trigger_dividerClockOut2 = PRE_1_PPQ;
			break;

		default:
		case 1:
			trigger_dividerClockOut2 = PRE_4_PPQ;
			break;

		case 2:
			trigger_dividerClockOut2 = PRE_8_PPQ;
			break;
		case 3:
			trigger_dividerClockOut2 = PRE_16_PPQ;
			break;
		case 4:
			trigger_dividerClockOut2 = PRE_32_PPQ;
			break;
		break;
		}
		*/
		break;

	case FRONT_SEQ_TRIGGER_GATE_MODE:
		//trigger_setGatemode(cortex_midiMsg.data2);
		break;

	default:
		break;
	}
}
#endif
