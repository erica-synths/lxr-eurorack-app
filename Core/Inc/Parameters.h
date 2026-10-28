/*
 * Parameters.h
 *
 * Created: 27.07.2012 16:48:46
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#define FOLLOW_PATTERN 0x2
#define FOLLOW_BAR 0x1

#define NUM_KIT_SLOTS 128 //64

#include "stm32f7xx_hal.h"

enum ParamEnums
{
	
	PAR_NONE,	//TODO this is modwheel	- stupid offset +/- 1   /*0*/
	PAR_MOD_WHEEL ,												/*1*/
	
	//waveform parameters need to be grouped 
	//makes the special case to show their names instead of 0-127 values easier
	
	PAR_OSC_WAVE_DRUM1 = PAR_MOD_WHEEL,							/*1*/
	PAR_OSC_WAVE_DRUM2,
	PAR_OSC_WAVE_DRUM3,
	PAR_OSC_WAVE_SNARE,
	PAR_NRPN_DATA_ENTRY_COARSE,
	PAR_WAVE1_CYM,
	PAR_WAVE1_HH,
	
	PAR_COARSE1,
	PAR_FINE1,
	PAR_COARSE2,		/*10*/
	PAR_FINE2,
	PAR_COARSE3,
	PAR_FINE3,
	PAR_COARSE4,
	PAR_FINE4,	
	PAR_COARSE5,
	PAR_FINE5,		
	PAR_COARSE6,
	PAR_FINE6,	
	
	PAR_MOD_WAVE_DRUM1,	/*20*/
	PAR_MOD_WAVE_DRUM2,
	PAR_MOD_WAVE_DRUM3,
	PAR_WAVE2_CYM,
	PAR_WAVE3_CYM,	
	PAR_WAVE2_HH,
	PAR_WAVE3_HH,	

	PAR_NOISE_FREQ1,		
	PAR_MIX1,
	
	PAR_MOD_OSC_F1_CYM,
	PAR_MOD_OSC_F2_CYM,		//70		/*30*/
	PAR_MOD_OSC_GAIN1_CYM,	
	PAR_MOD_OSC_GAIN2_CYM,
	PAR_MOD_OSC_F1,
	PAR_MOD_OSC_F2,		
	PAR_MOD_OSC_GAIN1,	
	PAR_MOD_OSC_GAIN2,
	
	PAR_FILTER_FREQ_1,
	PAR_FILTER_FREQ_2,
	PAR_FILTER_FREQ_3,
	PAR_FILTER_FREQ_4,					/*40*/
	PAR_FILTER_FREQ_5,	
	PAR_FILTER_FREQ_6,	

	PAR_RESO_1,
	PAR_RESO_2,
	PAR_RESO_3,	
	PAR_RESO_4,
	PAR_RESO_5,
	PAR_RESO_6,	
	
	PAR_VELOA1,		
	PAR_VELOD1,							/*50*/
	PAR_VELOA2,
	PAR_VELOD2,	
	PAR_VELOA3,		
	PAR_VELOD3,	
	PAR_VELOA4,
	PAR_VELOD4,	
	PAR_VELOA5,
	PAR_VELOD5,	
	PAR_VELOA6,
	PAR_VELOD6_CLOSED,					/*60*/
	PAR_VELOD6_OPEN,
	
	PAR_VOL_SLOPE1,
	PAR_VOL_SLOPE2,
	PAR_VOL_SLOPE3,
	PAR_VOL_SLOPE4,	
	PAR_VOL_SLOPE5,		
	PAR_VOL_SLOPE6,	
	
	PAR_REPEAT4,	
	PAR_REPEAT5,
	
	PAR_MOD_EG1,						/*70*/
	PAR_MOD_EG2,
	PAR_MOD_EG3,
	PAR_MOD_EG4,
	
	PAR_MODAMNT1,
	PAR_MODAMNT2,
	PAR_MODAMNT3,
	PAR_MODAMNT4,	
	
	PAR_PITCH_SLOPE1,
	PAR_PITCH_SLOPE2,
	PAR_PITCH_SLOPE3,					/*80*/
	PAR_PITCH_SLOPE4,
	
	PAR_FMAMNT1,	
	PAR_FM_FREQ1,	
	PAR_FMAMNT2,
	PAR_FM_FREQ2,	
	PAR_FMAMNT3,
	PAR_FM_FREQ3,	
	
	PAR_VOL1,	
	PAR_VOL2,	
	PAR_VOL3,							/*90*/
	PAR_VOL4,		
	PAR_VOL5,
	PAR_VOL6,	
	
	PAR_PAN1,	
	PAR_PAN2,	
	PAR_PAN3,
		PAR_NRPN_FINE,
		PAR_NRPN_COARSE,
	PAR_PAN4,
	PAR_PAN5,							/*100*/
	PAR_PAN6,	
	
	PAR_DRIVE1,	
	PAR_DRIVE2,	
	PAR_DRIVE3,	
	PAR_SNARE_DISTORTION,
	PAR_CYMBAL_DISTORTION,
	PAR_HAT_DISTORTION,	
	
	PAR_VOICE_DECIMATION1,
	PAR_VOICE_DECIMATION2,
	PAR_VOICE_DECIMATION3,				/*110*/
	PAR_VOICE_DECIMATION4,
	PAR_VOICE_DECIMATION5,
	PAR_VOICE_DECIMATION6,
	PAR_VOICE_DECIMATION_ALL, 	
	
	PAR_FREQ_LFO1 ,
	PAR_FREQ_LFO2,
	PAR_FREQ_LFO3,	
	PAR_FREQ_LFO4,
	PAR_FREQ_LFO5,
	PAR_FREQ_LFO6,						/*120*/
	
	PAR_AMOUNT_LFO1,
	PAR_AMOUNT_LFO2,
	PAR_AMOUNT_LFO3,
	PAR_AMOUNT_LFO4,
	PAR_AMOUNT_LFO5,		
	PAR_AMOUNT_LFO6,	
	
			PAR_BANK_CHANGE, //todo stupid offset -> param 0 /*127*/
	//######################################
	//######## END OF MIDI DATASIZE ########
	//######## PARAM NR 127 REACHED ########
	//######################################

	PAR_FILTER_DRIVE_1,					/*128*/
	PAR_FILTER_DRIVE_2,
	PAR_FILTER_DRIVE_3,					/*130*/
	PAR_FILTER_DRIVE_4,
	PAR_FILTER_DRIVE_5,
	PAR_FILTER_DRIVE_6,

	PAR_MIX_MOD_1,
	PAR_MIX_MOD_2,
	PAR_MIX_MOD_3,
	
	PAR_VOLUME_MOD_ON_OFF1,
	PAR_VOLUME_MOD_ON_OFF2,
	PAR_VOLUME_MOD_ON_OFF3,
	PAR_VOLUME_MOD_ON_OFF4,				/*140*/
	PAR_VOLUME_MOD_ON_OFF5,
	PAR_VOLUME_MOD_ON_OFF6,
	
	PAR_VELO_MOD_AMT_1,
	PAR_VELO_MOD_AMT_2,
	PAR_VELO_MOD_AMT_3,
	PAR_VELO_MOD_AMT_4,
	PAR_VELO_MOD_AMT_5,
	PAR_VELO_MOD_AMT_6,	
	
	// modulation destination for the velocity value in the sequencer
	// these are now indices into modTargets
	PAR_VEL_DEST_1,
	PAR_VEL_DEST_2,						/*150*/
	PAR_VEL_DEST_3,
	PAR_VEL_DEST_4,
	PAR_VEL_DEST_5,
	PAR_VEL_DEST_6,	
	
	PAR_WAVE_LFO1,
	PAR_WAVE_LFO2,
	PAR_WAVE_LFO3,
	PAR_WAVE_LFO4, 
	PAR_WAVE_LFO5,
	PAR_WAVE_LFO6,						/*160*/
	
	//the target and voice parameters must be after one another!
	// these represent the voice number that the lfo is targeting
	PAR_VOICE_LFO1,
	PAR_VOICE_LFO2,
	PAR_VOICE_LFO3,
	PAR_VOICE_LFO4,
	PAR_VOICE_LFO5,
	PAR_VOICE_LFO6,
	
	// these represent the parameter being targeted
	// these are now indices into modTargets
	PAR_TARGET_LFO1,	
	PAR_TARGET_LFO2,
	PAR_TARGET_LFO3,
	PAR_TARGET_LFO4,					/*170*/
	PAR_TARGET_LFO5,
	PAR_TARGET_LFO6,
	
	PAR_RETRIGGER_LFO1,
	PAR_RETRIGGER_LFO2,
	PAR_RETRIGGER_LFO3,
	PAR_RETRIGGER_LFO4,
	PAR_RETRIGGER_LFO5,
	PAR_RETRIGGER_LFO6,
	
	PAR_SYNC_LFO1,
	PAR_SYNC_LFO2,						/*180*/
	PAR_SYNC_LFO3,		
	PAR_SYNC_LFO4,
	PAR_SYNC_LFO5,
	PAR_SYNC_LFO6,
				
	PAR_OFFSET_LFO1,	
	PAR_OFFSET_LFO2,
	PAR_OFFSET_LFO3,
	PAR_OFFSET_LFO4,
	PAR_OFFSET_LFO5,
	PAR_OFFSET_LFO6,					/*190*/

	PAR_FILTER_TYPE_1,
	PAR_FILTER_TYPE_2,
	PAR_FILTER_TYPE_3,
	PAR_FILTER_TYPE_4,
	PAR_FILTER_TYPE_5,
	PAR_FILTER_TYPE_6,
	
	PAR_TRANS1_VOL,
	PAR_TRANS2_VOL,
	PAR_TRANS3_VOL,
	PAR_TRANS4_VOL,						/*200*/
	PAR_TRANS5_VOL,
	PAR_TRANS6_VOL,

	PAR_TRANS1_WAVE,
	PAR_TRANS2_WAVE,
	PAR_TRANS3_WAVE,	
	PAR_TRANS4_WAVE,	
	PAR_TRANS5_WAVE,	
	PAR_TRANS6_WAVE,	
	
	PAR_TRANS1_FREQ,
	PAR_TRANS2_FREQ,					/*210*/
	PAR_TRANS3_FREQ,
	PAR_TRANS4_FREQ,
	PAR_TRANS5_FREQ,
	PAR_TRANS6_FREQ,
	
	PAR_AUDIO_OUT1,
	PAR_AUDIO_OUT2,
	PAR_AUDIO_OUT3,
	PAR_AUDIO_OUT4,
	PAR_AUDIO_OUT5,
	PAR_AUDIO_OUT6,						/*220*/
	
//ATTENTION these were MIDI_NOTE_x and still appear as dummies in the mod target array!
	PAR_CV_1VOCT_1,
	PAR_CV_1VOCT_2,
	PAR_CV_1VOCT_3,
	PAR_CV_1VOCT_4,
	PAR_CV_1VOCT_5,
	PAR_CV_1VOCT_6,
	PAR_UNUSED_7,

	PAR_PWM_1,
	PAR_PWM_2,
	PAR_PWM_3, //230
	PAR_PWM_4,
	PAR_PWM_5,
	PAR_PWM_6,

	PAR_FX_TYPE,
	PAR_FX_ROUTING,
	PAR_FX_DRY_WET,

	PAR_FX_DIST_TYPE,
	PAR_FX_P1,
	PAR_FX_P2,
	PAR_FX_P3, //240
	PAR_FX_P4,
	PAR_FX_RATIO,
	PAR_FX_WAVE,
	PAR_FX_REVERB_TYPE,
	PAR_FX_DELAY_TYPE,
	PAR_MORPH_KIT_DEST,

/*ATTENTION kit load routine ignores everything following PAR_CV_DEST_1, when CV savbing is set to global!!!!*/

	PAR_CV_DEST_1,
	PAR_CV_DEST_2,
	PAR_CV_DEST_3,
	PAR_CV_DEST_4, //250
	PAR_CV_DEST_5,
	PAR_CV_DEST_6,
	PAR_CV_DEST_7,
	PAR_CV_DEST_8,
	//######################################
	//######## END OF MIDI DATASIZE     ####
	//######## PARAM NR CC2 127 REACHED ####
	//######################################
	PAR_CV_DEST_9, //255
	PAR_CV_DEST_10,
	PAR_CV_DEST_11,
	PAR_CV_DEST_12,
	PAR_CV_DEST_13,
	PAR_CV_DEST_14,
	PAR_CV_DEST_15,
	PAR_CV_DEST_16,
	PAR_CV_DEST_17,
	PAR_CV_DEST_18,



	PAR_CV_MOD_AMT_1,
	PAR_CV_MOD_AMT_2,
	PAR_CV_MOD_AMT_3,
	PAR_CV_MOD_AMT_4,
	PAR_CV_MOD_AMT_5,
	PAR_CV_MOD_AMT_6,
	PAR_CV_MOD_AMT_7,
	PAR_CV_MOD_AMT_8,
	PAR_CV_MOD_AMT_9,
	PAR_CV_MOD_AMT_10,
	PAR_CV_MOD_AMT_11,
	PAR_CV_MOD_AMT_12,
	PAR_CV_MOD_AMT_13,
	PAR_CV_MOD_AMT_14,
	PAR_CV_MOD_AMT_15,
	PAR_CV_MOD_AMT_16,
	PAR_CV_MOD_AMT_17,
	PAR_CV_MOD_AMT_18,

	PAR_CV_SRC_1,
	PAR_CV_SRC_2,
	PAR_CV_SRC_3,
	PAR_CV_SRC_4,
	PAR_CV_SRC_5,
	PAR_CV_SRC_6,
	PAR_CV_SRC_7,
	PAR_CV_SRC_8,
	PAR_CV_SRC_9,
	PAR_CV_SRC_10,
	PAR_CV_SRC_11,
	PAR_CV_SRC_12,
	PAR_CV_SRC_13,
	PAR_CV_SRC_14,
	PAR_CV_SRC_15,
	PAR_CV_SRC_16,
	PAR_CV_SRC_17,
	PAR_CV_SRC_18,


	//#########################################
	//######## End of sound Parameters ########
	//#########################################
	
	//all parameters in this section are only there to be referenced from the menu
	//they are not saved anywhere
	END_OF_SOUND_PARAMETERS,
	PAR_ROLL= END_OF_SOUND_PARAMETERS, //228
	PAR_MORPH,

	PAR_ACTIVE_STEP, 					/*230*/
	PAR_STEP_VOLUME,
	PAR_STEP_PROB,
	PAR_STEP_NOTE,
	
	PAR_EUKLID_LENGTH,
	PAR_EUKLID_STEPS,
	PAR_EUKLID_ROTATION,
	
	PAR_AUTOM_TRACK,
	
	// these are now indices into modTargets
	PAR_P1_DEST,
	PAR_P2_DEST,
	
	PAR_P1_VAL,							/*240*/
	PAR_P2_VAL,
	
	PAR_SHUFFLE,

	PAR_UNUSED,				// auto -> switch kits with pattern, off -> pattern change won't change kit
	PAR_PATTERN_NEXT,				//  - UNUSED
	
	PAR_TRACK_LENGTH,
	
	UNUSED_PAR_GLO_SAT,						//  - global saturation
	PAR_POS_Y,						//  - UNUSED
	PAR_FLUX,						//  - UNUSED
	PAR_SOM_FREQ,					//  - UNUSED
	PAR_TRACK_ROTATION,				// **PATROT

	PAR_STEP_FLAM,
	PAR_STEP_SHIFT,

	
	//#########################################
	//######## Global Parameters ##############
	//#########################################
	PAR_BEGINNING_OF_GLOBALS, //a placeholder to mark the beginning of the global var space not present in morph and not needed in the seq
	//global params
	PAR_BPM = PAR_BEGINNING_OF_GLOBALS,	/*250*/
	

	
	PAR_MIDI_CHAN_1,
	PAR_MIDI_CHAN_2,
	PAR_MIDI_CHAN_3,
	PAR_MIDI_CHAN_4,
	PAR_MIDI_CHAN_5,
	PAR_MIDI_CHAN_6,

	PAR_CV_SAVE,							// bool - UNUSED
	PAR_FOLLOW,							// bool
	
	
	PAR_QUANTISATION,					// UNUSED
	
	PAR_SCREENSAVER_ON_OFF,				// bool /*260*/
	PAR_MIDI_MODE,						// unused now
	PAR_MIDI_CHAN_7,
	PAR_MIDI_ROUTING,
	PAR_MIDI_FILT_TX,
	PAR_MIDI_FILT_RX,
	//prescaler control for trigger IO extension
	PAR_PRESCALER_CLOCK_IN,
	PAR_PRESCALER_CLOCK_OUT1,
	PAR_PRESCALER_CLOCK_OUT2,			//  - UNUSED
	PAR_TRIG_GATE_MODE,					//  - UNUSED

	PAR_BAR_RESET_MODE,					// bool 0 or 1   /*270*/
	PAR_MIDI_CHAN_GLOBAL,				// global midi channel
	// midi note - This is a note value that is sent when the track triggers, if it's 0 it means that
		// it will trigger the note that is specified on the step (PAR_STEP_NOTE). If it is non-zero then
		// PAR_STEP_NOTE is ignored and this note is sent instead
	//moved to globals

	PAR_MIDI_NOTE1,
	PAR_MIDI_NOTE2,
	PAR_MIDI_NOTE3,
	PAR_MIDI_NOTE4,
	PAR_MIDI_NOTE5,
	PAR_MIDI_NOTE6,
	PAR_MIDI_NOTE7,

	PAR_PATTERN_MODE,

	PAR_MORPH_TARGET, //which kit to morph to

	NUM_PARAMS	
};

extern uint8_t parameterOrder[END_OF_SOUND_PARAMETERS];


#endif /* PARAMETERS_H_ */
