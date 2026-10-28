/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "menu/menu.h"
#include "lcd.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ledHandler.h"

#include "IO/uart.h"


#include "Preset/PresetManager.h"
#include "menu/menuPages.h"
#include "menu/MenuText.h"
//#include "../Hardware/timebase.h"
//#include "../Hardware/SD/SPI_routines.h"
#include "menu/screensaver.h"
#include "menu/CcNr2Text.h"
//#include "menu/copyClearTools.h"
#include "buttonHandler.h"
#include <ctype.h>
#include <frontPanelParser_ui.h>
#include "front.h"
#include "Soundengine/FxBus.h"
//#include "Soundengine/Sequencer/sequencer.h"
#include "Soundengine/Soundengine.h"


uint8_t display_refresh_state = 0;
uint8_t display_needs_refresh = 0;
uint8_t display_needs_clear_all = 0;
uint8_t saveScreenPots[4]; //hold absolute pot position for save page name editing


#include "Soundengine/DSPAudio/modulationNode.h"

// uppercase 3 letters in buf
static void upr_three(char *buf);
// given a menuid and a param value fill buf with the short menu item value. will not exceed 3 chars
static void getMenuItemNameForValue(const uint8_t menuId, const uint8_t curParmVal, char *buf);
// given a menu id returns the number of entries
static uint8_t getMaxEntriesForMenu(uint8_t menuId);
// given a number, convert it to a note name
static void setNoteName(uint8_t num, char *buf);



static void menu_moveToMenuItem(int8_t inc);
static void menu_encoderChangeParameter(int8_t inc);

#define ARROW_SIGN '>'
#define ARROW_UP 0b11000101
#define ARROW_DOWN 0b11000110
//#define VERTICAL_LINE 0b01111100

enum saveStateEnum
{
	//SAVE_STATE_EDIT_TYPE,			// select one of the types of data to load/save (kit, pattern etc)
	SAVE_STATE_EDIT_PRESET_NR,		// editing the preset number slot
	SAVE_STATE_CHANGE_NAME_DIALOG_Y,
	SAVE_STATE_CHANGE_NAME_DIALOG_N,

	SAVE_STATE_EDIT_NAME1,
	SAVE_STATE_EDIT_NAME2,
	SAVE_STATE_EDIT_NAME3,
	SAVE_STATE_EDIT_NAME4,
	SAVE_STATE_EDIT_NAME5,
	SAVE_STATE_EDIT_NAME6,
	SAVE_STATE_EDIT_NAME7,
	SAVE_STATE_EDIT_NAME8,
	SAVE_STATE_OK					// confirmation is active
};

/** a struct for some save page parameters.*/
static volatile struct {
	unsigned category:3;					/**< 0= save kit, 1 = save pattern, 2 = morph sound, etc */
	unsigned nameBuffer:3;					// which name char array to use (kit, pattern, song, performance
	unsigned state:4;						/**< 0=edit kit/pat, 1=edit preset nr, 2-9 = edit name, 10=ok*/
} menu_saveOptions;


//-----------------------------------------------------------------
/** array holding all the names for the parameters
 *  short name, category, long name
 * These correspond with NamesEnum in menu.h
 * */
const Name valueNames[NUM_NAMES] =
{
		{SHORT_EMPTY,CAT_EMPTY,LONG_EMPTY},					//empty						//0
		{SHORT_COARSE,CAT_OSC,LONG_COARSE},					//coarse tune
		{SHORT_FINE,CAT_OSC,LONG_FINE},						//fine tune
		{SHORT_ATTACK,CAT_VELO_EG,LONG_ATTACK},				//attack velocity
		{SHORT_DECAY,CAT_VELO_EG,LONG_DECAY},				//decay velocity
		{SHORT_DECAY,CAT_PITCH_EG,LONG_DECAY},				//pitch decay
		{SHORT_MOD,CAT_PITCH_MOD,LONG_AMOUNT},				//mod amount
		{SHORT_AMNT,CAT_FM,LONG_AMOUNT},					//fm amount
		{SHORT_FREQ,CAT_FM,LONG_FREQ},						//fm frequency
		{SHORT_DRIVE, CAT_VOICE,LONG_DRIVE},				//drive
		{SHORT_VOL,CAT_VOICE,LONG_VOL},						//volume					//10
		{SHORT_PAN,CAT_VOICE,LONG_PAN},						//pan
		{SHORT_NOISE,CAT_NOISE,LONG_FREQ},					//noise frequency
		{SHORT_MIX,CAT_NOISE_OSC,LONG_MIX},					//mix
		{SHORT_REPEAT,CAT_VELO_EG,LONG_REPEAT_CNT},			//repeat
		{SHORT_FREQ,CAT_FILTER,LONG_FREQ},					//filter freq
		{SHORT_FIL_RESO,CAT_FILTER,LONG_RESONANCE},			//filter reso
		{SHORT_TYPE,CAT_FILTER,LONG_TYPE},				//filter type
		{SHORT_MOD_OSC1_FREQ,CAT_MOD_OSC,LONG_FREQ1},		//mod osc 1 freq
		{SHORT_MOD_OSC2_FREQ,CAT_MOD_OSC,LONG_FREQ2},		//mod osc 2 freq
		{SHORT_MOD_OSC1_GAIN,CAT_MOD_OSC,LONG_GAIN1},		//mod osc 1 gain			//20
		{SHORT_MOD_OSC2_GAIN,CAT_MOD_OSC,LONG_GAIN2},		//mod osc 2 gain

		{SHORT_FREQ,CAT_LFO,LONG_FREQ},						//freq lfo
		{SHORT_MOD,CAT_LFO,LONG_AMOUNT},					//mod lfo
		{SHORT_WAVE,CAT_LFO,LONG_WAVE},						//wave lfo
		{SHORT_DEST,CAT_LFO,LONG_DEST_PARAM},				//destination lfo (destination)

		{SHORT_SYNC,CAT_LFO,LONG_CLOCKSYNC},				//TEXT_SYNC_LFO,
		{SHORT_RETRIGGER,CAT_LFO,LONG_RETRIGGER},			//TEXT_RETRIGGER_LFO,
		{SHORT_OFFSET,CAT_LFO,LONG_OFFSET},					//TEXT_OFFSET_LFO,
		{SHORT_VOICE,CAT_LFO,LONG_DEST_VOICE},				//TEXT_TARGET_VOICE_LFO,

		{SHORT_SLOPE,CAT_VELO_EG,LONG_SLOPE},				//TEXT_EG_SLOPE,			//30

		{SHORT_DECAY1,CAT_VELO_EG,LONG_DECAY_CLOSED},		//TEXT_DECAY_CLOSED,
		{SHORT_DECAY2,CAT_VELO_EG,LONG_DECAY_OPEN},			//TEXT_DECAY_OPEN,

		{SHORT_WAVE,CAT_OSC,LONG_WAVE},						//TEXT_WAVEFORM

		{SHORT_WAVE,CAT_TRANS,LONG_WAVE},					//TEXT_TRANSIENT_WAVE
		{SHORT_VOL,CAT_TRANS,LONG_VOL},						//TEXT_TRANSIENT_VOLUME
		{SHORT_FREQ,CAT_TRANS,LONG_FREQ},					//TEXT_TRANSIENT_FREQ


		{SHORT_EQ_GAIN,CAT_EQ,LONG_GAIN},					//TEXT_EQ_GAIN
		{SHORT_EQ_FREQ,CAT_EQ,LONG_FREQ},					//TEXT_EQ_FREQ

#if USE_CODEC_EQ

		{SHORT_EQ,CAT_EQ,LONG_ENABLE},						//TEXT_CODEC_EQ_ENABLE
		{SHORT_MOD_OSC1_FREQ,CAT_EQ,LONG_FREQ},				//TEXT_CODEC_EQ_FB
		{SHORT_MOD_OSC2_FREQ,CAT_EQ,LONG_FREQ},				//TEXT_CODEC_EQ_FT
		{SHORT_MOD_OSC1_GAIN,CAT_EQ,LONG_GAIN},				//TEXT_CODEC_EQ_GB
		{SHORT_MOD_OSC2_GAIN,CAT_EQ,LONG_GAIN},				//TEXT_CODEC_EQ_GT

		{SHORT_LIMITER,CAT_LIMIT,LONG_ENABLE},				//TEXT_CODEC_LIMIT_ENABLE
		{SHORT_ATTACK,CAT_LIMIT,LONG_ATTACK},				//TEXT_CODEC_LIMIT_ATTACK
		{SHORT_RELEASE,CAT_LIMIT,LONG_RELEASE},				//TEXT_CODEC_LIMIT_RELEASE
		{SHORT_MAX,CAT_LIMIT,LONG_THRESH_MAX},				//TEXT_CODEC_LIMIT_MAX
		{SHORT_MIN,CAT_LIMIT,LONG_THRESH_MIN},				//TEXT_CODEC_LIMIT_MIN

#endif

		{SHORT_ROLL,CAT_PATTERN,LONG_ROLLRATE},				//TEXT_ROLL_SPEED
		{SHORT_MORPH,CAT_SOUND,LONG_MORPH},					//TEXT_X_FADE					//40


		{SHORT_VELOCITY,CAT_STEP,LONG_VELOCITY},			//TEXT_STEP_VELOCITY
		{SHORT_NOTE,CAT_STEP,LONG_NOTE},					//TEXT_NOTE
		{SHORT_PROBABILITY,CAT_STEP,LONG_PROBABILITY},		//TEXT_PROBABILITY,
		{SHORT_STEP,CAT_STEP,LONG_NUMBER},					//TEXT_ACTIVE_STEP

		{SHORT_LENGTH,CAT_PATTERN,LONG_LENGTH},				//TEXT_PAT_LENGTH,
		{SHORT_STEP,CAT_EUKLID,LONG_STEPS},					//TEXT_NUM_STEPS,
		{SHORT_ROTATION, CAT_EUKLID, LONG_ROTATION}, 		//TEXT_ROTATION
		{SHORT_FLAM, CAT_STEP, LONG_FLAM}, 		//TEXT_FLAM
		{SHORT_SHIFT, CAT_STEP, LONG_SHIFT}, 		//TEXT_SHIFT


		{SHORT_BPM,CAT_GLOBAL,LONG_TEMPO},					//TEXT_BPM

		{SHORT_CHANNEL,CAT_VOICE,LONG_MIDI_CHANNEL},		//TEXT_MIDI_CHANNEL
		{SHORT_OUT,CAT_VOICE,LONG_AUDIO_OUT},				//TEXT_AUDIO_OUT				//50

		{SHORT_SR,CAT_VOICE,LONG_SAMPLE_RATE},				//TEXT_SAMPLE_RATE

		{SHORT_REPEAT,CAT_PATTERN,LONG_REPEAT_CNT},				//TEXT_PATTERN_REPEAT
		{SHORT_NXT,CAT_PATTERN,LONG_NEXT_PAT},				//TEXT_PATTERN_NEXT

		//	{SHORT_PHASE,CAT_OSC,LONG_PHASE},				//TEXT_OSC_PHASE
		{SHORT_MODE,CAT_OSC,LONG_MODE},				//TEXT_MODE mix or mod for fm osc
		{SHORT_VOL,CAT_OSC,LONG_VOL},				//TEXT_OSC_VOLUME fm oscillator volume in mix mode
		{SHORT_DRIVE, CAT_FILTER,LONG_DRIVE},			//TEXT_FILTER_DRIVE

		{SHORT_DEST,	CAT_ACCENT,	LONG_DEST_PARAM},		//TEXT_VEL_DEST,
		{SHORT_AMNT,	CAT_ACCENT,	LONG_AMOUNT},			//TEXT_VEL_AMT,
		{SHORT_VOL,CAT_ACCENT,	LONG_VOLUME_MOD},		//TEXT_VEL_MOD_VOL,

		{SHORT_FETCH,	CAT_PARAMETER,	LONG_FETCH},		//TEXT_FETCH					//60
		{SHORT_FOLLOW,	CAT_SEQUENCER,	LONG_FOLLOW},		//TEXT_FOLLOW
		{SHORT_QUANT,	CAT_SEQUENCER,	LONG_QUANTISATION},			//TEXT_QUANTISATION

		{SHORT_TRACK,	CAT_SEQUENCER,	LONG_AUTOMATION_TRACK},			//TEXT_AUTOMATION_TRACK

		{SHORT_DEST,	CAT_SEQUENCER,	LONG_AUTOMATION_DEST},	//TEXT_PARAM_DEST
		{SHORT_VALUE,	CAT_SEQUENCER,	LONG_AUTOMATION_VAL},	//TEXT_PARAM_VAL

		{SHORT_SHUFFLE,	CAT_SEQUENCER,	LONG_SHUFFLE},	//TEXT_SHUFFLE

		{SHORT_SCREEN_SAVER, CAT_GLOBAL,LONG_SCREENSAVER},	//TEXT_SCREENSAVER_ON_OFF

		{SHORT_EMPTY,CAT_EMPTY,LONG_EMPTY},					//SKIP

		{SHORT_X,CAT_GENERATOR,LONG_X},	//TEXT_POS_X,
		{SHORT_Y,CAT_GENERATOR,LONG_Y},	//TEXT_POS_Y,										//70
		{SHORT_FLUX,CAT_GENERATOR,LONG_FLUX},	//TEXT_FLUX,
		{SHORT_FREQ,CAT_GENERATOR,LONG_FREQ},	//TEXT_SOM_FREQ,
		{SHORT_MIDI,CAT_MIDI,LONG_MODE},	//TEXT_MIDI_MODE
		{SHORT_MIDI_ROUTING, CAT_MIDI, LONG_MIDI_ROUTING}, // TEXT_MIDI_ROUTING
		{SHORT_MIDI_FILT_TX, CAT_MIDI, LONG_MIDI_FILT_TX}, // TEXT_MIDI_FILT_TX
		{SHORT_MIDI_FILT_RX, CAT_MIDI, LONG_MIDI_FILT_RX}, // TEXT_MIDI_FILT_RX
		{SHORT_TRIGGER_IN, CAT_CLOCK, LONG_CLOCK_IN}, // TEXT_TRIGGER_IN_PPQ
		{SHORT_TRIGGER_OUT1, CAT_CLOCK, LONG_CLOCK_OUT1}, // TEXT_TRIGGER_OUT1_PPQ
		{SHORT_TRIGGER_OUT2, CAT_CLOCK, LONG_TRIGGER_OUT2}, // TEXT_TRIGGER_OUT2_PPQ
		{SHORT_MODE, CAT_CLOCK, LONG_TRIGGER_GATE_MODE}, //TEXT_TRIGGER_GATE_MODE
		{SHORT_BAR_RESET_MODE, CAT_SEQUENCER, LONG_BAR_RESET_MODE}, // TEXT_BAR_RESET_MODE
		{SHORT_CHANNEL, CAT_MIDI, LONG_MIDI_CHANNEL}, // TEXT_MIDI_CHAN_GLOBAL
		{SHORT_PWM, CAT_OSC, LONG_PWM}, // PWM
		{SHORT_TYPE, CAT_FX, LONG_TYPE},  //TEXT_FX_TYPE
		{SHORT_OUT,CAT_FX,LONG_AUDIO_OUT},  //TEXT_FX_ROUTING*/
		{SHORT_DRY_WET, CAT_FX, LONG_DRY_WET},  //TEXT_FX_DRY_WET*/


		{SHORT_DRIVE, CAT_FX, LONG_DRIVE},/*TEXT_FX_DIST_DRIVE*/
		{SHORT_COLOR, CAT_FX, LONG_COLOR},/*TEXT_FX_DIST_P1*/
		{SHORT_RATIO, CAT_FX, LONG_RATIO},/*TEXT_FX_RATIO*/
		{SHORT_ATTACK, CAT_FX, LONG_ATTACK},/*TEXT_FX_COMP_ATTACK*/
		{SHORT_DECAY, CAT_FX, LONG_DECAY},/*TEXT_FX_COMP_DECAY*/
		{SHORT_THRESH, CAT_FX, LONG_THRESH},/*TEXT_FX_COMP_THRESHOLD*/
		{SHORT_FREQ, CAT_FX, LONG_FREQ},/*TEXT_FX_FREQ*/
		{SHORT_WAVE, CAT_FX, LONG_WAVE},/*TEXT_FX_WAVE*/
		{SHORT_VOL, CAT_FX, LONG_VOL},/*TEXT_FX_VOLUME*/
		{SHORT_TONE, CAT_FX, LONG_TONE},/*TEXT_FX_TONE*/
		{SHORT_GAIN, CAT_FX, LONG_GAIN},/*TEXT_FX_GAIN*/
		{SHORT_FEEDBACK, CAT_FX, LONG_FEEDBACK},/*TEXT_FX_FEEDBACK*/
		{SHORT_RANGE, CAT_FX, LONG_RANGE},/*TEXT_FX_RANGE*/
		{SHORT_TIME, CAT_FX, LONG_TIME},/*TEXT_FX_TIME*/
		{SHORT_PRE, CAT_FX, LONG_PREDELAY},/*TEXT_FX_PREDELAY*/
		{SHORT_REF, CAT_FX, LONG_REFLECTION},/*TEXT_FX_REFLECTION*/
		{SHORT_P1, CAT_FX, LONG_P1},/*TEXT_FX_P1*/
		{SHORT_P2, CAT_FX, LONG_P2},/*TEXT_FX_P2*/
		{SHORT_P3, CAT_FX, LONG_P3},/*TEXT_FX_P3*/
		{SHORT_P4, CAT_FX, LONG_P4},/*TEXT_FX_P4*/
		{SHORT_KCM, CAT_GLOBAL, LONG_PATTERN_MODE},/*TEXT_AUTO_SWITCH_KIT*/

		{SHORT_DEST, CAT_MOD, LONG_DEST_PARAM},/*TEXT_CV_DEST*/
		{SHORT_AMNT, CAT_MOD, LONG_AMOUNT},/*TEXT_CV_AMT*/
		{SHORT_SRC, CAT_MOD, LONG_SRC}, /*TEXT_CV_SRC*/
		{SHORT_1VOCT, CAT_MOD, LONG_1VOCT},/*TEXT_1VOCT*/

		{SHORT_CV, CAT_GLOBAL, LONG_CV},/*TEXT_CV_SAVE*/
		{SHORT_MORPH_TARGET, CAT_SOUND, LONG_MORPH_TARGET},/*TEXT_MORPH_TARGET*/



};

//---------------------------------------------------------------
// Parameter types. These correspond with ParamEnums and entries in parameter_values
const enum Datatypes PROGMEM parameter_dtypes[NUM_PARAMS] = {
	    /*PAR_NONE*/ 			DTYPE_0B127,							// 0
	    /*PAR_OSC_WAVE_DRUM1*/ 	DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_OSC_WAVE_DRUM2*/ 	DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_OSC_WAVE_DRUM3*/ 	DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_OSC_WAVE_SNARE*/  DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*NRPN_DATA_ENTRY_COARSE*/ DTYPE_0B127,
	    /*PAR_WAVE1_CYM*/  		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_WAVE1_HH*/  		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_COARSE1*/ 		DTYPE_PM63,//DTYPE_0B127,
	    /*PAR_FINE1*/ 			DTYPE_PM63,
	    /*PAR_COARSE2*/			DTYPE_PM63,//DTYPE_0B127,,							// 10
	    /*PAR_FINE2*/ 			DTYPE_PM63,
	    /*PAR_COARSE3*/ 		DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_FINE3*/ 			DTYPE_PM63,
	    /*PAR_COARSE4*/ 		DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_FINE4*/ 			DTYPE_PM63,
	    /*PAR_COARSE5*/			DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_FINE5*/ 			DTYPE_PM63,
	    /*PAR_COARSE6*/ 		DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_FINE6*/ 			DTYPE_PM63,
	    /*PAR_MOD_WAVE_DRUM1*/  DTYPE_MENU | (MENU_WAVEFORM<<8),		//20
	    /*PAR_MOD_WAVE_DRUM2*/  DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_MOD_WAVE_DRUM3*/  DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_WAVE2_CYM*/ 		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_WAVE3_CYM*/ 		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_WAVE2_HH*/ 		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_WAVE3_HH*/ 		DTYPE_MENU | (MENU_WAVEFORM<<8),
	    /*PAR_NOISE_FREQ1*/ 	DTYPE_0B127,
	    /*PAR_MIX1*/ 			DTYPE_0B127,
	    /*PAR_MOD_OSC_F1_CYM*/ 	DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_MOD_OSC_F2_CYM*/ 	DTYPE_PM63,//DTYPE_0B127,,							//30
	    /*PAR_MOD_OSC_GAIN1_CYM*/ DTYPE_0B127,
	    /*PAR_MOD_OSC_GAIN2_CYM*/ DTYPE_0B127,
	    /*PAR_MOD_OSC_F1*/ 		DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_MOD_OSC_F2*/ 		DTYPE_PM63,//DTYPE_0B127,,
	    /*PAR_MOD_OSC_GAIN1*/ 	DTYPE_0B127,
	    /*PAR_MOD_OSC_GAIN2*/ 	DTYPE_0B127,
	    /*PAR_FILTER_FREQ_1*/ 	DTYPE_0B127,
	    /*PAR_FILTER_FREQ_2*/ 	DTYPE_0B127,
	    /*PAR_FILTER_FREQ_3*/ 	DTYPE_0B127,
	    /*PAR_FILTER_FREQ_4*/ 	DTYPE_0B127,							//40
	    /*PAR_FILTER_FREQ_5*/ 	DTYPE_0B127,
	    /*PAR_FILTER_FREQ_6*/ 	DTYPE_0B127,
	    /*PAR_RESO_1*/ 			DTYPE_0B127,
	    /*PAR_RESO_2*/ 			DTYPE_0B127,
	    /*PAR_RESO_3*/ 			DTYPE_0B127,
	    /*PAR_RESO_4*/ 			DTYPE_0B127,
	    /*PAR_RESO_5*/ 			DTYPE_0B127,
	    /*PAR_RESO_6*/ 			DTYPE_0B127,
	    /*PAR_VELOA1*/ 			DTYPE_0B127,
	    /*PAR_VELOD1*/ 			DTYPE_0B127,							//50
	    /*PAR_VELOA2*/ 			DTYPE_0B127,
	    /*PAR_VELOD2*/ 			DTYPE_0B127,
	    /*PAR_VELOA3*/ 			DTYPE_0B127,
	    /*PAR_VELOD3*/ 			DTYPE_0B127,
	    /*PAR_VELOA4*/ 			DTYPE_0B127,
	    /*PAR_VELOD4*/ 			DTYPE_0B127,
	    /*PAR_VELOA5*/ 			DTYPE_0B127,
	    /*PAR_VELOD5*/ 			DTYPE_0B127,
	    /*PAR_VELOA6*/ 			DTYPE_0B127,
	    /*PAR_VELOD6_CLOSED*/ 	DTYPE_0B127,							//60
	    /*PAR_VELOD6_OPEN*/ 	DTYPE_0B127,
	    /*PAR_VOL_SLOPE1*/ 		DTYPE_0B127,
	    /*PAR_VOL_SLOPE2*/ 		DTYPE_0B127,
	    /*PAR_VOL_SLOPE3*/ 		DTYPE_0B127,
	    /*PAR_VOL_SLOPE4*/ 		DTYPE_0B127,
	    /*PAR_VOL_SLOPE5*/ 		DTYPE_0B127,
	    /*PAR_VOL_SLOPE6*/ 		DTYPE_0B127,
	    /*PAR_REPEAT4*/ 		DTYPE_0B127,
	    /*PAR_REPEAT5*/ 		DTYPE_0B127,
	    /*PAR_MOD_EG1*/ 		DTYPE_0B127,							//70
	    /*PAR_MOD_EG2*/ 		DTYPE_0B127,
	    /*PAR_MOD_EG3*/ 		DTYPE_0B127,
	    /*PAR_MOD_EG4*/ 		DTYPE_0B127,
	    /*PAR_MODAMNT1*/ 		DTYPE_0B127,
	    /*PAR_MODAMNT2*/ 		DTYPE_0B127,
	    /*PAR_MODAMNT3*/ 		DTYPE_0B127,
	    /*PAR_MODAMNT4*/ 		DTYPE_0B127,
	    /*PAR_PITCH_SLOPE1*/ 	DTYPE_0B127,
	    /*PAR_PITCH_SLOPE2*/ 	DTYPE_0B127,
	    /*PAR_PITCH_SLOPE3*/ 	DTYPE_0B127,							//80
	    /*PAR_PITCH_SLOPE4*/ 	DTYPE_0B127,
	    /*PAR_FMAMNT1*/ 		DTYPE_0B127,
	    /*PAR_FM_FREQ1*/ 		DTYPE_PM63,//DTYPE_0B127,
	    /*PAR_FMAMNT2*/ 		DTYPE_0B127,
	    /*PAR_FM_FREQ2*/ 		DTYPE_PM63,//DTYPE_0B127,
	    /*PAR_FMAMNT3*/ 		DTYPE_0B127,
	    /*PAR_FM_FREQ3*/ 		DTYPE_PM63,//DTYPE_0B127,
	    /*PAR_VOL1*/ 			DTYPE_0B127,
	    /*PAR_VOL2*/ 			DTYPE_0B127,
	    /*PAR_VOL3*/ 			DTYPE_0B127,							//90
	    /*PAR_VOL4*/ 			DTYPE_0B127,
	    /*PAR_VOL5*/ 			DTYPE_0B127,
	    /*PAR_VOL6*/ 			DTYPE_0B127,
	    /*PAR_PAN1*/ 			DTYPE_PM63,
	    /*PAR_PAN2*/ 			DTYPE_PM63,
	    /*PAR_PAN3*/ 			DTYPE_PM63,
	    /*NRPN_FINE*/ 			DTYPE_0B127,
	    /*NRPN_COARSE*/ 		DTYPE_0B127,
	    /*PAR_PAN4*/ 			DTYPE_PM63,
	    /*PAR_PAN5*/ 			DTYPE_PM63,								//100
	    /*PAR_PAN6*/ 			DTYPE_PM63,
	    /*PAR_DRIVE1*/ 			DTYPE_0B127,
	    /*PAR_DRIVE2*/ 			DTYPE_0B127,
	    /*PAR_DRIVE3*/ 			DTYPE_0B127,
	    /*PAR_SNARE_DISTORTION*/ DTYPE_0B127,
	    /*PAR_CYMBAL_DISTORTION*/ DTYPE_0B127,
	    /*PAR_HAT_DISTORTION*/ 	DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION1*/ DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION2*/ DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION3*/ DTYPE_0B127,							//110
	    /*PAR_VOICE_DECIMATION4*/ DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION5*/ DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION6*/ DTYPE_0B127,
	    /*PAR_VOICE_DECIMATION_ALL */ DTYPE_0B127,
	    /*PAR_FREQ_LFO1 */ 		DTYPE_0B127,
	    /*PAR_FREQ_LFO2*/		DTYPE_0B127,
	    /*PAR_FREQ_LFO3*/ 		DTYPE_0B127,
	    /*PAR_FREQ_LFO4*/ 		DTYPE_0B127,
	    /*PAR_FREQ_LFO5*/ 		DTYPE_0B127,
	    /*PAR_FREQ_LFO6*/ 		DTYPE_0B127,							//120
	    /*PAR_AMOUNT_LFO1*/		DTYPE_0B127,
	    /*PAR_AMOUNT_LFO2*/ 	DTYPE_0B127,
	    /*PAR_AMOUNT_LFO3*/ 	DTYPE_0B127,
	    /*PAR_AMOUNT_LFO4*/		DTYPE_0B127,
	    /*PAR_AMOUNT_LFO5*/ 	DTYPE_0B127,
	    /*PAR_AMOUNT_LFO6*/ 	DTYPE_0B127,
	    /*PAR_RESERVED4*/ 		DTYPE_0B127,
	    /*PAR_FILTER_DRIVE_1*/ 	DTYPE_0B127,
	    /*PAR_FILTER_DRIVE_2*/ 	DTYPE_0B127,
	    /*PAR_FILTER_DRIVE_3*/ 	DTYPE_0B127,							//130
	    /*PAR_FILTER_DRIVE_4*/ 	DTYPE_0B127,
	    /*PAR_FILTER_DRIVE_5*/ 	DTYPE_0B127,
	    /*PAR_FILTER_DRIVE_6*/	DTYPE_0B127,
	    /*PAR_MIX_MOD_1*/ 		DTYPE_MENU | (MENU_MIX_FM<<8),
	    /*PAR_MIX_MOD_2*/ 		DTYPE_MENU | (MENU_MIX_FM<<8),
	    /*PAR_MIX_MOD_3*/ 		DTYPE_MENU | (MENU_MIX_FM<<8),
	    /*PAR_VOLUME_MOD_ON_OFF1*/ DTYPE_ON_OFF,
	    /*PAR_VOLUME_MOD_ON_OFF2*/ DTYPE_ON_OFF,
	    /*PAR_VOLUME_MOD_ON_OFF3*/ DTYPE_ON_OFF,
	    /*PAR_VOLUME_MOD_ON_OFF4*/ DTYPE_ON_OFF,						//140
	    /*PAR_VOLUME_MOD_ON_OFF5*/ DTYPE_ON_OFF,
	    /*PAR_VOLUME_MOD_ON_OFF6*/ DTYPE_ON_OFF,
	    /*PAR_VELO_MOD_AMT_1*/ 	DTYPE_0B127,
	    /*PAR_VELO_MOD_AMT_2*/ 	DTYPE_0B127,
	    /*PAR_VELO_MOD_AMT_3*/ 	DTYPE_0B127,
	    /*PAR_VELO_MOD_AMT_4*/ 	DTYPE_0B127,
	    /*PAR_VELO_MOD_AMT_5*/ 	DTYPE_0B127,
	    /*PAR_VELO_MOD_AMT_6*/ 	DTYPE_0B127,
	    /*PAR_VEL_DEST_1*/ 		DTYPE_TARGET_SELECTION_VELO,
	    /*PAR_VEL_DEST_2*/ 		DTYPE_TARGET_SELECTION_VELO,			//150
	    /*PAR_VEL_DEST_3*/ 		DTYPE_TARGET_SELECTION_VELO,
	    /*PAR_VEL_DEST_4*/ 		DTYPE_TARGET_SELECTION_VELO,
	    /*PAR_VEL_DEST_5*/ 		DTYPE_TARGET_SELECTION_VELO,
	    /*PAR_VEL_DEST_6*/ 		DTYPE_TARGET_SELECTION_VELO,
	    /*PAR_WAVE_LFO1*/ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),
	    /*PAR_WAVE_LFO2*/ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),
	    /*PAR_WAVE_LFO3*/ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),
	    /*PAR_WAVE_LFO4 */ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),
	    /*PAR_WAVE_LFO5*/ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),
	    /*PAR_WAVE_LFO6*/ 		DTYPE_MENU | (MENU_LFO_WAVES<<8),		//160
	    /*PAR_VOICE_LFO1*/ 		DTYPE_VOICE_LFO,
	    /*PAR_VOICE_LFO2*/ 		DTYPE_VOICE_LFO,
	    /*PAR_VOICE_LFO3*/ 		DTYPE_VOICE_LFO,
	    /*PAR_VOICE_LFO4*/ 		DTYPE_VOICE_LFO,
	    /*PAR_VOICE_LFO5*/ 		DTYPE_VOICE_LFO,
	    /*PAR_VOICE_LFO6*/ 		DTYPE_VOICE_LFO,
	    /*PAR_TARGET_LFO1*/ 	DTYPE_TARGET_SELECTION_LFO,
	    /*PAR_TARGET_LFO2*/ 	DTYPE_TARGET_SELECTION_LFO,
	    /*PAR_TARGET_LFO3*/ 	DTYPE_TARGET_SELECTION_LFO,
	    /*PAR_TARGET_LFO4*/ 	DTYPE_TARGET_SELECTION_LFO,				//170
	    /*PAR_TARGET_LFO5*/ 	DTYPE_TARGET_SELECTION_LFO,
	    /*PAR_TARGET_LFO6*/ 	DTYPE_TARGET_SELECTION_LFO,
	    /*PAR_RETRIGGER_LFO1*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_RETRIGGER_LFO2*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_RETRIGGER_LFO3*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_RETRIGGER_LFO4*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_RETRIGGER_LFO5*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_RETRIGGER_LFO6*/ 	DTYPE_MENU | (MENU_RETRIGGER<<8),
	    /*PAR_SYNC_LFO1*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),
	    /*PAR_SYNC_LFO2*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),		//180
	    /*PAR_SYNC_LFO3*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),
	    /*PAR_SYNC_LFO4*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),
	    /*PAR_SYNC_LFO5*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),
	    /*PAR_SYNC_LFO6*/ 		DTYPE_MENU | (MENU_SYNC_RATES<<8),
	    /*PAR_OFFSET_LFO1*/ 	DTYPE_0B127,
	    /*PAR_OFFSET_LFO2*/ 	DTYPE_0B127,
	    /*PAR_OFFSET_LFO3*/ 	DTYPE_0B127,
	    /*PAR_OFFSET_LFO4*/ 	DTYPE_0B127,
	    /*PAR_OFFSET_LFO5*/ 	DTYPE_0B127,
	    /*PAR_OFFSET_LFO6*/ 	DTYPE_0B127,							//190
	    /*PAR_FILTER_TYPE_1*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_FILTER_TYPE_2*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_FILTER_TYPE_3*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_FILTER_TYPE_4*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_FILTER_TYPE_5*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_FILTER_TYPE_6*/ 	DTYPE_MENU | (MENU_FILTER<<8),
	    /*PAR_TRANS1_VOL*/ 		DTYPE_0B127,
	    /*PAR_TRANS2_VOL*/ 		DTYPE_0B127,
	    /*PAR_TRANS3_VOL*/ 		DTYPE_0B127,
	    /*PAR_TRANS4_VOL*/ 		DTYPE_0B127,							//200
	    /*PAR_TRANS5_VOL*/ 		DTYPE_0B127,
	    /*PAR_TRANS6_VOL*/ 		DTYPE_0B127,
	    /*PAR_TRANS1_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS2_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS3_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS4_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS5_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS6_WAVE*/ 	DTYPE_MENU | (MENU_TRANS<<8),
	    /*PAR_TRANS1_FREQ*/ 	DTYPE_0B127,
	    /*PAR_TRANS2_FREQ*/ 	DTYPE_0B127,							//210
	    /*PAR_TRANS3_FREQ*/ 	DTYPE_0B127,
	    /*PAR_TRANS4_FREQ*/ 	DTYPE_0B127,
	    /*PAR_TRANS5_FREQ*/ 	DTYPE_0B127,
	    /*PAR_TRANS6_FREQ*/ 	DTYPE_0B127,
	    /*PAR_AUDIO_OUT1*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),
	    /*PAR_AUDIO_OUT2*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),
	    /*PAR_AUDIO_OUT3*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),
	    /*PAR_AUDIO_OUT4*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),
	    /*PAR_AUDIO_OUT5*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),
	    /*PAR_AUDIO_OUT6*/ 		DTYPE_MENU | (MENU_AUDIO_OUT<<8),		//220
	    // these 7
	    /*CV_1VOCT_1*/		DTYPE_NOTE_NAME,
	    /*CV_1VOCT_2*/		DTYPE_NOTE_NAME,
	    /*CV_1VOCT_3*/		DTYPE_NOTE_NAME,
	    /*CV_1VOCT_4*/		DTYPE_NOTE_NAME,
	    /*CV_1VOCT_5*/		DTYPE_NOTE_NAME,
	    /*CV_1VOCT_6*/		DTYPE_NOTE_NAME,
	    /*PAR_UNUSED7*/		DTYPE_NOTE_NAME,

		/*PAR_PWM_1,*/			DTYPE_0B127,
		/*PAR_PWM_2,*/			DTYPE_0B127,
		/*PAR_PWM_3,*/			DTYPE_0B127,
		/*PAR_PWM_4,*/			DTYPE_0B127,
		/*PAR_PWM_5,*/			DTYPE_0B127,
		/*PAR_PWM_6,*/			DTYPE_0B127,

		/*PAR_FX_TYPE*/			DTYPE_MENU | (MENU_FX<<8),
		/*TEXT_FX_ROUTING*/		DTYPE_MENU | (MENU_AUDIO_OUT_FX<<8),
		/*TEXT_FX_AMOUNT*/		DTYPE_0B100,

		/*PAR_FX_DIST_TYPE*/ 	DTYPE_MENU| (MENU_DIST<<8),
		/*PAR_FX_P1*/			DTYPE_0B127,
		/*PAR_FX_P2*/			DTYPE_0B127,
		/*PAR_FX_P3*/			DTYPE_0B127,
		/*PAR_FX_P4*/			DTYPE_0B127,
		/*PAR_FX_RATIO*/		DTYPE_MENU | (MENU_RATIO<<8),
		/*PAR_FX_WAVE*/			DTYPE_MENU | (MENU_WAVEFORM<<8),
		/*PAR_FX_REVERB_TYPE*/	DTYPE_MENU | (MENU_REVERB<<8),
		/*PAR_FX_DELAY_TYPE*/	DTYPE_MENU | (MENU_DELAY<<8),
		/*PAR_MORPH_KIT_DEST*/	DTYPE_0B127,

		/*PAR_CV_DEST_1*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_2*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_3*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_4*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_5*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_6*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_7*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_8*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_9*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_10*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_11*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_12*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_13*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_14*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_15*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_16*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_17*/		DTYPE_TARGET_SELECTION_CV,
		/*PAR_CV_DEST_18*/		DTYPE_TARGET_SELECTION_CV,

		/*PAR_CV_MOD_AMT_1*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_2*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_3*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_4*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_5*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_6*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_7*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_8*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_9*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_10*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_11*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_12*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_13*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_14*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_15*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_16*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_17*/	DTYPE_0B100,
		/*PAR_CV_MOD_AMT_18*/	DTYPE_0B100,

		/*PAR_CV_SRC_1*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_2*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_3*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_4*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_5*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_6*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_7*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_8*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_9*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_10*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_11*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_12*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_13*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_14*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_15*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_16*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_17*/		DTYPE_MENU | (MENU_CV_SRC<<8),
		/*PAR_CV_SRC_18*/		DTYPE_MENU | (MENU_CV_SRC<<8),


	    /*PAR_ROLL*/ 			DTYPE_MENU | (MENU_ROLL_RATES<<8),
	    /*PAR_MORPH*/ 			DTYPE_0B255,
	    /*PAR_ACTIVE_STEP */ 	DTYPE_0B127,							//230
	    /*PAR_STEP_VOLUME*/ 	DTYPE_0B127,
	    /*PAR_STEP_PROB*/ 		DTYPE_0B100,
	    /*PAR_STEP_NOTE*/ 		DTYPE_NOTE_NAME,
	    /*PAR_EUKLID_LENGTH*/ 	DTYPE_1B16,
	    /*PAR_EUKLID_STEPS*/ 	DTYPE_1B16,
	    /*PAR_EUKLID_ROTATION*/	DTYPE_0B15,
	    /*PAR_AUTOM_TRACK*/ 	DTYPE_0b1,
	    /*PAR_P1_DEST*/ 		DTYPE_0B127,//DTYPE_AUTOM_TARGET, ->unused in module
	    /*PAR_P2_DEST*/ 		DTYPE_0B127,//DTYPE_AUTOM_TARGET, ->unused in module,
	    /*PAR_P1_VAL*/ 			DTYPE_0B127,							//240
	    /*PAR_P2_VAL*/ 			DTYPE_0B127,
	    /*PAR_SHUFFLE*/ 		DTYPE_0B127,
	    /*PAR_UNUSED*/ 			DTYPE_0B127,
	    /*PAR_PATTERN_NEXT*/ 	DTYPE_MENU | (MENU_NEXT_PATTERN<<8),
	    /*PAR_TRACK_LENGTH*/ 	DTYPE_1B64,
	    /*PAR_GLO_SAT*/			DTYPE_0B127,
	    /*PAR_POS_Y*/ 			DTYPE_0B127,
	    /*PAR_FLUX*/ 			DTYPE_0B127,
	    /*PAR_SOM_FREQ*/ 		DTYPE_0B127,
	    /*PAR_TRACK_ROTATION*/  DTYPE_1B16,  //**PATROT this is not shown in menu, but if it were it would really be 0 to 15
		/*PAR_STEP_FLAM*/ 		DTYPE_MENU | (MENU_STEP_FLAM<<8),							//251
		/*PAR_STEP_SHIFT*/		DTYPE_MENU | (MENU_STEP_SHIFT<<8),
	    /*PAR_BPM*/ 			DTYPE_0B255,							//251
	    /*PAR_MIDI_CHAN_1*/ 	DTYPE_1B16,
	    /*PAR_MIDI_CHAN_2*/ 	DTYPE_1B16,
	    /*PAR_MIDI_CHAN_3*/ 	DTYPE_1B16,
	    /*PAR_MIDI_CHAN_4*/ 	DTYPE_1B16,
	    /*PAR_MIDI_CHAN_5*/ 	DTYPE_1B16,
	    /*PAR_MIDI_CHAN_6*/ 	DTYPE_1B16,
	   /*PAR_CV_SAVE*/ 			DTYPE_MENU| (MENU_CV_SAVE<<8),
	    /*PAR_FOLLOW*/ 			DTYPE_ON_OFF,//DTYPE_MENU | (MENU_FOLLOW<<8),
	    /*PAR_QUANTISATION*/ 	DTYPE_MENU | (MENU_SEQ_QUANT<<8),
	    /*PAR_SCREENSAVER_ON_OFF*/ DTYPE_ON_OFF,						//261
	    /*PAR_MIDI_MODE*/ 		DTYPE_MENU | (MENU_MIDI<<8),			  // This is now unused.
	    /*PAR_MIDI_CHAN_7*/ 	DTYPE_1B16,
	    /*PAR_MIDI_ROUTING*/	DTYPE_MENU | (MENU_MIDI_ROUTING<<8),
	    /*PAR_MIDI_FILT_TX*/    DTYPE_MENU | (MENU_MIDI_FILTERING<<8),
	    /*PAR_MIDI_FILT_RX*/    DTYPE_MENU | (MENU_MIDI_FILTERING<<8),
		/*PAR_PRESCALER_CLOCK_IN*/		DTYPE_MENU | (MENU_PPQ_IN<<8),
		/*PAR_PRESCALER_CLOCK_OUT1*/	DTYPE_MENU | (MENU_PPQ_OUT<<8),
		/*PAR_PRESCALER_CLOCK_OUT2*/	DTYPE_MENU | (MENU_PPQ_OUT<<8),
		/*PAR_TRIGGER_GATE_MODE*/	DTYPE_ON_OFF,
	    /*PAR_BAR_RESET_MODE*/  DTYPE_ON_OFF,
	    /*PAR_MIDI_CHAN_GLOBAL*/DTYPE_1B16,		// global midi channel

	    /*PAR_MIDI_NOTE1*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE2*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE3*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE4*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE5*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE6*/		DTYPE_NOTE_NAME,
	    /*PAR_MIDI_NOTE7*/		DTYPE_NOTE_NAME,
		/*PAR_PATTERN_MODE*/ DTYPE_MENU | (MENU_PAT_MODE<<8),
		/*PAR_MORPH_TARGET*/ 	DTYPE_0B127
};



#define NUM_CHARS 76
//beware! rearranging this will break toUpper and toLowerCase functions!
const char characterOrder[NUM_CHARS] =
{
		32, //space
		65, //A
		66,
		67,
		68,
		69,
		70,
		71,
		72,
		73,
		74,
		75,
		76,
		77,
		78,
		79,
		80,
		81,
		82,
		83,
		84,
		85,
		86,
		87,
		88,
		89,
		90, //Z
		97, //a
		98,
		99,
		100,
		101,
		102,
		103,
		104,
		105,
		106,
		107,
		108,
		109,
		110,
		111,
		112,
		113,
		114,
		115,
		116,
		117,
		118,
		119,
		120,
		121,
		122, //z
		48, //0
		49,
		50,
		51,
		52,
		53,
		54,
		55,
		56,
		57, //9
		95, //_
		45, //-
		46, //.
		91, // [
		93, //]
		33, // !
		63, //?
		126, //~
		43, //+
		36, //$
		64,//@
		38,//&
		35//#
};
//-----------------------------------------------------------------------
//                            vars
//-----------------------------------------------------------------------
/** the lower 3 bit indicate the active parameter.
the upper bits indicate the active page no.
 */

char nameEditBuffer[8];

static uint8_t menuIndex = 0;

static uint8_t menu_TargetVoiceGapIndex = 0xFF;

uint8_t menu_numSamples = 0;

//preset vars
uint8_t menu_selectedPresetNr[NUM_PRESET_LOCATIONS];
uint8_t menu_loadedPresetNr[NUM_CURRENT];

uint32_t sucessTimer = 0;

uint8_t menu_shownPattern = 0;
uint8_t menu_muteModeActive = 0;
uint8_t menu_activeBar = 0;	//the currently visible bar, selectable with <-> bar buttons
uint8_t menu_songMenuPosition = 0;

/** buffer to minimize the display configuration.
It holds a representation of the display content so only the changed cells have to be updated*/
char currentDisplayBuffer[2][16];	/**< what is currently shown on the LCD*/
char editDisplayBuffer[2][17];		/**< what should be shown on the LCD after the next update. length 17 to allow zero termination without overflow.*/
uint8_t visibleCursor=0;	/* cursor position and whether it's visible rightmost=visible, then row (0,1) then col: ccccrv */
/* calc cursor to location (row, col both zero based) and make visible/invisible */
#define VIS_CURS(r,c,v) (((c) << 2) | ((r) << 1) | (v))
uint8_t menu_activePage = 0;				/**< indicates which menu page is currently shown*/
uint8_t menu_activeVoice = 0;
uint8_t menu_playedPattern = 0;
uint8_t menu_activePatternBank = 0;
static uint8_t editModeActive = 0;			/**< when edit mode is active, only the currently active parameter is shown with full name*/
static uint8_t lastEncoderButton = 0;		/**< stores the state of the encoder button so the edit mode is only switched once when the button is pressed*/

/** array holding all the available parameter values*/
uint8_t mainParameters[NUM_KIT_SLOTS][END_OF_SOUND_PARAMETERS];
//uint8_t abs_pot_Parameter_position[NUM_PARAMS];
uint8_t activeParameters[NUM_PARAMS];
//uint8_t morphParameters[END_OF_SOUND_PARAMETERS];/**< a second array for sound x-fade to another preset*/
//uint8_t *morphParameters;
//-----------------------------------------------------------------
void copyNameToEditBuffer(uint8_t presetNr, uint8_t nameType)
{
	for(int i=0;i<8;i++)
	{
		nameEditBuffer[i] = 0; //Make sure to use blank space if character is not found
		for(int j=0;j<NUM_CHARS;j++)
		{
			if(characterOrder[j] == savefileNames[presetNr][nameType][i])
			{
				nameEditBuffer[i] = j;
				break;
			}
		}
	}

}
//-----------------------------------------------------------------
void copyEditBufferToName(uint8_t presetNr, uint8_t nameType)
{
	for(int i=0;i<8;i++)
	{
		savefileNames[presetNr][nameType][i] = characterOrder[(uint8_t)nameEditBuffer[i]];
	}
}
//-----------------------------------------------------------------
void menu_setShownPattern(uint8_t patternNr)
{
	menu_shownPattern = patternNr;
	frontPanel_sendData(SEQ_CC,SEQ_SET_SHOWN_PATTERN,menu_shownPattern);
}

uint8_t menu_getViewedPattern()
{
	return menu_shownPattern;
}

//-----------------------------------------------------------------
void menu_init()
{
	lcd_clear();

	paramToModTargetInit();

	memset(menu_selectedPresetNr,0,sizeof(uint8_t) *NUM_PRESET_LOCATIONS );
	memset(menu_loadedPresetNr,0,sizeof(uint8_t) *NUM_CURRENT );

	memset(saveScreenPots,0,4*sizeof(uint8_t));

	memset(mainParameters, 0, sizeof(uint8_t) * END_OF_SOUND_PARAMETERS * NUM_KIT_SLOTS);


	activeParameters[PAR_EUKLID_LENGTH] = 16;
	activeParameters[PAR_EUKLID_STEPS] = 16;
	activeParameters[PAR_EUKLID_ROTATION] = 0;


	//initialize the roll value
	activeParameters[PAR_ROLL] = 5;
	//frontPanel_sendData(SEQ_CC,SEQ_ROLL_RATE,8); //value is initialized in cortex firmware

	activeParameters[PAR_BPM] = 120;


	// todo I tried to move everything below here to main.c before the call
	// to copyClear_clearCurrentPattern (see 7b0932f20b948ff29037bf6c2b1ecb7eba8bb89e)
	// but then the global data loaded from glo.cfg seemed like it was not being sent to the back.
	// need to figure out why as it's likely to crop up again at some point.

	//set voice 1 active
	menu_switchPage(0);
	frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,0);
	//the currently active button is lit
	led_setActiveVoice(0);

	//light up voice mode LED

	led_setMode2(0);


	//display start menu page
	menu_repaintAll();
}

//-----------------------------------------------------------------
/*
void menu_fastRefreshDisplay()
{
	// turn off the cursor
	lcd_turnOn(1,0);
	for(uint8_t i=0;i<2;i++)
	{
		for(uint8_t j=0;j<16;j++)
		{
			if(currentDisplayBuffer[i][j] != editDisplayBuffer[i][j])
			{
				//something has changed

				//we have to omit the zero termination sign
				if(editDisplayBuffer[i][j] == '\0')
				{
					//make a clear space out of zero termination
					editDisplayBuffer[i][j] = ' ';
				}

				//set cursor to current position
				lcd_setcursor(j,(uint8_t)(i+1));
				//send the new character
				lcd_data(editDisplayBuffer[i][j]);
				//update currentDisplayBuffer
				currentDisplayBuffer[i][j] = editDisplayBuffer[i][j];
			}
		}

	}

	// set cursor position and make visible or not
	lcd_setcursor((uint8_t)(visibleCursor >> 2), (uint8_t)(((visibleCursor & 2) >> 1)+1));
	lcd_turnOn(1,(visibleCursor & 1));
}
*/
//-----------------------------------------------------------------
/** compare the currentDisplayBuffer with the editDisplayBuffer and send all differences to the display*/

static volatile uint8_t refresh_i, refresh_j;

uint8_t sendDisplayBuffer()
{
#if 0
	// turn off the cursor
	lcd_turnOn(1,0);
	for(uint8_t i=0;i<2;i++)
	{
		for(uint8_t j=0;j<16;j++)
		{
			if(currentDisplayBuffer[i][j] != editDisplayBuffer[i][j])
			{
				//something has changed

				//we have to omit the zero termination sign
				if(editDisplayBuffer[i][j] == '\0')
				{
					//make a clear space out of zero termination
					editDisplayBuffer[i][j] = ' ';
				}

				//set cursor to current position
				lcd_setcursor(j,(uint8_t)(i+1));
				//send the new character
				lcd_data(editDisplayBuffer[i][j]);
				//update currentDisplayBuffer
				currentDisplayBuffer[i][j] = editDisplayBuffer[i][j];
			}	
		}

	}

	// set cursor position and make visible or not
	lcd_setcursor((uint8_t)(visibleCursor >> 2), (uint8_t)(((visibleCursor & 2) >> 1)+1));
	lcd_turnOn(1,(visibleCursor & 1));
#else
	switch(display_refresh_state)
	{
	case 0:
		if(display_needs_refresh)
		{
			display_refresh_state++;
			display_needs_refresh = 0;


		}
		//do nothing
		break;

	case 1:
		// turn off the cursor
		if(lcd_turnOn_nonblock(1,0))
		{
			display_refresh_state++;
			refresh_i = refresh_j = 0;
		}
		break;

	case 2:
		if(currentDisplayBuffer[refresh_i][refresh_j] != editDisplayBuffer[refresh_i][refresh_j])
		{
			//something has changed

			//we have to omit the zero termination sign
			if(editDisplayBuffer[refresh_i][refresh_j] == '\0')
			{
				//make a clear space out of zero termination
				editDisplayBuffer[refresh_i][refresh_j] = ' ';
			}
			display_refresh_state++;
		}
		else
		{
			refresh_j++;
			if(refresh_j>=16)
			{
				refresh_j=0;

				refresh_i++;
				if(refresh_i>=2)
				{
					display_refresh_state = 5;
					break;
				}
			}
		}

/*
		//set cursor to current position
		if(lcd_setcursor_nonBlock(refresh_j,(uint8_t)(refresh_i+1)))
		{
			//IS PROBABLY NEVER REACHED HERE
			//set cursor block only called to speed up state machine
			display_refresh_state++;
		}
*/
		break;


	case 3:
		//set cursor to current position
		if(lcd_setcursor_nonBlock(refresh_j,(uint8_t)(refresh_i+1)))
		{
			display_refresh_state++;
		}
		break;

	case 4:
		//send the new character
		if(lcd_data_nonBlock(editDisplayBuffer[refresh_i][refresh_j]))
		{
			//update currentDisplayBuffer
			//No idea why it happens that a zero termination slips through to here,
			//but it is sometimes the case and leads to wrong signs on the display!
			//probably when a menu_repaint() happens when sendDisplaybuffer is running...
			//but this seems an appropriate fix.
			if(editDisplayBuffer[refresh_i][refresh_j] == '\0')
			{
				//make a clear space out of zero termination
				editDisplayBuffer[refresh_i][refresh_j] = ' ';
			}
			currentDisplayBuffer[refresh_i][refresh_j] = editDisplayBuffer[refresh_i][refresh_j];

			refresh_j++;
			if(refresh_j>=16)
			{
				refresh_j=0;

				refresh_i++;
				if(refresh_i>=2)
				{
					display_refresh_state = 5;
					break;
				}
			}


			display_refresh_state = 2;


		}

		break;

	case 5:
		if(lcd_setcursor_nonBlock((uint8_t)(visibleCursor >> 2), (uint8_t)(((visibleCursor & 2) >> 1)+1)))
		{
			display_refresh_state++;
		}
		break;

	case 6:
		if(	lcd_turnOn_nonblock(1,(visibleCursor & 1)) )
		{
			display_refresh_state = 0;
		}
		break;
	}
#endif

	return display_refresh_state;
}
//-----------------------------------------------------------------
void menu_tick()
{

	if((menu_activePage == PATTERN_SAVE_PAGE_OK) || (menu_activePage == KIT_SAVE_PAGE_OK))
	{
		if(systick_ticks - sucessTimer > 4000)
		{

			buttonHandler_setMode(SELECT_MODE_VOICE);
			led_clearAllBlinkLeds();


			//update the status LED
			led_setMode2(SELECT_MODE_VOICE);
			//set menu to voice page mode
			menu_switchPage(menu_getActiveVoice());
			led_setActiveSelectButton(menu_getSubPage()+1);
			menu_resetActiveParameter();
		}
	}


	if(display_needs_clear_all && (display_refresh_state == 0))
	{
		//handled here, since a direct all clear could overwrite the display buffers while another repaint is in progress

		display_needs_clear_all = 0;
		memset(editDisplayBuffer,' ',2*17);
		memset(currentDisplayBuffer,127,2*16);
		menu_repaint();

		display_needs_refresh = 1;

		//TODO could possibly be sped up by aborting and restarting running transfer instead of queing a new one
	}



	sendDisplayBuffer();
}
//-----------------------------------------------------------------
void menu_repaintAll()
{

	//if(copyClear_Mode == MODE_NONE)
	{
		display_needs_clear_all = 1;


		//move memset to nonblocking routine to not overwrite while in progress
		//memset(editDisplayBuffer,' ',2*17);
		//memset(currentDisplayBuffer,127,2*16);

		//menu_repaint();
	}		
}
//-----------------------------------------------------------------
static void menu_repaintSimplePage()
{
#if !EURORACK_MODULE
	if(copyClear_showClearMenu == 1)
	{
		strcpy(&editDisplayBuffer[0][0], "clear [");
		switch(copyClear_clearTarget)
		{
			default:

			case CLEAR_KIT:
				strcpy(&editDisplayBuffer[0][7], "kit]?");
				break;

			case CLEAR_TRACK:
				strcpy(&editDisplayBuffer[0][7], "track]?");
			break;

			case CLEAR_PATTERN:
				strcpy(&editDisplayBuffer[0][7], "pattern]?");
			break;

			case CLEAR_AUTOMATION1:
				strcpy(&editDisplayBuffer[0][7], "autom. 1]?");
			break;

			case CLEAR_AUTOMATION2:
				strcpy(&editDisplayBuffer[0][7], "autom. 2]?");
			break;
		}


	}
	else
#endif
		if(menu_activePage == PATTERN_LOAD_PAGE)
	{
		strcpy(&editDisplayBuffer[0][0], "Select Pattern");
		strcpy(&editDisplayBuffer[1][0], "Current: ");
		numtostrps(&editDisplayBuffer[1][9],menu_playedPattern+1);
	}
	else if(menu_activePage == PATTERN_SAVE_PAGE)
	{
		strcpy(&editDisplayBuffer[0][0], "Save Pattern");
		strcpy(&editDisplayBuffer[1][0], "Current: ");
		numtostrps(&editDisplayBuffer[1][9],menu_playedPattern+1);
	}
	else if(menu_activePage == PATTERN_SAVE_PAGE_OK)
	{
		memset(&editDisplayBuffer[1][0], ' ', 16);
		strcpy(&editDisplayBuffer[0][0], "Save Pattern");
		strcpy(&editDisplayBuffer[1][0], "success!");

	}
	else if(menu_activePage == KIT_SAVE_PAGE_OK)
	{
		memset(&editDisplayBuffer[1][0], ' ', 16);
		strcpy(&editDisplayBuffer[0][0], "Save Kit");
		strcpy(&editDisplayBuffer[1][0], "success!");
	}
#if !EURORACK_MODULE
	else if(menu_activePage == MENU_LAST_STEP_PAGE)
	{
		strcpy(&editDisplayBuffer[0][0], "Last Step ");
		uint8_t voice = menu_getActiveVoice();
		if(buttonHandler_getMode() == SELECT_MODE_LAST_STEP_TRACK) //track
		{
			strcpy(&editDisplayBuffer[0][10], "Trk: ");
			char trackNr[4];
			sprintf(trackNr, "%d", voice+1);
			strcpy(&editDisplayBuffer[0][15], trackNr);

			strcpy(&editDisplayBuffer[1][0], "Current: ");
			numtostrps(&editDisplayBuffer[1][9],seq_getTrackLength(menu_activeVoice));
		}


		else //all
		{
			strcpy(&editDisplayBuffer[0][10], "All");
			strcpy(&editDisplayBuffer[1][0], "Current: ");
			numtostrps(&editDisplayBuffer[1][9],seq_getTrackLength(menu_activeVoice));
		}



	}
#endif
#if !EURORACK_MODULE
	else if(menu_activePage == SONG_MODE_PAGE)
	{
		//show list of chained patterns
		//0123456789..
		// [002] | p05 ↑
		//> 003  | p01 ↓


		uint8_t pos = seq_songPositionPointer;
		/*if(pos==255) pos = 0;
		else if(pos>MAX_SONG_LENGTH) pos = MAX_SONG_LENGTH;
		*/


		//song position pointer if visible
		if(pos == menu_songMenuPosition)
		{
			editDisplayBuffer[0][0] = '>';
		}
		else if(pos == menu_songMenuPosition+1)
		{
			editDisplayBuffer[1][0] = '>';
		}

		//----- line 1 -----
		if(editModeActive)
		{

			numtostrpu(&editDisplayBuffer[0][2], menu_songMenuPosition, '0');

			editDisplayBuffer[0][8] = '[';
			editDisplayBuffer[0][12] = ']';
		}
		else
		{
			editDisplayBuffer[0][1] = '[';
			numtostrpu(&editDisplayBuffer[0][2], menu_songMenuPosition, '0');
			editDisplayBuffer[0][5] = ']';
		}
		editDisplayBuffer[0][7] = '|';

		if(seq_activeSong[menu_songMenuPosition] < SONG_END)
		{
			//show pattern number
			numtostrpu(&editDisplayBuffer[0][9], seq_activeSong[menu_songMenuPosition], '0');
			editDisplayBuffer[0][9] = 'p'; //overwrites the first zero... we only have 64 patterns

		}
		else if(seq_activeSong[menu_songMenuPosition] == SONG_LOOP)
		{
			strcpy(&editDisplayBuffer[0][9], "LOOP");
			if(editModeActive) editDisplayBuffer[0][13] = ']';
		}
		else if(seq_activeSong[menu_songMenuPosition] == SONG_END)
		{
			strcpy(&editDisplayBuffer[0][9], "END");
			if(editModeActive) editDisplayBuffer[0][12] = ']';
		}

		if( (menu_songMenuPosition) > 0)
			editDisplayBuffer[0][15] = ARROW_UP;


		//----- line 2 -----
		if( (menu_songMenuPosition+1) < MAX_SONG_LENGTH)
		{
			numtostrpu(&editDisplayBuffer[1][2], menu_songMenuPosition+1, '0');

			editDisplayBuffer[1][7] = '|';

			if(seq_activeSong[menu_songMenuPosition+1] < SONG_END)
			{
				//show pattern number
				numtostrpu(&editDisplayBuffer[1][9], seq_activeSong[menu_songMenuPosition + 1], '0');
				editDisplayBuffer[1][9] = 'p'; //overwrites the first zero... we only have 64 patterns

			}
			else if(seq_activeSong[menu_songMenuPosition+1] == SONG_LOOP)
			{
				strcpy(&editDisplayBuffer[1][9], "LOOP");
			}
			else if(seq_activeSong[menu_songMenuPosition+1] == SONG_END)
			{
				strcpy(&editDisplayBuffer[1][9], "END");
			}


			if( (menu_songMenuPosition+2) < MAX_SONG_LENGTH)
				editDisplayBuffer[1][15] = ARROW_DOWN;
		}

	}
#endif
}
//-----------------------------------------------------------------
static void menu_repaintLoadSavePage()
{


	const char *toptxt=0;
	//Top row
	strcpy(&editDisplayBuffer[0][0],
			menu_activePage == SAVE_PAGE ?
					PSTR("Save:") :
					PSTR("Load:"));



	switch(menu_saveOptions.category) {
	default:
	case SAVE_TYPE_KIT:
		toptxt=PSTR("Kit     ");
		break;
#if !EURORACK_MODULE
	case SAVE_TYPE_PATTERN:
		toptxt=PSTR("Pattern ");
		break;
#endif
	case SAVE_TYPE_MORPH:
		toptxt=PSTR("MorphKit");
		break;
#if !EURORACK_MODULE
	case SAVE_TYPE_SONG:
		toptxt=PSTR("Song    ");
		break;
#endif
	case SAVE_TYPE_GLO:
		toptxt=PSTR("Settings");
		break;
#if !EURORACK_MODULE
	case SAVE_TYPE_PROJECT:
		toptxt=PSTR("Project ");
		break;
	case SAVE_TYPE_ALL:
		toptxt=PSTR("All     ");
		break;
#endif
	//case SAVE_TYPE_SAMPLES:     toptxt=PSTR("Samples "); break;
	}

	strcpy(&editDisplayBuffer[0][6],toptxt);
	/*
	if(menu_saveOptions.state == SAVE_STATE_EDIT_TYPE) {
		if(editModeActive) {
			editDisplayBuffer[0][5]='[';
			editDisplayBuffer[0][14]=']';
		} else {
			//arrow before parameter
			editDisplayBuffer[0][5]= ARROW_SIGN;
		}
	}
	*/
/*
	if(menu_saveOptions.category == SAVE_TYPE_PATTERN)
	{
		if(menu_activePage == SAVE_PAGE)
		{
			if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_N)
			{
				strcpy(&editDisplayBuffer[1][0], "to SD? [y/N]");
			}
			else if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_Y)
			{
				strcpy(&editDisplayBuffer[1][0], "to SD? [Y/n]");
			}
		}
		else
		{
			if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_N)
			{
				strcpy(&editDisplayBuffer[1][0], "from SD? [y/N]");
			}
			else if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_Y)
			{
				strcpy(&editDisplayBuffer[1][0], "from SD? [Y/n]");
			}
		}
	}
	else
		*/
	{

		//Bottom row - name and number if applicable
		if( menu_saveOptions.category < SAVE_TYPE_GLO) { //no name and number for global settings or samples
			//the preset number
			numtostrpu(&editDisplayBuffer[1][1], menu_selectedPresetNr[menu_saveOptions.category],' ');

			if(menu_saveOptions.state == SAVE_STATE_EDIT_PRESET_NR)
			{
				//if(dummyEditMode) {
					editDisplayBuffer[1][0]='[';
					editDisplayBuffer[1][4]=']';
					/*
				} else {
					//arrow before parameter
					editDisplayBuffer[1][0]= ARROW_SIGN;
					//visibleCursor = VIS_CURS(1,1,1);
				}
				*/
			}
			// write the full preset name
#if 0
			memcpy(&editDisplayBuffer[1][5],(const void*)savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer],8);
#else
			for(int i=0; i<8;i++)
			{
				editDisplayBuffer[1][5+i] = characterOrder[(uint8_t)nameEditBuffer[i]];
			}


#endif
		}

		// bottom row - for save page, we show editing box or underline cursor as applicable
		if(menu_activePage == SAVE_PAGE)
		{
			//the preset number
			if( menu_saveOptions.category < SAVE_TYPE_GLO) //not saving global settings, so print name and number
			{
				// if we are editing the name
				if( (menu_saveOptions.state >= SAVE_STATE_EDIT_NAME1) && (menu_saveOptions.state <= SAVE_STATE_EDIT_NAME8) )
				{
					if(editModeActive) { // draw a box around active character (encoder changes value)
						editDisplayBuffer[1][4+menu_saveOptions.state-SAVE_STATE_EDIT_NAME1]='[';
						editDisplayBuffer[1][6+menu_saveOptions.state-SAVE_STATE_EDIT_NAME1]=']';
					} else { // using encoder to move left and right, using knob to change letter
						//underline under current char
						visibleCursor =(uint8_t)( VIS_CURS(1,5+menu_saveOptions.state-SAVE_STATE_EDIT_NAME1,1));
					}

					//ok button - shown in naming mode
					memcpy(&editDisplayBuffer[1][14],menuText_ok,2);
				}
				else if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_N)
				{
					strcpy(&editDisplayBuffer[1][0], "Edit Name? [y/N]");
				}
				else if (menu_saveOptions.state == SAVE_STATE_CHANGE_NAME_DIALOG_Y)
				{
					strcpy(&editDisplayBuffer[1][0], "Edit Name? [Y/n]");
				}
			} // not saving global settings


			if((menu_saveOptions.state==SAVE_STATE_OK) ||
					(menu_saveOptions.category >= SAVE_TYPE_GLO && menu_saveOptions.state >= SAVE_STATE_EDIT_PRESET_NR))
			{
				//arrow before parameter
				editDisplayBuffer[1][13]= ARROW_SIGN;
				//visibleCursor = VIS_CURS(1,14,1);
				//ok button - shown in naming mode
				memcpy(&editDisplayBuffer[1][14],menuText_ok,2);
			}
		} else { // bottom row - Load page is active
			//ok button shown for none
			/*
			if(menu_saveOptions.category != SAVE_TYPE_KIT && menu_saveOptions.category != SAVE_TYPE_MORPH) {
				memcpy(&editDisplayBuffer[1][14],menuText_ok,2);

				if((menu_saveOptions.state==SAVE_STATE_OK) ||
						(menu_saveOptions.category >= SAVE_TYPE_GLO && menu_saveOptions.state > SAVE_STATE_EDIT_TYPE)) {
					//arrow before parameter
					editDisplayBuffer[1][13]= ARROW_SIGN;
					//visibleCursor = VIS_CURS(1,14,1);
				}
			} else { //clear ok text
				*/
				editDisplayBuffer[1][14] = 0;
				editDisplayBuffer[1][15] = 0;
			//}
		}
	}
}
//-----------------------------------------------------------------
static uint8_t has2ndPage(uint8_t menuPage)
{
	uint8_t textType;
	//if in mod matrix, use modMatrixPages instead of menuPages
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		textType = pgm_read_byte(&modMatrixPages[menu_activePage][menuPage].top1 + 4);
	}
	else
	{
		// the active page
		textType = pgm_read_byte(&menuPages[menu_activePage][menuPage].top1 + 4);
	}

//	const uint8_t textType = pgm_read_byte(&menuPages[menu_activePage][menuPage].top1 + 4);
	if(textType != TEXT_EMPTY)
	{
		return 1;
	}
	else return 0;	

}
//-----------------------------------------------------------------
static uint8_t checkScrollSign(uint8_t activePage, uint8_t activeParameter)
{
	const uint8_t is2ndPage = (uint8_t)(activeParameter>3);

#if 1
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		if(is2ndPage) {
			//if we are on 2nd screen, and there are more sub-pages after this show "*" to signify both ways are available
			if((activePage < 2-1) && (pgm_read_byte(&modMatrixPages[MENU_MIDI_PAGE][activePage+1].top1) != TEXT_EMPTY))
				return '*';
			else // on 2nd screen, no sub-pages after this
				return '<';
		} else { // on 1st screen
			if(has2ndPage(activePage)) { // have a second screen
				if(activePage > 0)  // on first screen and there are sub-pages before this and a second screen after
					return '*';
				else
					return '>';
			} else { // don't have a second screen
				if(activePage > 0) // on first screen and have sub-pages before this but not second screen after
					return '<';
				else // on first screen, no sub-pages before... would not happen
					return 0;
			}
		}
	}
	else
#endif
	{

		//**GMENU show '*' when both left and right movement are possible in the global settings menu
		// show > or < as appropriate
	#if 1
		if(menu_activePage==MENU_MIDI_PAGE) {
			if(is2ndPage) {
				//if we are on 2nd screen, and there are more sub-pages after this show "*" to signify both ways are available
				if((activePage < NUM_SUB_PAGES-1) && (pgm_read_byte(&menuPages[MENU_MIDI_PAGE][activePage+1].top1) != TEXT_EMPTY))
					return '*';
				else // on 2nd screen, no sub-pages after this
					return '<';
			} else { // on 1st screen
				if(has2ndPage(activePage)) { // have a second screen
					if(activePage > 0)  // on first screen and there are sub-pages before this and a second screen after
						return '*';
					else
						return '>';
				} else { // don't have a second screen
					if(activePage > 0) // on first screen and have sub-pages before this but not second screen after
						return '<';
					else // on first screen, no sub-pages before... would not happen
						return 0;
				}
			}
		}
		else if(menu_activePage==SEQ_PAGE) {
			if(is2ndPage) {
				//if we are on 2nd screen, and there are more sub-pages after this show "*" to signify both ways are available
				if((activePage < NUM_SUB_PAGES-1) && (pgm_read_byte(&menuPages[SEQ_PAGE][activePage+1].top1) != TEXT_EMPTY))
					return '*';
				else // on 2nd screen, no sub-pages after this
					return '<';
			} else { // on 1st screen
				if(has2ndPage(activePage)) { // have a second screen
					if(activePage > 0)  // on first screen and there are sub-pages before this and a second screen after
						return '*';
					else
						return '>';
				} else { // don't have a second screen
					if(activePage > 0) // on first screen and have sub-pages before this but not second screen after
						return '<';
					else // on first screen, no sub-pages before... would not happen
						return 0;
				}
			}
		}
#endif
	}

	// normal handling for other menus
	if(has2ndPage(activePage))
		return is2ndPage?'<':'>';
	else
		return 0;

}
//-----------------------------------------------------------------
void menu_setNumSamples(uint8_t num)
{
	menu_numSamples = num;
}
//-----------------------------------------------------------------

static void menu_displayModTargetFull(uint8_t curParmVal)
{

	//**AUTOM **VELO - determine the full name to display for edit mode
	// a letters for the category and 8 letters for the name

	if( pgm_read_word(&modTargets[curParmVal].param)==PAR_NONE ) {
		strcpy(&editDisplayBuffer[1][0],menuText_off);
		return;
	}

	uint8_t nameidx = pgm_read_byte(&modTargets[curParmVal].nameIdx);

	strcpy(&editDisplayBuffer[1][0],
		(const char PROGMEM *)(&catNames[
					 pgm_read_byte(&valueNames[nameidx].category)
										]));
	strcpy(&editDisplayBuffer[1][8],
		(const char PROGMEM *)(&longNames[
					pgm_read_byte(&valueNames[nameidx].longName)
										 ]));
}

static void menu_displayModTargetShort(uint8_t curParmVal, char *valueAsText, char inclVoice)
{
	//**AUTOM **VELO - render a short name for non edit mode (encoder mode)

	if( pgm_read_word(&modTargets[curParmVal].param)==PAR_NONE ) {
		memcpy(valueAsText,menuText_off,3);
	} else {
		uint8_t off=0;
		if(inclVoice){
			// first digit is voice #, subsequent two letters are part of short name
			// remember that curParmVal is our index into modTargets, not an actual parameter number
			valueAsText[off++]=(char)(voiceFromModTargValue(curParmVal) + '0');
		}

		const uint8_t name = pgm_read_byte(&modTargets[curParmVal].nameIdx);
		memcpy(&valueAsText[off],shortNames[pgm_read_byte(&valueNames[name].shortName)],(size_t)(3-off));

	}
}

//-----------------------------------------------------------------
// paint the screen when in normal parameter edit mode
// (as opposed to repainting the load save screen)
void menu_repaintGeneric()
{
	//first get the active sub-page and parameter from the menuIndex
	const uint8_t activeParameter	= menuIndex & MASK_PARAMETER; // this will be 0 to 7 for the 4 columns
	const uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT; // this is the active sub page
	uint8_t curParmVal;

	const Page *ap;
	//if in mod matrix, use modMatrixPages instead of menuPages
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		ap=&modMatrixPages[menu_activePage][activePage];
	}
	else
	{
		// the active page
		ap=&menuPages[menu_activePage][activePage];
	}
	if(editModeActive) //show single parameter with full name
	{
		//get address from top1-4 from activeParameter (base adress top1 + offset)
		uint8_t parName = pgm_read_byte(&ap->top1 + activeParameter);
		uint16_t parNr = pgm_read_word(&ap->bot1 + activeParameter);

		curParmVal = activeParameters[parNr];

		// just clear the whole thing
		memset(&editDisplayBuffer[0][0],' ',16);
		memset(&editDisplayBuffer[1][0],' ',16);

#if !EURORACK_MODULE
		if((parameter_dtypes[parNr] & 0xff) == DTYPE_AUTOM_TARGET)
		{
			//**AUTOM this is an index into modTargets now
			//Top row (which destination (1 or 2) and which voice it's targeting)
			memcpy(&editDisplayBuffer[0][0],PSTR("AutDst"),6);
			numtostru(&editDisplayBuffer[0][7],(uint8_t)( parNr - PAR_P1_DEST + 1));

			// bottom row is the category and long name for the parameter being targeted
			if( pgm_read_word(&modTargets[curParmVal].param)==PAR_NONE ) {
				//  OFF
				strcpy(&editDisplayBuffer[1][0], menuText_off);
			} else {
				memcpy(&editDisplayBuffer[0][9],PSTR("Voice"),5);
				//**AUTOM - determine voice number to display for edit mode
				// remember that curParmVal is our index into modTargets, not an actual parameter number
				numtostru(&editDisplayBuffer[0][15], voiceFromModTargValue(curParmVal));
				// display the full mod target
				menu_displayModTargetFull(curParmVal);

			}

		} // parameter type is automation target

		else // parameter type is not automation target
#endif
		{
			//Top row left -> category
			strcpy(&editDisplayBuffer[0][0],
				(const char PROGMEM *)(&catNames[pgm_read_byte(&valueNames[parName].category)]));
			//Top row right -> long name
			strcpy(&editDisplayBuffer[0][8],
				(const char PROGMEM *)(&longNames[pgm_read_byte(&valueNames[parName].longName)]));

			//Bottom row -> parameter value (set below)
			switch(parameter_dtypes[parNr] & 0xFF)
			{
			case DTYPE_TARGET_SELECTION_CV:
			case DTYPE_TARGET_SELECTION_VELO: //switch(parameters_dtypes[parNr] & 0x0F)
			case DTYPE_TARGET_SELECTION_LFO:
				//**LFO **VELO edit mode - display the full target (Cat and long name)
				// curParmVal is an index into modTargets
				menu_displayModTargetFull(curParmVal);
				break;
				/*
			case DTYPE_MIX_FM: //switch(parameters_dtypes[parNr] & 0x0F)
				if(curParmVal == 1)
					memcpy(&editDisplayBuffer[1][13], menuText_mix, 3);
				else
					memcpy(&editDisplayBuffer[1][13],menuText_fm,3);
				break;
				*/
			case DTYPE_ON_OFF: //switch(parameters_dtypes[parNr] & 0x0F)
				if(curParmVal == 1)
					memcpy(&editDisplayBuffer[1][13],menuText_on,3);
				else
					memcpy(&editDisplayBuffer[1][13],menuText_off,3);
				break;

			case DTYPE_MENU: //switch(parameters_dtypes[parNr] & 0x0F)
			{
				//get the used menu (upper 4 bit)
				const uint8_t menuId = (parameter_dtypes[parNr] >> 8);
				// get the value name
				getMenuItemNameForValue(menuId, curParmVal, &editDisplayBuffer[1][13]);
			}
				break; // switch(parameters[parNr].dtype&0x0F) case DTYPE_MENU

			case DTYPE_PM63: //switch(parameters_dtypes[parNr] & 0x0F)
				// -63 to +63
				if( (parNr == PAR_COARSE1) || (parNr == PAR_COARSE2) || (parNr == PAR_COARSE3) || (parNr == PAR_COARSE4) || (parNr == PAR_COARSE5) || (parNr == PAR_COARSE6)  || (parNr == PAR_FM_FREQ1) || (parNr == PAR_FM_FREQ2) || (parNr == PAR_FM_FREQ3) || (parNr == PAR_MOD_OSC_F1_CYM)  || (parNr == PAR_MOD_OSC_F2_CYM) || (parNr == PAR_MOD_OSC_F1) || (parNr == PAR_MOD_OSC_F2) )
				{
					numtostrps(&editDisplayBuffer[1][13],(int8_t)(curParmVal - 60));
				}
				else
				{
					numtostrps(&editDisplayBuffer[1][13],(int8_t)(curParmVal - 63));
				}

				break;

			case DTYPE_PM100:
				numtostrps(&editDisplayBuffer[1][13],(int8_t)(curParmVal - 100));
				break;

			case DTYPE_NOTE_NAME: //switch(parameters_dtypes[parNr] & 0x0F)
				// note names TODO we could create another dtype but we don't want to waste them
				if(parNr >= PAR_MIDI_NOTE1 && parNr <= PAR_MIDI_NOTE7 && curParmVal==0)
					memcpy(&editDisplayBuffer[1][13],menuText_any,3);
				else
					setNoteName(curParmVal, &editDisplayBuffer[1][13]);
				break;

			case DTYPE_0b1: // the only 0/1 is automation track, make it look 1 based
				numtostrpu(&editDisplayBuffer[1][13],(uint8_t)(curParmVal+1),' ');
				break;
			default: //switch(parameters_dtypes[parNr] & 0x0F)
				// unsigned numeric value: just print the value
			case DTYPE_0B127:
			case DTYPE_0B100:
			case DTYPE_0B255:
			case DTYPE_1B16:
			case DTYPE_0B15:
			case DTYPE_1B64:
			case DTYPE_VOICE_LFO:

				if(parNr == PAR_BPM)
				{
					if(curParmVal == 0)
					{
						editDisplayBuffer[1][13] = 'E';
						editDisplayBuffer[1][14] = 'X';
						editDisplayBuffer[1][15] = 'T';
					}
					else
					{
						numtostrpu(&editDisplayBuffer[1][13],(uint8_t)(curParmVal),' ');
					}
				}
				if(parNr == PAR_MORPH_TARGET)
				{
					memcpy(&editDisplayBuffer[1][0], savefileNames[curParmVal][KIT_NAMES], 8);
					numtostrpu(&editDisplayBuffer[1][13],(uint8_t)(curParmVal),' ');
				}
				else
				{
					numtostrpu(&editDisplayBuffer[1][13],(uint8_t)(curParmVal),' ');
				}
				break;


			} //switch(parameters_dtypes[parNr] & 0x0F) end

		} // parameter type is not automation target

	} // if editmode active
	else
	{ // editmode not active - show regular menu parameters

		//check if parameters 1-4 or 5-8 are shown
		const uint8_t is2ndPage = (activeParameter>3) * 4;

		// top texts
		memcpy(&editDisplayBuffer[0][0],
			&shortNames[pgm_read_byte(&valueNames[pgm_read_byte(&ap->top1+is2ndPage)].shortName)],3);
		memcpy(&editDisplayBuffer[0][4],
			&shortNames[pgm_read_byte(&valueNames[pgm_read_byte(&ap->top2+is2ndPage)].shortName)],3);
		memcpy(&editDisplayBuffer[0][8],
			&shortNames[pgm_read_byte(&valueNames[pgm_read_byte(&ap->top3+is2ndPage)].shortName)],3);
		memcpy(&editDisplayBuffer[0][12],
			&shortNames[pgm_read_byte(&valueNames[pgm_read_byte(&ap->top4+is2ndPage)].shortName)],3);

		// capitalize selected item
		upr_three(&editDisplayBuffer[0][(activeParameter%4)*4]);

		//check if scroll sign needs to be shown
		uint8_t showScrollSign = checkScrollSign(activePage, activeParameter);
		editDisplayBuffer[0][15] = showScrollSign;

		// bottom values

		// some place to store the parameter value as text
		char valueAsText[4];

		for(uint8_t i=0;i<4;i++) { // for each of the four columns of values
			const uint16_t parNr = pgm_read_word(&ap->bot1 + i +is2ndPage);

			curParmVal = activeParameters[parNr];

			//convert the parameter uint8_t value to a 3 place char
			if(parNr==PAR_NONE) {
				memcpy(valueAsText,menuText_blank,3);
			} else { //not if(parNr==PAR_NONE)

				switch(parameter_dtypes[parNr] & 0xFF) {
				case DTYPE_TARGET_SELECTION_CV:
				case DTYPE_TARGET_SELECTION_VELO: //switch(parameters[parNr].dtype&0x0F)
				case DTYPE_TARGET_SELECTION_LFO: //switch(parameters[parNr].dtype&0x0F)
					//**LFO **VELO non-edit mode display short name with no numeric prefix
					menu_displayModTargetShort(curParmVal,valueAsText,0 /*dont include voice number */);
					break;
					/*
				case DTYPE_AUTOM_TARGET: //switch(parameters[parNr].dtype&0x0F)
					//AUTOM - determine the name to display for non-edit mode
					menu_displayModTargetShort(curParmVal,valueAsText,1 //include voice number );
					break;
					*/
				case DTYPE_PM63: //switch(parameters[parNr].dtype&0x0F)
					// -63 to 64
					if( (parNr == PAR_COARSE1) || (parNr == PAR_COARSE2) || (parNr == PAR_COARSE3) || (parNr == PAR_COARSE4) || (parNr == PAR_COARSE5) || (parNr == PAR_COARSE6)  || (parNr == PAR_FM_FREQ1) || (parNr == PAR_FM_FREQ2) || (parNr == PAR_FM_FREQ3) || (parNr == PAR_MOD_OSC_F1_CYM)  || (parNr == PAR_MOD_OSC_F2_CYM) || (parNr == PAR_MOD_OSC_F1) || (parNr == PAR_MOD_OSC_F2) )
					{
						numtostrps(valueAsText,(int8_t)(curParmVal - 60));
					}
					else
					{
						numtostrps(valueAsText,(int8_t)(curParmVal - 63));
					}
					break;

				case DTYPE_PM100:
					numtostrps(valueAsText,(int8_t)(curParmVal - 100));
					break;

				case DTYPE_NOTE_NAME: //switch(parameters[parNr].dtype&0x0F)
					// note names
					if(parNr >= PAR_MIDI_NOTE1 && parNr <= PAR_MIDI_NOTE7 && curParmVal==0)
						// for the voice note setting, the value 0 designates Any, whereas for the
						// step note name it does not. (Any means that the step note value is sent, as opposed to
						// the voice note being sent for all triggered notes)
						memcpy(valueAsText,menuText_any,3);
					else
						setNoteName(curParmVal, valueAsText);
					break;
/*
				case DTYPE_MIX_FM: //switch(parameters[parNr].dtype&0x0F)
					if(curParmVal == 1)
						memcpy(valueAsText,menuText_mix,3);
					else
						memcpy(valueAsText,menuText_fm,3);
					break;
					*/
				case DTYPE_ON_OFF: //switch(parameters[parNr].dtype&0x0F)
					if(curParmVal == 1)
						memcpy(valueAsText,menuText_on,3);
					else
						memcpy(valueAsText,menuText_off,3);
					break;
				case DTYPE_MENU: //switch(parameters[parNr].dtype&0x0F)
				{
					//get the used menu (upper 4 bit)
					const uint8_t menuId = parameter_dtypes[parNr] >> 8;
					getMenuItemNameForValue(menuId, curParmVal, valueAsText);
				}
					break;

				case DTYPE_0b1: // switch(parameters[parNr].dtype&0x0F)
					//automation track 0/1 value, make it look 1 based
					numtostrpu(valueAsText,(uint8_t)(curParmVal+1),' ');
					break;
				default: //switch(parameters[parNr].dtype&0x0F)
				case DTYPE_0B127:
				case DTYPE_0B100:
				case DTYPE_0B255:
				case DTYPE_1B16:
				case DTYPE_0B15:
				case DTYPE_1B64:
				case DTYPE_VOICE_LFO:
					if(parNr == PAR_BPM)
					{
						if(curParmVal == 0)
						{
							valueAsText[0] = 'E';
							valueAsText[1] = 'X';
							valueAsText[2] = 'T';
						}
						else
						{
							numtostrpu(valueAsText,curParmVal,' ');
						}
					}
					else
					{
						// fallthrough for the rest of the unsigned values
						numtostrpu(valueAsText,curParmVal,' ');
					}
					break;


				} //switch(parameters[parNr].dtype&0x0F) end

			} // if(parNr==PAR_NONE)

			//write the text to the edit buffer
			memcpy(&editDisplayBuffer[1][4*i],valueAsText,3);

		}	// for 1 to 4 .. each menu item
	} // if editmode not active
}

//-----------------------------------------------------------------
void menu_debugPrint(char* c)
{
	// just clear the whole thing
	memset(&editDisplayBuffer[0][0],' ',16);
	memset(&editDisplayBuffer[1][0],' ',16);

	//Top row left -> category
	strcpy(&editDisplayBuffer[0][0], c);


	display_needs_refresh = 1;
}
//-----------------------------------------------------------------
// repaint the screen
void menu_repaint()
{
	if(screensaver_Active) return;

	visibleCursor = 0;
	if(menu_activePage == LOAD_PAGE || menu_activePage==SAVE_PAGE)
	{
		//this is a special case because the load/save page differs from all the other pages
		menu_repaintLoadSavePage();

	}
	else if((menu_activePage >= PATTERN_LOAD_PAGE  && menu_activePage <= SONG_MODE_PAGE))
	{
		menu_repaintSimplePage();
	}
	else if(buttonHandler_getDrumButton())
	{
		strcpy(&editDisplayBuffer[0][0], "Selected Drum ");

		strcpy(&editDisplayBuffer[1][0], "Voice: ");
		switch(menu_activeVoice)
		{
		case 0:
		case 1:
		case 2:
			sprintf(&editDisplayBuffer[1][7], "Drum %d", menu_activeVoice+1);
			break;

		case 3:
			sprintf(&editDisplayBuffer[1][7], "Snare");
			break;

		case 4:
			sprintf(&editDisplayBuffer[1][7], "Clp/Cym");
			break;

		case 5:
			sprintf(&editDisplayBuffer[1][7], "Cl HH");
			break;

		case 6:
			sprintf(&editDisplayBuffer[1][7], "Op HH");
			break;

		default:
			numtostrps(&editDisplayBuffer[1][7],menu_activeVoice+1);
			break;

		}

	}
	else {
		menu_repaintGeneric();
	}

	//now send the changes from the edit buffer to the display
	//sendDisplayBuffer();
	display_needs_refresh = 1;
};
//-----------------------------------------------------------------
char toUpperCase(char c)
{
	if(c >= 27 && c <= 52) // Lower case letters
	{
		return c-26;
	}
	return c;
}
//-----------------------------------------------------------------
char toLowerCase(char c)
{
	if(c >= 1 && c <= 26) // upper case letters
	{
		return c+26;
	}
	return c;
}
//-----------------------------------------------------------------
void menu_handleSaveScreenKnobValue(uint8_t potNr, uint8_t value)
{
	uint8_t x;
	const uint8_t range = 127;



	if(menu_saveOptions.state >=SAVE_STATE_EDIT_NAME1 && menu_saveOptions.state <=SAVE_STATE_EDIT_NAME8)
	{
		switch(potNr) {
		// move cursor
		case 0: //switch(potNr)
			x = value/(range/7); //0-7
			const uint8_t newState = (uint8_t)((SAVE_STATE_EDIT_NAME1 + x) & 0x0F);
			if(newState != menu_saveOptions.state)
			{
				menu_saveOptions.state = newState;
				//force complete repaint
				menu_repaintAll();
			}
			break;

			// select lower,upper,number
		case 1://switch(potNr)
			x = value > (range/2); //0-1
			switch(x)
			{
			case 0: //Upper Case
#if 0
				savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] =
					(char)toupper(savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]);
#else
				nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = 	(char)toUpperCase(nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]);
#endif
				break;

			case 1: //lower case
#if 0
				savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] =
					(char)tolower(savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]);
#else
				nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = 	(char)toLowerCase(nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]);
#endif
				break;
			}
			menu_repaint();
			break;

			// scroll through letters
		case 2: //switch(potNr)

#if 0
			    //32 to 127 => default ascii range numbers/letters
				x = (uint8_t)(value/(range/(127-32.f)) + 32); //32 - 127
				savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = x;
#else
				//0 to NUM_CHARS
				x = (uint8_t)((value/(float)range) * (NUM_CHARS-1));
				nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = x;// characterOrder[x];

#endif

				menu_repaint();
				break;

				//nothing
		case 3: //switch(potNr)
		default://switch(potNr)
			break;
		} //switch(potNr)
	}		

}
//-----------------------------------------------------------------
void menu_showSdError(uint8_t res)
{

#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#else

#endif

	lcd_process_display_interrupt = 0;
	while(sendDisplayBuffer() != 0);

	lcd_clear();
	lcd_home();
	lcd_string(PSTR("SD Card error:"));
	lcd_setcursor(0,2);
	switch(res)
	{
		// There is no valid FAT volume on the drive or wrong lower layer implementation.
		case FR_NO_FILESYSTEM:
			lcd_string("No FS found!");
			break;

		// The lower layer, disk_read, disk_write or disk_ioctl function, reported that an unrecoverable hard error occured.
		case FR_DISK_ERR:

			lcd_string("Disk error!");
			break;

		/*The lower layer, disk_initialize function, reported that the storage device could not be got ready to work. One of the following possibilities is suspected.

		No medium in the drive.
		Wrong lower layer implementation.
		Wrong hardware configuration.
		The storage device has broken.
		*/
		case FR_NOT_READY:
			lcd_string("No Card!");
			break;

		case FR_NO_FILE:
			lcd_string("File not found!");
			break;

		default:
			lcd_string("unknown!");
			break;

	}
	HAL_Delay(2000);
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Stop_IT(&htim2);	//calc sample interrupt - prio 1
#else

#endif
	lcd_process_display_interrupt = 1;
}
//-----------------------------------------------------------------
void menu_handleLoadSaveMenu(int8_t inc, uint8_t btnClicked)
{
	//this is a special case because the load/save page differs from all the other pages
	// when this is called, edit mode will have already been set or cleared. this is only
	// called when something has changed

	//---- handle the button ----
	if(btnClicked) {
		//if the ok button is active or we are in global save and ok is active, save/load the preset on click
	//	if( (editModeActive && menu_saveOptions.state == SAVE_STATE_OK) ||
		//	(menu_saveOptions.category >= SAVE_TYPE_GLO && menu_saveOptions.state >= SAVE_STATE_EDIT_PRESET_NR) ) {


		// Save Page
		//----------
		//
		// click confirms slot
		// -> change name? (y/N)
		// ...

		//Load Page
		//----------
		//
		// click confirms load/save

			if(menu_activePage == SAVE_PAGE) {

				/**< 0=edit kit/pat, 1=edit preset nr, 2-9 = edit name, 10=ok*/
				switch(menu_saveOptions.state)
				{
				case SAVE_STATE_EDIT_PRESET_NR:
					//click -> change name [y/N]
					if(preset_isNameEmpty(savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer]))
					{
						menu_saveOptions.state = SAVE_STATE_EDIT_NAME1;
						sprintf(savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer], "Unnamed");
					}
					else
					{
						menu_saveOptions.state = SAVE_STATE_CHANGE_NAME_DIALOG_N;
					}
					break;

				case SAVE_STATE_OK:
				case SAVE_STATE_CHANGE_NAME_DIALOG_N:
					//click -> save

					copyEditBufferToName(menu_selectedPresetNr[menu_saveOptions.category] ,menu_saveOptions.nameBuffer );
					switch(menu_saveOptions.category)
					{
#if !EURORACK_MODULE
						case SAVE_TYPE_SONG:
							//nothing to do here
							break;
						case SAVE_TYPE_PATTERN:
							preset_savePatternRAM(menu_selectedPresetNr[SAVE_TYPE_PATTERN]);
							break;
#endif
						case SAVE_TYPE_KIT:
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#endif
							preset_saveDrumsetRam(menu_selectedPresetNr[SAVE_TYPE_KIT],0);
#if EURORACK_MODULE
							preset_saveDrumsetSD(menu_selectedPresetNr[SAVE_TYPE_KIT],0);
#endif
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Stop_IT(&htim2);	//calc sample interrupt - prio 1
#endif

							menu_loadedPresetNr[CURRENT_KIT] = menu_selectedPresetNr[SAVE_TYPE_KIT];
							menu_activePage = KIT_SAVE_PAGE_OK;
							sucessTimer = systick_ticks;
							menu_repaint();

							break;

						case SAVE_TYPE_MORPH:
							preset_saveDrumsetRam(menu_selectedPresetNr[SAVE_TYPE_MORPH],1);
							preset_saveDrumsetSD(menu_selectedPresetNr[SAVE_TYPE_MORPH],1);

							//menu_loadedPresetNr[CURRENT_KIT] = menu_selectedPresetNr[SAVE_TYPE_KIT];
							menu_activePage = KIT_SAVE_PAGE_OK;
							sucessTimer = systick_ticks;
							menu_repaint();
							break;

						case SAVE_TYPE_GLO:
						{
							FRESULT res;
							res = preset_saveGlobals();
							if(res != FR_OK) menu_showSdError(res);
						}
							break;
#if !EURORACK_MODULE
						case SAVE_TYPE_PROJECT:
						{
							FRESULT res;
							if(!seq_isRunning())
							{
								HAL_TIM_Base_Stop(&htim2);	//calc sample interrupt - prio 1
							}

							res = preset_saveProject(menu_selectedPresetNr[SAVE_TYPE_PROJECT]);
							if(!seq_isRunning())
							{
								  HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
							}

							if(res != FR_OK) menu_showSdError(res);
						}
							break;


						case SAVE_TYPE_ALL:
							//preset_saveProject(menu_currentPresetNr[SAVE_TYPE_ALL],1);
							break;
#endif
					}
					menu_resetSaveParameters();
					break;

				case SAVE_STATE_CHANGE_NAME_DIALOG_Y:
#if !EURORACK_MODULE
					if(menu_saveOptions.category == SAVE_TYPE_PATTERN)
					{
						for(uint8_t i=0;i<NUM_PATTERN;i++)
						{
							preset_savePatternRAM(i);

						}
						menu_activePage = PATTERN_SAVE_PAGE_OK;
						sucessTimer = systick_ticks;
						menu_repaint();
					}
					else
#endif
					{
						//click -> edit name
						menu_saveOptions.state = SAVE_STATE_EDIT_NAME1;
						//copy current preset name to target slot
						uint8_t type = NUM_CURRENT;
						switch(menu_saveOptions.category)
						{
							case SAVE_TYPE_KIT:
								type = CURRENT_KIT;
								break;

#if !EURORACK_MODULE
							case SAVE_TYPE_SONG:
								type = CURRENT_SONG;
								break;
#endif
							case SAVE_TYPE_MORPH:
								type = CURRENT_MORPH;
								break;
#if !EURORACK_MODULE
							case SAVE_TYPE_PROJECT:
								type = CURRENT_PROJECT;
								break;
#endif

							default:
#if !EURORACK_MODULE
							case SAVE_TYPE_PATTERN:
							case SAVE_TYPE_ALL:
#endif
							case SAVE_TYPE_GLO:

								break;
						}
						if(type != NUM_CURRENT)
						{
							//TODO: check naming here!
							copyNameToEditBuffer(menu_loadedPresetNr[type],menu_saveOptions.nameBuffer );
							//memcpy((void*)savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer], (const void*)savefileNames[menu_loadedPresetNr[type]][menu_saveOptions.nameBuffer],8);
						}
					}
					break;
				case SAVE_STATE_EDIT_NAME1:
				case SAVE_STATE_EDIT_NAME2:
				case SAVE_STATE_EDIT_NAME3:
				case SAVE_STATE_EDIT_NAME4:
				case SAVE_STATE_EDIT_NAME5:
				case SAVE_STATE_EDIT_NAME6:
				case SAVE_STATE_EDIT_NAME7:
				case SAVE_STATE_EDIT_NAME8:
					break;

				}



			} else { //menu_activePage != SAVE_PAGE

				//load page 
				switch(menu_saveOptions.category) {

				case SAVE_TYPE_KIT:
#if 1
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#endif
					preset_loadDrumsetRam(menu_selectedPresetNr[menu_saveOptions.category],0, 0);
					menu_TargetVoiceGapIndex = getModTargetGapIndex(activeParameters[PAR_TARGET_LFO1 + menu_activeVoice]);

					menu_loadedPresetNr[CURRENT_KIT] = menu_selectedPresetNr[menu_saveOptions.category];

					preset_touchTimer(PRESET_KIT_TIMER);


#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Stop_IT(&htim2);	//calc sample interrupt - prio 1
#endif
					//store selected kit in current pattern
#if USE_PATTERN_EDIT_BUFFER
					patternBackupBuffer.kitNr = menu_loadedPresetNr[CURRENT_KIT];

#else
#if !EURORACK_MODULE
					seq_patternSet.kitNr[seq_activePattern] = menu_loadedPresetNr[CURRENT_KIT];
#endif
#endif

#else
					preset_loadDrumset(menu_selectedPresetNr[menu_saveOptions.category],0);
					// **LFOTARGFIX - save the gap index
					menu_TargetVoiceGapIndex = getModTargetGapIndex(activeParameters[PAR_TARGET_LFO1 + menu_activeVoice]);
#endif
					break;

				case SAVE_TYPE_MORPH:
#if 1
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#endif
					preset_loadDrumsetRam(menu_selectedPresetNr[menu_saveOptions.category],1, 0);
					activeParameters[PAR_MORPH_KIT_DEST] = menu_selectedPresetNr[menu_saveOptions.category];
					menu_loadedPresetNr[CURRENT_MORPH] = menu_selectedPresetNr[menu_saveOptions.category];
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Stop_IT(&htim2);	//calc sample interrupt - prio 1
#endif
#else
					//load to morph buffer
					preset_loadDrumset(menu_selectedPresetNr[menu_saveOptions.category],1);
#endif
					break;
#if !EURORACK_MODULE
				case SAVE_TYPE_SONG:
					//just set pointer to other song
					seq_activeSong = seq_songData[menu_selectedPresetNr[menu_saveOptions.category]];
					menu_loadedPresetNr[CURRENT_SONG] = menu_selectedPresetNr[menu_saveOptions.category];
					break;
#endif
#if !EURORACK_MODULE
				case SAVE_TYPE_PATTERN:
					//if(preset_loadPatternSD(menu_currentPresetNr[SAVE_TYPE_PATTERN]))
						//menu_resetSaveParameters();

					switch(menu_saveOptions.state)
					{
						case SAVE_STATE_CHANGE_NAME_DIALOG_Y:
							for(uint8_t i=0;i<NUM_PATTERN;i++)
							{
								preset_loadPatternRAM(i);
								menu_resetSaveParameters();
							}
							//editModeActive = 1; //to prevent confirmation encoder click from setting edit mode active (toggle)
							//buttonHandler_buttonPressed(BUT_MODE1); //return to main menu
							menu_saveOptions.state = SAVE_STATE_CHANGE_NAME_DIALOG_N;
							break;

						default:
						case SAVE_STATE_CHANGE_NAME_DIALOG_N:
							break;
					}
					break;
#endif


#if !EURORACK_MODULE
				case SAVE_TYPE_PROJECT:
				{

					FRESULT res;


					res = preset_loadProject(menu_selectedPresetNr[SAVE_TYPE_PROJECT]);

					if(res == FR_NO_FILE)
					{
						res = preset_createProject(menu_selectedPresetNr[SAVE_TYPE_PROJECT]);
					}

					if(res != FR_OK)
					{
						menu_showSdError(res);
					}
					else
					{
						menu_loadedPresetNr[CURRENT_PROJECT] = menu_selectedPresetNr[SAVE_TYPE_PROJECT];
						menu_resetSaveParameters();
					}
				}
					break;
#endif
/*
				case SAVE_TYPE_ALL:
					preset_loadAll(menu_currentPresetNr[SAVE_TYPE_ALL],1);
					menu_resetSaveParameters();
					break;
*/
				case SAVE_TYPE_GLO:
					preset_loadGlobals();
					menu_resetSaveParameters();						
					break;
#if USE_SAMPLES
				case SAVE_TYPE_SAMPLES:

					//spi_deInit();
					//send load sample command to mainboard
					frontPanel_sendData(SAMPLE_CC,SAMPLE_START_UPLOAD,0x00);

					//Display load message
					lcd_clear();
					lcd_home();
					lcd_string(PSTR("Sample upload"));
					lcd_setcursor(0,2);
					lcd_string(PSTR("Started"));
					//wait for ack
					{
						uint8_t ret = uart_waitAck();
						if(ret == ACK)
						{

						}
						else
						{

						}
					}
					//re-initialize SD-Card
					preset_init();
					//redraw screen
					// menu_repaintAll(); screen will be repainted later, relax!

					frontPanel_sendData(SAMPLE_CC,SAMPLE_COUNT,0x00);

					break;
#endif

				default:
					break;
				}
			} // menu_activePage == or != SAVE_PAGE
		//} // if ok button active
	} // btnClicked

	//if not in name edit mode, always set edit mode active true. (is this a dirty hack?!?)
	if(!((menu_saveOptions.state >= SAVE_STATE_EDIT_NAME1) && (menu_saveOptions.state <= SAVE_STATE_OK))) editModeActive = 1;


//####### handle the encoder #######
	if(editModeActive)
	{
		//encoder changes value
		///**< 0=edit kit/pat, 1=edit preset nr, 2 = edit name*/
		switch(menu_saveOptions.state) {

/*
		case 0: //switch(menu_saveOptions.state) - edit kit/Pat

			//No more category change with encoder or pots


			if(inc<0) {
				if(menu_saveOptions.category!=0) {
					menu_saveOptions.category--;
				}
			} else if(inc>0) {
				if(menu_saveOptions.category < NUM_SAVE_TYPES-1) {
					menu_saveOptions.category++;
				}
			}

			preset_loadName(menu_currentPresetNr[menu_saveOptions.category],menu_saveOptions.category);

			break;
			*/

		case SAVE_STATE_EDIT_PRESET_NR: //switch(menu_saveOptions.state) - edit preset nr
			if(inc<0) {
				if(menu_selectedPresetNr[menu_saveOptions.category]+inc>=0)	{
					DISABLE_CONV_WARNING
					menu_selectedPresetNr[menu_saveOptions.category] += inc;
					END_DISABLE_CONV_WARNING
				}
			} else if(inc>0) {
				if(menu_selectedPresetNr[menu_saveOptions.category]<NUM_KIT_SLOTS-1) {
					DISABLE_CONV_WARNING
					menu_selectedPresetNr[menu_saveOptions.category] += inc;
					END_DISABLE_CONV_WARNING
				}
			}
			//if on load page, load the new preset when the preset number is changed
			//V2 change -> encoder click needed to confirm!
			if((inc!=0)) {

				// always load the name regardless of what else
				//preset_loadName(menu_currentPresetNr[menu_saveOptions.category], menu_saveOptions.nameBuffer, menu_saveOptions.category);

				if(menu_activePage == LOAD_PAGE) {
					// if loading and type is kit and morph do an insta-load
					switch(menu_saveOptions.category) {
					case SAVE_TYPE_KIT:

						break;
					case SAVE_TYPE_MORPH:

						break;
					}
				}
			}
			copyNameToEditBuffer(menu_selectedPresetNr[menu_saveOptions.category],menu_saveOptions.nameBuffer );
			break; //switch(menu_saveOptions.state) - edit preset nr
		case SAVE_STATE_CHANGE_NAME_DIALOG_N:
		case SAVE_STATE_CHANGE_NAME_DIALOG_Y:
			if(inc<0)
			{
				if(menu_saveOptions.state > SAVE_STATE_CHANGE_NAME_DIALOG_Y) menu_saveOptions.state--;

			}
			else if(inc>0)
			{
				if(menu_saveOptions.state < SAVE_STATE_CHANGE_NAME_DIALOG_N) menu_saveOptions.state++;

			}
			break;
		//edit name
		case SAVE_STATE_EDIT_NAME1: // switch(menu_saveOptions.state) char 1
		case SAVE_STATE_EDIT_NAME2: // switch(menu_saveOptions.state) char 2
		case SAVE_STATE_EDIT_NAME3: // switch(menu_saveOptions.state) char 3
		case SAVE_STATE_EDIT_NAME4: // switch(menu_saveOptions.state) char 4
		case SAVE_STATE_EDIT_NAME5: // switch(menu_saveOptions.state) char 5
		case SAVE_STATE_EDIT_NAME6: // switch(menu_saveOptions.state) char 6
		case SAVE_STATE_EDIT_NAME7: // switch(menu_saveOptions.state) char 7
		case SAVE_STATE_EDIT_NAME8: // switch(menu_saveOptions.state) char 8
			DISABLE_CONV_WARNING
#if 0
			savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] += inc;
#else
			if(inc < 0)
			{
				if(nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] != 0)
				nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]	+= inc;
			}
			else if (nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] < NUM_CHARS-1)
			{
				nameEditBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1]	+= inc;
			}
#endif

#if 0
			//32 to 127 => default ascii range numbers/letters
			if( savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] < 32) savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = 32;
			else if( savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] > 127) savefileNames[menu_selectedPresetNr[menu_saveOptions.category]][menu_saveOptions.nameBuffer][menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] = 127;
#else

				//0 to NUM_CHARS


#endif


			//presetNameBuffer[menu_saveOptions.state - SAVE_STATE_EDIT_NAME1] += inc;
			END_DISABLE_CONV_WARNING
			break;

		case SAVE_STATE_OK:

			break;

		default: //switch(menu_saveOptions.state)
			break;
		} //switch(menu_saveOptions.state)
	}

	else { //editModeActive is false


		//encoder selects value to change
		if(inc<0) {
			//encoder no longer selects category/type
			if(menu_saveOptions.state > SAVE_STATE_EDIT_NAME1) {
				menu_saveOptions.state --;
				//no name edit on load page
				if(menu_activePage == LOAD_PAGE && menu_saveOptions.state >= SAVE_STATE_EDIT_NAME1) {
					menu_saveOptions.state = SAVE_STATE_EDIT_PRESET_NR;
				}
			}
		} else if(inc>0) {
			if(menu_saveOptions.state < SAVE_STATE_OK) {
				menu_saveOptions.state ++;

				if(menu_activePage == LOAD_PAGE)
				{
					//no name edit on load page
					if(menu_saveOptions.state >= SAVE_STATE_EDIT_NAME1 ) {
						menu_saveOptions.state = SAVE_STATE_OK;
					}

					/*
					// ok button doesnt exist on kit and morph
					if(menu_saveOptions.state == SAVE_STATE_OK ) {
						if(menu_saveOptions.category == SAVE_TYPE_KIT ||
							menu_saveOptions.category == SAVE_TYPE_MORPH)
							menu_saveOptions.state = SAVE_STATE_EDIT_PRESET_NR;
					}
					*/

					//ok button only for load pattern
					//if( (menu_saveOptions.what != SAVE_TYPE_PATTERN) &&
					//	(menu_saveOptions.what >= SAVE_TYPE_GLO)  && menu_saveOptions.state == SAVE_STATE_OK ) {
					//	menu_saveOptions.state = SAVE_STATE_EDIT_PRESET_NR;
					//}
				}
			}
		} // inc<0

	} //editModeActive

}

#if !EURORACK_MODULE
void menu_handleSongMenu(int8_t inc, uint8_t btnClicked)
{
	if(editModeActive)
	{
#define SONG_END MAX_PATTERN
#define SONG_LOOP SONG_END+1
		if(inc>0)
		{
			 if(seq_activeSong[menu_songMenuPosition] < SONG_LOOP) seq_activeSong[menu_songMenuPosition]++;
		}
		else if(inc<0)
		{
			if(seq_activeSong[menu_songMenuPosition] > 0)seq_activeSong[menu_songMenuPosition]--;
		}
	}
	else
	{
		if(inc>0)
		{
			if(menu_songMenuPosition > MAX_SONG_LENGTH) menu_songMenuPosition = 0;
			if(menu_songMenuPosition < (MAX_SONG_LENGTH-1) ) menu_songMenuPosition++;
		}
		else if(inc<0)
		{
			if(menu_songMenuPosition > 0) menu_songMenuPosition--;
		}
	}
	//repaint handled in base encoder function
	//if(btnClicked) menu_repaintAll();
	//else menu_repaint();

}
#endif
// given a menu id returns the number of entries
static uint8_t getMaxEntriesForMenu(uint8_t menuId)
{
	switch(menuId) // clever GCC. no need to use pgm_read_byte
	{
	case MENU_TRANS:
		return transientNames[0][0];
	case MENU_AUDIO_OUT:
		return outputNames[0][0];
	case MENU_AUDIO_OUT_FX:
		return outputNamesFx[0][0];
	case MENU_FILTER:
		return filterTypes[0][0];
	case MENU_SYNC_RATES:
		return syncRateNames[0][0];
	case MENU_LFO_WAVES:
		return lfoWaveNames[0][0];
	case MENU_RETRIGGER:
		return retriggerNames[0][0];
	case MENU_FOLLOW:
		return followNames[0][0];
		break;
	case MENU_SEQ_QUANT:
		return quantisationNames[0][0];
	case MENU_MIDI:
		return midiModes[0][0];
	case MENU_NEXT_PATTERN:
		return nextPatternNames[0][0];
	case MENU_WAVEFORM:
		return (uint8_t)(waveformNames[0][0] + menu_numSamples);
	case MENU_ROLL_RATES:
		return rollRateNames[0][0];
	case MENU_MIDI_ROUTING:
		return midiRoutingNames[0][0];
	case MENU_MIDI_FILTERING:
		return midiFilterNames[0][0];
	case MENU_PPQ_OUT:
		return ppqOutNames[0][0];
	case MENU_PPQ_IN:
		return ppqInNames[0][0];
	case MENU_MIX_FM:
		return mixFmNames[0][0];

	case MENU_FX:
		return fxNames[0][0];

	case MENU_DIST:
		return distNames[0][0];

	case MENU_RATIO:
		return ratioNames[0][0];

	case MENU_DELAY:
		return delayNames[0][0];

	case MENU_REVERB:
		return reverbNames[0][0];

	case MENU_PAT_MODE:
		return patModeNames[0][0];

	case MENU_STEP_FLAM:
		return stepFlamNames[0][0];

	case MENU_STEP_SHIFT:
			return stepShiftNames[0][0];

	case MENU_CV_SRC:
			return cvSrcNames[0][0];

	case MENU_CV_SAVE:
		return cvSaveNames[0][0];

	default:
		return 0;
	}
}

// given a menu id, and a value returns the name from PROGMEM
static void getMenuItemNameForValue(const uint8_t menuId, const uint8_t curParmVal, char *buf)
{
	const void *p=0;
	switch(menuId)
	{
	case MENU_TRANS:
		p = transientNames[curParmVal + 1];
		break;
	case MENU_AUDIO_OUT:
		p = outputNames[curParmVal + 1];
		break;
	case MENU_AUDIO_OUT_FX:
		p = outputNamesFx[curParmVal + 1];
		break;
	case MENU_FILTER:
		p = filterTypes[curParmVal + 1];
		break;
	case MENU_WAVEFORM:
		if(curParmVal < waveformNames[0][0])
			p=waveformNames[curParmVal + 1];
		else
		{
			buf[0]='s';
			buf[2]=' ';
			numtostru(&buf[1],(uint8_t)(curParmVal - waveformNames[0][0]));
			return;
		}
		break;

	case MENU_SYNC_RATES:
		p=syncRateNames[curParmVal + 1];
		break;
	case MENU_LFO_WAVES:
		p=lfoWaveNames[curParmVal+1];
		break;
	case MENU_RETRIGGER:
		p=retriggerNames[curParmVal+1];
		break;
	case MENU_FOLLOW:
		p=followNames[curParmVal+1];
		break;
	case MENU_SEQ_QUANT:
		p=quantisationNames[curParmVal+1];
		break;
	case MENU_MIDI:
		p=midiModes[curParmVal+1];
		break;
	case MENU_NEXT_PATTERN:
		p=nextPatternNames[curParmVal+1];
		break;
	case MENU_ROLL_RATES:
		p=rollRateNames[curParmVal+1];
		break;
	case MENU_MIDI_ROUTING:
		p=midiRoutingNames[curParmVal+1];
		break;
	case MENU_MIDI_FILTERING:
		p=midiFilterNames[curParmVal+1];
		break;
	case MENU_PPQ_IN:
		p=ppqInNames[curParmVal+1];
		break;
	case MENU_PPQ_OUT:
		p=ppqOutNames[curParmVal+1];
		break;
	case MENU_MIX_FM:
		p=mixFmNames[curParmVal+1];
		break;

	case MENU_FX:
		p=fxNames[curParmVal+1];
		break;

	case MENU_DIST:
		p=distNames[curParmVal+1];
		break;

	case MENU_RATIO:
		p=ratioNames[curParmVal+1];
		break;

	case MENU_DELAY:
		p=delayNames[curParmVal+1];
		break;

	case MENU_REVERB:
		p=reverbNames[curParmVal+1];
		break;

	case MENU_PAT_MODE:
		p=patModeNames[curParmVal+1];
		break;

	case MENU_STEP_FLAM:
		p=stepFlamNames[curParmVal+1];
		break;

	case MENU_STEP_SHIFT:
		p=stepShiftNames[curParmVal+1];
		break;

	case MENU_CV_SRC:
		p=cvSrcNames[curParmVal+1];
		break;

	case MENU_CV_SAVE:
		p=cvSaveNames[curParmVal+1];
			break;


	default:
		p=menuText_dash;
		break;
	}
	memcpy(buf,p,3);

}

//-----------------------------------------------------------------
// called when either the encoder is rotated, or the encoder button is clicked
// inc - the number of clicks the encoder has moved
// button - 1 if the button is depressed, 0 if its not
void menu_parseEncoder(int8_t inc, uint8_t button)
{
	uint8_t btnClicked=0;
	// handle the button being clicked or released
	if(button != lastEncoderButton) { // was the button clicked?
		btnClicked=button;
		if(btnClicked)
		{

#if !EURORACK_MODULE
			if (copyClear_Mode == MODE_CLEAR)
			{
				//execute
				copyClear_executeClear();
			}
			else
#endif
			{
			// toggle edit mode
				if(!screensaver_Active)
					editModeActive = (uint8_t)(1-editModeActive);
			}
		}
		lastEncoderButton = button;
		if(!screensaver_Active)
			menu_repaintAll();
	} else if(inc==0)
		return; // nothing has changed. do nothing

	screensaver_touch();

	//invert encoder
	//inc = (int8_t)(inc * -1);

	if(menu_activePage == LOAD_PAGE || menu_activePage == SAVE_PAGE) {
		menu_handleLoadSaveMenu(inc, btnClicked);
	}
#if !EURORACK_MODULE
	else if(menu_activePage == SONG_MODE_PAGE)
	{
		menu_handleSongMenu(inc, btnClicked);
	}
#endif
#if !EURORACK_MODULE
	else if (menu_activePage == MENU_LAST_STEP_PAGE)
	{
		uint8_t mode = buttonHandler_getMode();
		uint8_t l = seq_getTrackLength(menu_activeVoice);
		if(inc < 0)
		{
			if(l!=1)
				l += inc;
		}
		else
		{
			l += inc;
			if(l > 64) l = 64;
		}

		if(mode == SELECT_MODE_LAST_STEP_TRACK)
		{
			seq_setTrackLength(menu_activeVoice, l);
		}
		else if (mode == SELECT_MODE_LAST_STEP_ALL)
		{
			for(int i=0;i<7;i++)
			{
				seq_setTrackLength(i, l);
			}
		}

	}
#endif
	else if(inc!=0) {
		//============================= handle encoder change ==============================
#if !EURORACK_MODULE
		if(copyClear_isClearModeActive()) {
			//encoder selects clear target
			uint8_t target = copyClear_getClearTarget();
			if(inc<0) {
				if(target!=0) {
					target--;
				}
			} else if(inc>0) {
				if(target!=CLEAR_AUTOMATION2) {
					target++;
				}
			}
			copyClear_setClearTarget(target);
			return;

		} else
#endif
			if(editModeActive) {
			// edit mode is active so change the value of the current parameter
			menu_encoderChangeParameter(inc);
			//re-enable editMode, since switch sub page turns it off while switching fx menu
			editModeActive=1;
			menu_repaint();
			return;
		} else {
			//edit mode not active so encoder selects active parameter
			menu_moveToMenuItem(inc);
		}

	}
	menu_repaintAll();

}

//-----------------------------------------------------------------
// called when edit mode is active
// encoder controls parameter value
// given a delta, will apply that to the current parameter
static void menu_encoderChangeParameter(int8_t inc)
{

	preset_touchTimer(PRESET_KIT_TIMER);

	const uint8_t activeParameter	= menuIndex & MASK_PARAMETER;
	const uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT;

	//get address from top1-8 from activeParameter (base adress top1 + offset)
	uint16_t paramNr;
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		paramNr		= pgm_read_word(&modMatrixPages[menu_activePage][activePage].bot1 + activeParameter);
	}
	else
	{
		paramNr		= pgm_read_word(&menuPages[menu_activePage][activePage].bot1 + activeParameter);
	}
	//uint8_t *paramValue = &mainParameters[paramNr];
	uint8_t *paramValue;

	paramValue = &activeParameters[paramNr];

	//increase parameter value
	if(inc>0) //positive increase
	{
		if(*paramValue != 255) //omit wrap for 0B255 dtypes
			*paramValue = (uint8_t)(*paramValue + inc);
	}
	else if (inc<0) //neg increase
	{
		if(*paramValue >= abs(inc)) //omit negative wrap. inc can also be -2 or -3 depending on turn speed!
		{
			DISABLE_CONV_WARNING
			*paramValue += inc;
			END_DISABLE_CONV_WARNING
		}
	}

	switch(parameter_dtypes[paramNr] & 0xFF)
	{
	case DTYPE_TARGET_SELECTION_VELO: //parameter_dtypes[paramNr] & 0x0F
	{
		//**VELO encoder value limit to start and end of range for this voice
		// get voice, and figure valid range, translate to param number before sending
		uint8_t voiceNr=(uint8_t)(paramNr - PAR_VEL_DEST_1);
		if(*paramValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
			if(inc < 0) // going down, allow 0
				*paramValue=0;
			else // going up fix to start
				*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		} else if (*paramValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
			*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		}

		// determine the parameter id to send across
		uint8_t value = (uint8_t)pgm_read_word(&modTargets[*paramValue].param);
		uint8_t upper,lower;
		/*
		 *  upper: rightmost bit is 1 if the parameter we are targeting is in the "above 127" range
		 *         next 6 bits are the voice number (0 to 5) of which voice is being dealt with here
		 *  lower: the (0-127) value representing which parameter is being modulated
		 */
		upper = (uint8_t)( (uint8_t)((value&0x80)>>7) | ((voiceNr&0x3f)<<1) );
		lower = value&0x7f;
		frontPanel_sendData(CC_VELO_TARGET,upper,lower);
		//return;
	}
		break;

	case DTYPE_VOICE_LFO://parameter_dtypes[paramNr] & 0x0F
	{
		//**LFO - limit voice number to 1-6 range. determine target selection so we can send it with voice #
		if(*paramValue < 1)
			*paramValue = 1;
		else if(*paramValue > 6)
			*paramValue = 6;

		// **LFOTARGFIX - ensure that the lfo mod target is pointing to the same type of modulation
		// on the new voice
		const uint8_t newTargVal=getModTargetIdxFromGapIdx((uint8_t)(*paramValue-1),menu_TargetVoiceGapIndex);
		// update the lfo mod target
		activeParameters[PAR_TARGET_LFO1 + (paramNr - PAR_VOICE_LFO1)] = newTargVal;

		// determine the real param value given the index into modTargets
		uint8_t value =  (uint8_t)pgm_read_word(&modTargets[newTargVal].param);

		/*  upper: rightmost bit is 1 if the parameter we are targeting is in the "above 127" range
		 *         next 6 bits are the voice number (0 to 5) of which voice is being dealt with here
		 *  lower: the (0-127) value representing which parameter is being modulated
		 */
		uint8_t upper,lower;
		upper = (uint8_t)(((value&0x80)>>7) | ((((uint8_t)(paramNr - PAR_VOICE_LFO1))&0x3f)<<1));
		lower = value&0x7f;
		frontPanel_sendData(CC_LFO_TARGET,upper,lower);
		//return;
	}
		break;
	case DTYPE_TARGET_SELECTION_LFO://parameter_dtypes[paramNr] & 0x0F
	{
		//**LFO - limit encoder start and end to range for the target voice (not the lfo number)
		// this is a value from 1 to 6, so we adjust to be 0 based
		uint8_t voiceNr =  (uint8_t)(activeParameters[PAR_VOICE_LFO1+(paramNr - PAR_TARGET_LFO1)]-1);
		if(*paramValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
			if(inc < 0) // going down, allow 0
				*paramValue=0;
			else // going up fix to start
				*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		} else if (*paramValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
			*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		}

		// **LFOTARGFIX - save the gap index
		menu_TargetVoiceGapIndex = getModTargetGapIndex(*paramValue);

		uint8_t value =  (uint8_t)pgm_read_word(&modTargets[*paramValue].param);
		uint8_t upper,lower;
		upper = (uint8_t)((uint8_t)((value&0x80)>>7) | (((paramNr - PAR_TARGET_LFO1)&0x3f)<<1));
		lower = value&0x7f;
		frontPanel_sendData(CC_LFO_TARGET,upper,lower);
		// fall thru to update display
	}
		break;

	case DTYPE_TARGET_SELECTION_CV:
	{
		//**CV encoder value limit to start and end of range for this voice
		// get voice, and figure valid range, translate to param number before sending
		//uint8_t cvNr=(uint8_t)(paramNr - PAR_CV_DEST_1);
		uint16_t cvNr=(paramNr - PAR_CV_DEST_1);
		uint8_t voiceNr = cvNr / 3;
#if NUM_MOD_SLOTS != 18
#error voice calculation needs to be adapted to  NUM_MOD_SLOTS number!!!
#endif


		if(*paramValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
			if(inc < 0) // going down, allow 0
				*paramValue=0;
			else // going up fix to start
				*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		} else if (*paramValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
			*paramValue = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		}

		// determine the parameter id to send across
		uint8_t value = (uint8_t)pgm_read_word(&modTargets[*paramValue].param);
#if 0
		uint8_t upper,lower;
		/*
		 *  upper: rightmost bit is 1 if the parameter we are targeting is in the "above 127" range
		 *         next 6 bits are the voice number (0 to 5) of which voice is being dealt with here
		 *  lower: the (0-127) value representing which parameter is being modulated
		 */
		upper = (uint8_t)( (uint8_t)((value&0x80)>>7) | ((voiceNr&0x3f)<<1) );
		lower = value&0x7f;
		frontPanel_sendData(CC_VELO_TARGET,upper,lower);
#else
		modNode_setDestination(&cvModulators[cvNr], value);

#endif

		//return;
	}
		break;
/*
	case DTYPE_AUTOM_TARGET: {//parameter_dtypes[paramNr] & 0x0F
		const uint8_t nmt=getNumModTargets();
		//AUTOM - limit to valid range for encoder
		if(*paramValue >= nmt)
			*paramValue = (uint8_t)(nmt-1);
		break;

	}
*/

	case DTYPE_0B255:
		//if(*paramValue > 255)
		//	*paramValue = 255;
		break;



	case DTYPE_1B16://parameter_dtypes[paramNr] & 0x0F
		if(*paramValue < 1)
			*paramValue = 1;
		else if(*paramValue > 16)
			*paramValue = 16;
		break;



	case DTYPE_1B64:
		if(*paramValue < 1)
			*paramValue = 1;
		else if(*paramValue > 64)
			*paramValue = 64;
		break;
		break;
	case DTYPE_0B15:
		if(*paramValue>15)
			*paramValue = 15;
		break;
	//case DTYPE_MIX_FM://parameter_dtypes[paramNr] & 0x0F
	case DTYPE_ON_OFF:
	case DTYPE_0b1:
		if(*paramValue > 1)
			*paramValue = 1;
		break;



	case DTYPE_MENU://parameter_dtypes[paramNr] & 0x0F
	{
		//get the used menu (upper 4 bit)
		const uint8_t menuId = parameter_dtypes[paramNr] >> 8;
		//get the number of entries
		uint8_t numEntries = getMaxEntriesForMenu(menuId);
		if(*paramValue >= numEntries)
			*paramValue = (uint8_t)(numEntries-1);

	} // parameter_dtypes[paramNr] & 0x0F case DTYPE_MENU
		break;

	default://parameter_dtypes[paramNr] & 0x0F

	case DTYPE_0B100:
		if(*paramValue > 100)
			*paramValue = 100;
		break;

	case DTYPE_PM100:
		if(*paramValue > 200)
			*paramValue = 200;
		break;

	case DTYPE_PM63:
	case DTYPE_0B127:
		if(*paramValue > 127)
			*paramValue = 127;
		break;
	} //parameter_dtypes[paramNr] & 0x0F


	// TODO this will also send MIDI_CC or CC_2 for the above items that have already been sent. is this desired?
	//send parameter change to uart tx
	if(paramNr < 128) // => Sound Parameter
		frontPanel_sendData(MIDI_CC,(uint8_t)paramNr,*paramValue);
	//else if(paramNr > 127 && (paramNr < END_OF_SOUND_PARAMETERS)) // => Sound Parameter above 127
	else if(paramNr > 127 && (paramNr <= 255)) // => Sound Parameter above 127
		frontPanel_sendData(CC_2,(uint8_t)(paramNr-128),*paramValue);
	else if (paramNr>=255 && (paramNr < END_OF_SOUND_PARAMETERS)) // => Sound Parameter above 255
	{
		frontPanel_sendData(CC3,(uint8_t)(paramNr-255),*paramValue);
	}
	else // non sound parameters (ie current step data, etc)
		menu_parseGlobalParam(paramNr,activeParameters[paramNr]);

	//frontPanel_sendData(0xb0,paramNr,*paramValue);
}

//-----------------------------------------------------------------
// given an encoder wheel change, will set the menu item to the correct new one
// or not change it if boundaries are reached
static void menu_moveToMenuItem(int8_t inc)
{

	int8_t activeParameter	= menuIndex & MASK_PARAMETER; // will be 0 to 7
	int8_t activePage		= (int8_t)(menuIndex >> PAGE_SHIFT); // will be 0 to 31

	uint8_t param;
	uint8_t allowedSkips	=3; //how many skip items do we allow in a row

	// clamp encoder to +/- 1 to make logic simpler
	inc = inc>0?1:-1;

checkvalid:
	DISABLE_CONV_WARNING
	activeParameter+=inc; // set new desired active parameter value
	END_DISABLE_CONV_WARNING
	if(inc > 0) {
		if(activeParameter == 4) { // move to 2nd section of sub-page

		} else if(activeParameter == 8) { // moved past end of 2nd sub-page

			if((menu_activePage==MENU_MIDI_PAGE) || (menu_activePage==SEQ_PAGE) || (buttonHandler_getMode() == SELECT_MODE_MOD)){ // in global menu, we can move to next sub-page

				activeParameter=0; // set to par 0 of next sub page
				activePage++; // and change page
				if(activePage >=NUM_SUB_PAGES)
					activePage=0; // wrap around to 1st (would only happen if we fill all 8. unlikely)
			} else
				return; // moved past last param on section 2. not allowed in most modes
		}
	} else { // inc == -1
		if(activeParameter == 3) { // move to first section of sub-page

		} else if(activeParameter==-1) { // if we went past 0
			if((menu_activePage==MENU_MIDI_PAGE) || (menu_activePage==SEQ_PAGE) || (buttonHandler_getMode() == SELECT_MODE_MOD)){ // in global menu, we can move to previous sub-page

				activeParameter=7; // put us on the last param of the previous sub-page
				activePage--; // and change page
				if(activePage < 0)
				{
					//do not wrap to last page
					activeParameter = 0;
					activePage=0;//NUM_SUB_PAGES-1; //wrap around to last page (would only happen if we fill all 8. unlikely)
				}
			} else
				return; // move past first param on section 1. not allowed in most modes
		}
	}

	// read the new param
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		param = pgm_read_byte(&modMatrixPages[menu_activePage][activePage].top1 + activeParameter);
	}
	else
	{
		param = pgm_read_byte(&menuPages[menu_activePage][activePage].top1 + activeParameter);
	}
	if(param == TEXT_SKIP) {
		if(allowedSkips--) // skip this one and check the next
			goto checkvalid;
		else
			return; // would never happen unless we have > 3 skips in a row
	}

	if(param == TEXT_EMPTY) // disallow the change
		return;

	// set the change
	menuIndex = (uint8_t)( (activePage << PAGE_SHIFT) | activeParameter);

}

//-----------------------------------------------------------------
void menu_setLoadSaveCategory(uint8_t cat)
{
	menu_saveOptions.category = cat;

	switch(menu_saveOptions.category) {
		default:
		case SAVE_TYPE_KIT:
		case SAVE_TYPE_MORPH:
			menu_saveOptions.nameBuffer = KIT_NAMES;
			break;
#if !EURORACK_MODULE
		case SAVE_TYPE_PATTERN:
			menu_saveOptions.nameBuffer = PATTERN_NAMES;
			menu_saveOptions.state = SAVE_STATE_CHANGE_NAME_DIALOG_N;
			break;

		case SAVE_TYPE_SONG:
			menu_saveOptions.nameBuffer = SONG_NAMES;
			break;

		case SAVE_TYPE_PROJECT:
			menu_saveOptions.nameBuffer = PROJECT_NAMES;
			break;
#endif
		case SAVE_TYPE_GLO:
#if !EURORACK_MODULE
		case SAVE_TYPE_ALL:
#endif
			menu_saveOptions.nameBuffer = UNUSED_NAMES;
			break;
		//case SAVE_TYPE_SAMPLES:     toptxt=PSTR("Samples "); break;
		}
}
//-----------------------------------------------------------------
// this is called when we need to reset the load/save page to it's default
// state
void menu_resetSaveParameters()
{

	//globals are autosaved now, samples don't exist no more
/*
	//reset save params
	if(menu_saveOptions.category >= SAVE_TYPE_GLO) // global and samples only have one choice - hit ok.
	{
		// return to the load/save kit page with "type" selected
		menu_saveOptions.state = SAVE_STATE_EDIT_PRESET_NR;
		menu_saveOptions.category = SAVE_TYPE_KIT;
	}
	else
	{
		*/
		// return to the state where preset number is selected
		editModeActive = 1;
		menu_saveOptions.state	= SAVE_STATE_EDIT_PRESET_NR;//SAVE_STATE_EDIT_TYPE;
	//}

	menu_repaintAll();

}
//-----------------------------------------------------------------
// switches us to a different menu sub page. If that page is already active
// and has multiple screens, will toggle to the other screen
// if its the global menu, and there are multiple pages will toggle to the next page
void menu_switchSubPage(uint8_t subPageNr, uint8_t toggle)
{
	//leave edit mode if active
	editModeActive = 0;

	//get current position
	uint8_t activeParameter	= menuIndex & MASK_PARAMETER;
	uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT;

	if((toggle) && (subPageNr == activePage)) { // toggle only
		// we are toggling to next screen, or back to first
		if(activeParameter < 4) {
			// toggle to next if possible
			if(has2ndPage(activePage))
				activeParameter=4;
			else if((menu_activePage==MENU_MIDI_PAGE) || (menu_activePage==SEQ_PAGE)){ // special case for global - wrap around to first
				activePage=0;
				activeParameter=0;
			}
		} else { // we are on 2nd screen

			if(buttonHandler_getMode() == SELECT_MODE_MOD)
			{
				if(activePage < NUM_SUB_PAGES-1 &&
				   pgm_read_byte(&modMatrixPages[menu_activePage][activePage+1].top1) != TEXT_EMPTY) {
					activePage++;
				} else {
					activePage=0;
				}
			}
			else if((menu_activePage==MENU_MIDI_PAGE) || (menu_activePage==SEQ_PAGE)){
				// in global mode, we move to next page if possible (or wrap to first)
				if(activePage < NUM_SUB_PAGES-1 &&
				   pgm_read_byte(&menuPages[menu_activePage][activePage+1].top1) != TEXT_EMPTY) {
					activePage++;
				} else {
					activePage=0;
				}
			}

			activeParameter=0;
		}
	} else { // move to different sub page

#if MOD_MATRIX_4_PAGE
		if(buttonHandler_getMode() == SELECT_MODE_MOD)
		{
			if(toggle)
			{

				if((activeParameter < 4)  && (has2ndPage(activePage))) {
					activeParameter=4;
				} else {
					activePage=0;
					activeParameter=0;
				}
			}
		}
		else
#endif
		{
		// we are moving to a different (specific) subpage
		activePage=subPageNr;
		if(activeParameter > 3 && has2ndPage(activePage))
			activeParameter = 4;
		else
			activeParameter = 0;
		}
	}

	menuIndex = (uint8_t)( (activePage << PAGE_SHIFT) | activeParameter);
}

//-----------------------------------------------------------------
void menu_resetActiveParameter()
{
	uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT;
	if(!has2ndPage(activePage))
	{
		DISABLE_CONV_WARNING
		menuIndex &= ~MASK_PARAMETER;
		END_DISABLE_CONV_WARNING
	}		
};
//-----------------------------------------------------------------
uint8_t menu_getSubPage()
{
	return (menuIndex&MASK_PAGE)>>PAGE_SHIFT;
};
//-----------------------------------------------------------------
void menu_switchPage(uint8_t pageNr)
{
	//autosave config page settings
	/*
	if(menu_activePage == MENU_MIDI_PAGE)
	{
		if(pageNr != MENU_MIDI_PAGE)
		{
			preset_saveGlobals();
		}
	}
	*/

	//clear all sequencer buttons
#if !EURORACK_MODULE
	led_clearSequencerLeds();
#endif

	switch(pageNr) {
	case MENU_MIDI_PAGE:
		{ // global settings page
			uint8_t toggle = (menu_activePage == MENU_MIDI_PAGE);
			menu_activePage=MENU_MIDI_PAGE;
			// leave edit mode if active
			editModeActive = 0;

			// **GMENU TODO do we need to clear any leds?
			if (toggle) // if we are on the settings page already, toggle between all pages
			{
				menu_switchSubPage(menu_getSubPage(),1);
			}
			else
			{
				//if we enter the cfg menu from somewhere else, start on page 0
				menu_switchSubPage(0,0);
			}
		}

		break;

	case FX_MODE_PAGE:

		menu_switchSubPage(fxBus.fxType, 1 );
		editModeActive = 0;
		menu_activePage=FX_MODE_PAGE;
		break;
#if !EURORACK_MODULE
	case SONG_MODE_PAGE:
		editModeActive = 0;
		menu_activePage=SONG_MODE_PAGE;
		led_initPatternSelectLeds(); //to show proper menu bank bar buttons
		break;

	case PATTERN_LOAD_PAGE:
	{
		// leave edit mode if active
		editModeActive = 0;
		menu_activePage=PATTERN_LOAD_PAGE;
		led_initPatternSelectLeds();

	}
	break;

	case PATTERN_SAVE_PAGE:
	{

		// leave edit mode if active
		editModeActive = 0;
		menu_activePage=PATTERN_SAVE_PAGE;
		led_initPatternSelectLeds();

	}
	break;

	case EUKLID_PAGE:
	{
		uint8_t toggle = (menu_activePage == EUKLID_PAGE);
		menu_activePage=EUKLID_PAGE;
		// leave edit mode if active
		editModeActive = 0;

		// **GMENU TODO do we need to clear any leds?
		if (toggle) // if we are on the settings page already, toggle between all pages
			menu_switchSubPage(menu_getSubPage(), 1);
		else menu_switchSubPage(0,0);

		led_updateSeqLeds();
	}
		break;
#endif
	case PERF_PAGE:
	{
		uint8_t toggle = (menu_activePage == PERF_PAGE);

		// **GMENU TODO do we need to clear any leds?
		if (toggle) // if we are on the settings page already, toggle between all pages
		{
			menu_switchSubPage(menu_getSubPage(),1);
		}
		else menu_switchSubPage(0,0);
	}

	case MENU_LAST_STEP_PAGE:
	case PATTERN_SETTINGS_PAGE:
	case SEQ_PAGE:

		menu_activePage = pageNr;
		//leave edit mode if active
		editModeActive = 0;
		break;


	case SAVE_PAGE:
		{

			//re-init the save page variables
			menu_resetSaveParameters();

			if((menu_activePage != LOAD_PAGE) && (menu_activePage != SAVE_PAGE))
			{
				//when coming from another page, do a complete reset and show the sound select page
				menu_resetSaveParameters();
				//menu_saveOptions.what	= SAVE_TYPE_SOUND;
			}

			menu_activePage = SAVE_PAGE;

			// load the name of the current preset
			copyNameToEditBuffer(menu_selectedPresetNr[menu_saveOptions.category],menu_saveOptions.nameBuffer );
			//preset_loadName(menu_currentPresetNr[menu_saveOptions.category], menu_saveOptions.nameBuffer, menu_saveOptions.category);
		}
		break;
	case LOAD_PAGE:
		{
			//re-init the save page variables
			menu_resetSaveParameters();

			if((menu_activePage != LOAD_PAGE) && (menu_activePage != SAVE_PAGE))
			{
				//when coming from another page, do a complete reset and show the sound select page
				menu_resetSaveParameters();
				//menu_saveOptions.what	= SAVE_TYPE_SOUND;
			}

			menu_activePage = LOAD_PAGE;
			// load the name of the current preset from disk
			copyNameToEditBuffer(menu_selectedPresetNr[menu_saveOptions.category],menu_saveOptions.nameBuffer );
			//preset_loadName(menu_currentPresetNr[menu_saveOptions.category], menu_saveOptions.nameBuffer, menu_saveOptions.category);
		}
		break;

	default: //voice pages etc
	{
		menu_activePage = pageNr;
		if(pageNr<7) {
			menu_setActiveVoice(pageNr);
		}
		//leave edit mode if active
		editModeActive = 0;

		//query current sequencer step states and light up the corresponding leds
#if 0
		uint8_t trackNr = menu_getActiveVoice(); //max 6 => 0x6 = 0b110
		uint8_t patternNr = menu_shownPattern; //max 7 => 0x07 = 0b111
		uint8_t value = (uint8_t)((trackNr<<4) | (patternNr&0x7));
		frontPanel_sendData(LED_CC,LED_QUERY_SEQ_TRACK,value);
#else

#if !EURORACK_MODULE
		led_updateSeqLeds();
#endif
#endif
		}
		break;
	}		


	//re initialize the voice LEDs
	if(pageNr==PERF_PAGE)
	{
		//light up mute leds
	//	buttonHandler_showMuteLEDs();
	} else
	{

		//led_setActiveVoiceLeds((uint8_t)(1<<(menu_getActiveVoice())));
		//menu_muteModeActive = 0;
	}

	//go to 1st parameter on sub page
	menu_resetActiveParameter();

	//force complete repaint
	menu_repaintAll();
};
//-----------------------------------------------------------------
void menu_sendAllGlobals()
{
	uint16_t i;
	for(i=PAR_BEGINNING_OF_GLOBALS;(i<NUM_PARAMS);i++)
	{
		menu_parseGlobalParam(i,activeParameters[i]);
	}
};
//-----------------------------------------------------------------
// This is used to send a global (non voice specific) parameter to the back.
// This includes values for the currently active step in the sequencer
void menu_parseGlobalParam(uint16_t paramNr, uint8_t value)
{
	switch(paramNr)
	{

	// midi mode is not used anymore
	//case PAR_MIDI_MODE:
	//	frontPanel_sendData(SEQ_CC,SEQ_MIDI_MODE,parameter_values[PAR_MIDI_MODE]);
	//	break;

	case PAR_MORPH_TARGET:
		preset_morph(preset_morphValue, 1);
		break;

	case PAR_MIDI_CHAN_7:
		paramNr -= 5; //because they are not after one another in the param list
	case PAR_MIDI_CHAN_1:
	case PAR_MIDI_CHAN_2:
	case PAR_MIDI_CHAN_3:
	case PAR_MIDI_CHAN_4:
	case PAR_MIDI_CHAN_5:
	case PAR_MIDI_CHAN_6:
	case PAR_MIDI_CHAN_GLOBAL:
	{
		uint8_t voice;
		uint8_t channel = (uint8_t)(value-1);
		if(paramNr==PAR_MIDI_CHAN_GLOBAL)
			voice= 7; // will be 7 for global channel
		else
			voice=(uint8_t)(paramNr - PAR_MIDI_CHAN_1); // will be 0-6 for voice channels
		frontPanel_sendData(SEQ_CC,SEQ_MIDI_CHAN,(uint8_t)((voice<<4) | channel ));
	}
	break;


	case UNUSED_PAR_GLO_SAT:

		break;

	case PAR_POS_Y:
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());
		frontPanel_sendData(SEQ_CC,SEQ_POSY,value);
		break;

	case PAR_FLUX:
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());
		frontPanel_sendData(SEQ_CC,SEQ_FLUX,value);
		break;

	case PAR_SOM_FREQ:
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());
		frontPanel_sendData(SEQ_CC,SEQ_SOM_FREQ,value);
		break;

	case PAR_TRACK_LENGTH:
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());
		frontPanel_sendData(SEQ_CC,SEQ_TRACK_LENGTH,value);
		break;

	case PAR_SHUFFLE:
		frontPanel_sendData(SEQ_CC,SEQ_SHUFFLE,value);
		break;

	case PAR_AUTOM_TRACK:
		frontPanel_sendData(SEQ_CC,SEQ_SET_AUTOM_TRACK,value);
		break;

	case PAR_P1_DEST:
	case PAR_P2_DEST:
	{ //step range 0-127 value range 0-255!
		//**AUTOM - translate back into a parameter when sending value to cortex
		uint8_t tmp=(uint8_t)pgm_read_word(&modTargets[value].param);
		frontPanel_sendData(SEQ_CC, SEQ_SELECT_ACTIVE_STEP,activeParameters[PAR_ACTIVE_STEP]);
		frontPanel_sendData((uint8_t)
				(paramNr==PAR_P1_DEST ? SET_P1_DEST : SET_P2_DEST),tmp>>7,tmp&0x7f);
		break;
	}

	case PAR_P1_VAL:
		frontPanel_sendData(SET_P1_VAL,activeParameters[PAR_ACTIVE_STEP],value);
		break;

	case PAR_P2_VAL:
		frontPanel_sendData(SET_P2_VAL,activeParameters[PAR_ACTIVE_STEP],value);
		break;
/*
	case PAR_QUANTISATION:
		frontPanel_sendData(SEQ_CC,SEQ_SET_QUANT,value);
		break;
		*/

	case PAR_SCREENSAVER_ON_OFF:

		break;

	case PAR_BPM:
		frontPanel_sendData(SET_BPM,value&0x7f,(value>>7)&0x7f);
		break;
	case PAR_MORPH:
	{
		//value += (value==127)*1;
		preset_morph(value, 1);
	}
	break;

	case PAR_CV_SAVE:

		break;


	case PAR_VOICE_DECIMATION1:
	case PAR_VOICE_DECIMATION2:
	case PAR_VOICE_DECIMATION3:
	case PAR_VOICE_DECIMATION4:
	case PAR_VOICE_DECIMATION5:
	case PAR_VOICE_DECIMATION6:
	case PAR_VOICE_DECIMATION_ALL:
		//select the track nr
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,((uint8_t)(paramNr-PAR_VOICE_DECIMATION1)));
		//send the new output channel
		frontPanel_sendData(VOICE_CC,VOICE_DECIMATION,value);
		break;

		/*
		case PAR_AUDIO_OUT1:
		case PAR_AUDIO_OUT2:
		case PAR_AUDIO_OUT3:
		case PAR_AUDIO_OUT4:
		case PAR_AUDIO_OUT5:
		case PAR_AUDIO_OUT6:
		//select the track nr
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,paramNr-PAR_AUDIO_OUT1);
		//send the new output channel
		frontPanel_sendData(VOICE_CC,VOICE_AUDIO_OUT,value);
		break;
		 */

	case PAR_ROLL:
	{
		/*roll rates
			0 - one shot immediate trigger
			1 - 1/1
			2 - 1/2
			3 - 1/3
			4 - 1/4
			5 - 1/6
			6 - 1/8
			7 - 1/12
			8 - 1/16
			9 - 1/24
			10 - 1/32
			11 - 1/48
			12 - 1/64
			13 - 1/96
			14 - 1/128
		 */
		frontPanel_sendData(SEQ_CC,SEQ_ROLL_RATE,value);
	}
	break;



#if !EURORACK_MODULE
	case PAR_EUKLID_LENGTH: {
		// set the length of the currently active track
		uint8_t length = (uint8_t)(value-1);
		uint8_t pattern = menu_shownPattern;
	//	uint8_t msg = (uint8_t)((pattern&0x7) | (length<<3));
		//select the track nr
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());
	//	frontPanel_sendData(SEQ_CC,SEQ_EUKLID_LENGTH,msg);

		//uint8_t length 	= cortex_midiMsg.data2 >> 3;
		//length += 1;
		euklid_setLength(menu_getActiveVoice(), length+1);


		uint8_t steps = (uint8_t)(activeParameters[PAR_EUKLID_STEPS]); // max 16
		//uint8_t pattern = menu_shownPattern; //max 7
		//msg = (uint8_t)((pattern&0x7) | (steps<<3));
		//frontPanel_sendData(SEQ_CC,SEQ_EUKLID_STEPS,msg);

		euklid_setSteps(menu_getActiveVoice(),steps,pattern);
		led_updateSeqLeds();
	}
	break;

	case PAR_EUKLID_STEPS:	{
		uint8_t steps =(uint8_t)( value);
		uint8_t pattern = menu_shownPattern;
		//uint8_t msg =(uint8_t)( (pattern&0x7) | (steps<<3));

		//select the track nr
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());


		//activeParameters[PAR_EUKLID_STEPS] = frontParser_midiMsg.data2;
		//							menu_repaint();
		//frontPanel_sendData(SEQ_CC,SEQ_EUKLID_STEPS,msg);
		euklid_setSteps(menu_getActiveVoice(),steps,pattern);
		led_updateSeqLeds();
	}
	break;
	case PAR_EUKLID_ROTATION:	{
		uint8_t rotation =(uint8_t)(value);
		uint8_t pattern = menu_shownPattern;
		//uint8_t msg =(uint8_t)( (pattern&0x7) | (rotation<<3));

		//select the track nr
		frontPanel_sendData(SEQ_CC,SEQ_SET_ACTIVE_TRACK,menu_getActiveVoice());

		//frontPanel_sendData(SEQ_CC,SEQ_EUKLID_ROTATION,msg);
		euklid_setRotation(menu_getActiveVoice(),rotation,pattern);
		//frontParser_updateTrackLeds(frontParser_activeTrack, pattern);
		led_updateSeqLeds();
	}
	break;
#endif
/*
	case PAR_PATTERN_BEAT:
		frontPanel_sendData(SEQ_CC,SEQ_SET_PAT_BEAT,value);
		break;

	case PAR_PATTERN_NEXT:
		frontPanel_sendData(SEQ_CC,SEQ_SET_PAT_NEXT,value);
		break;
*/


	case PAR_ACTIVE_STEP:
		frontPanel_sendData(SEQ_CC,SEQ_REQUEST_STEP_PARAMS,value);
		break;

	case PAR_STEP_PROB:
		frontPanel_sendData(SEQ_CC,SEQ_PROB,value);
		break;

	case PAR_STEP_NOTE:
		frontPanel_sendData(SEQ_CC,SEQ_NOTE,value);
		break;

	case PAR_STEP_FLAM:
		frontPanel_sendData(SEQ_CC,SEQ_FLAM,value);
		break;

	case PAR_STEP_SHIFT:
		frontPanel_sendData(SEQ_CC,SEQ_SHIFT,value);
		break;

	case PAR_STEP_VOLUME:
		frontPanel_sendData(SEQ_CC,SEQ_VOLUME,value);
		break;
	case PAR_MIDI_ROUTING:
		frontPanel_sendData(SEQ_CC, SEQ_MIDI_ROUTING, value);
		break;
	case PAR_MIDI_FILT_TX:
		frontPanel_sendData(SEQ_CC, SEQ_MIDI_FILT_TX, value);
		break;
	case PAR_MIDI_FILT_RX:
		frontPanel_sendData(SEQ_CC, SEQ_MIDI_FILT_RX, value);
		break;
	case PAR_PRESCALER_CLOCK_IN:
			frontPanel_sendData(SEQ_CC, SEQ_TRIGGER_IN_PPQ, value);
			break;
	case PAR_PRESCALER_CLOCK_OUT1:
			frontPanel_sendData(SEQ_CC, SEQ_TRIGGER_OUT1_PPQ, value);
			break;
	case PAR_PRESCALER_CLOCK_OUT2:
			frontPanel_sendData(SEQ_CC, SEQ_TRIGGER_OUT2_PPQ, value);
			break;
	case PAR_TRIG_GATE_MODE:
			frontPanel_sendData(SEQ_CC, SEQ_TRIGGER_GATE_MODE, value);
			break;
	case PAR_BAR_RESET_MODE:
		frontPanel_sendData(SEQ_CC, SEQ_BAR_RESET_MODE, value);
		break;

	}
}
//-----------------------------------------------------------------
#if !EURORACK_MODULE
static void menu_processSpecialCaseValues(uint16_t paramNr/*, const uint8_t *value*/)
{
	//To see the generated pattern we have to update the step view
	if( (paramNr == PAR_EUKLID_LENGTH) || (paramNr == PAR_EUKLID_STEPS) || (paramNr == PAR_EUKLID_ROTATION) )
	{
		//query current sequencer step states and light up the corresponding leds 
		//frontPanel_sendData(LED_CC,LED_QUERY_SEQ_TRACK,menu_activePage);
		led_clearSequencerLeds();
	}
}
#endif
//-----------------------------------------------------------------
static uint8_t limitPotValue(int8_t diff, uint16_t paramNr)
{
	uint8_t originalValue;
	if(paramNr >= END_OF_SOUND_PARAMETERS)
	{
		originalValue = activeParameters[paramNr];
	}
	else
	{
		originalValue = activeParameters[paramNr];
	}

	//prevent  overflow
	uint8_t newValue;
	if(diff >= 0)
	{
		if((originalValue + diff) > 255) newValue = 0xff;
		else newValue = originalValue + diff;
	}
	else
	{
		if(((uint8_t)(originalValue + diff))  > originalValue)
			newValue = 0;
		else
			newValue = originalValue + diff;
	}

	//limit to range

	switch(parameter_dtypes[paramNr] & 0xFF)
	{
	case DTYPE_TARGET_SELECTION_CV:
	case DTYPE_TARGET_SELECTION_VELO:
	case DTYPE_TARGET_SELECTION_LFO: {


#if 0
		//**VELO **LFO limit range to start and end for the applicable voice, but allow a setting of 0 (for off)
		uint8_t voiceNr=0;
		uint8_t s, e, rng;
		if(paramNr >= PAR_VEL_DEST_1 && paramNr <= PAR_VEL_DEST_6 )
			voiceNr=(uint8_t)(paramNr-PAR_VEL_DEST_1);
		else if(paramNr >= PAR_TARGET_LFO1 && paramNr <= PAR_TARGET_LFO6) {
			// this is the voice being targeted, not the lfo number
			voiceNr=(uint8_t)(mainParameters[PAR_VOICE_LFO1+(paramNr - PAR_TARGET_LFO1)]-1);
		}
		s=pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		e=pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		// start will be a non-zero value. We need to allow setting a value of 0 (off) or
		// something within the start to end range.
		rng=(uint8_t)(frac * ((e-s)+1)); // where in the range we are
		if(!rng)
			return 0; // set to off
		else
			return (uint8_t)((s-1)+rng); // set to an offset beginning at start of range

#else
#pragma message "noch machen!"
#endif
	}
		break;
/*
	case DTYPE_AUTOM_TARGET:
		//AUTOM - scale to our range for pot value
		if(newValue >= (getNumModTargets()-1)) newValue =  (getNumModTargets()-1);
		//return (uint8_t)((getNumModTargets()-1) * frac);
*/

	case DTYPE_0B255:
		//return value;
		break;
	case DTYPE_VOICE_LFO:
		// the LFO target voice number is stored and displayed as 1 based
		if(newValue > 6) newValue = 6;
		else if(newValue < 1) newValue = 1;
		//return (uint8_t)(1 + 5*frac);
		break;
	case DTYPE_1B16:
		if(newValue > 16) newValue = 16;
		else if(newValue < 1) newValue = 1;
		break;
	case DTYPE_1B64:
		if(newValue > 64) newValue = 64;
		else if(newValue < 1) newValue = 1;
		break;
	case DTYPE_0B15:
		if(newValue > 15) newValue = 15;
		//return (uint8_t)(15*frac);
		break;
		// These are 0 or 1
	//case DTYPE_MIX_FM:
	case DTYPE_ON_OFF:
	case DTYPE_0b1:
		if(newValue > 1) newValue = 1;
		//return frac>0.5f;
		break;
	case DTYPE_MENU:
	{
		//get the used menu (upper 4 bit)
		const uint8_t menuId = parameter_dtypes[paramNr] >> 8;
		//get the number of entries
		uint8_t numEntries=getMaxEntriesForMenu(menuId);
		//return (uint8_t)(frac * (numEntries-1));
		if(newValue > (numEntries-1)) newValue = (numEntries-1);
	}

		break;

	case DTYPE_PM100:
		if(newValue > 200) newValue = 200;
		break;

	case DTYPE_0B100:
		if(newValue > 100) newValue = 100;
		break;
	// These ones are all in the 0-127 range
	case DTYPE_PM63:
	case DTYPE_NOTE_NAME:
	case DTYPE_0B127:
	default:
		if(newValue > 127) newValue = 127;
		//return (uint8_t)(127*frac);
		break;
	}
	return newValue;
}
//-----------------------------------------------------------------
#if 0
//used in normal pot model, can probably be removed
static uint8_t getDtypeValue(uint8_t value, uint16_t paramNr)
{
	const float frac = (value/255.f); // yield a fraction from 0 to 1

	switch(pgm_read_word(&parameter_dtypes[paramNr]) & 0xFF)
	{
	case DTYPE_TARGET_SELECTION_VELO:
	case DTYPE_TARGET_SELECTION_LFO: {
		//**VELO **LFO limit range to start and end for the applicable voice, but allow a setting of 0 (for off)
		uint8_t voiceNr=0;
		uint8_t s, e, rng;
		if(paramNr >= PAR_VEL_DEST_1 && paramNr <= PAR_VEL_DEST_6 )
			voiceNr=(uint8_t)(paramNr-PAR_VEL_DEST_1);
		else if(paramNr >= PAR_TARGET_LFO1 && paramNr <= PAR_TARGET_LFO6) {
			// this is the voice being targeted, not the lfo number
			voiceNr=(uint8_t)(mainParameters[PAR_VOICE_LFO1+(paramNr - PAR_TARGET_LFO1)]-1);
		}
		s=pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		e=pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		// start will be a non-zero value. We need to allow setting a value of 0 (off) or
		// something within the start to end range.
		rng=(uint8_t)(frac * ((e-s)+1)); // where in the range we are
		if(!rng)
			return 0; // set to off
		else
			return (uint8_t)((s-1)+rng); // set to an offset beginning at start of range
		}
		break;

	case DTYPE_AUTOM_TARGET:
		//**AUTOM - scale to our range for pot value
		return (uint8_t)((getNumModTargets()-1) * frac);
	case DTYPE_0B255:
		return value;
		break;
	case DTYPE_VOICE_LFO:
		// the LFO target voice number is stored and displayed as 1 based
		return (uint8_t)(1 + 5*frac);
		break;
	case DTYPE_1B16:
		return (uint8_t)(1 + 15*frac);
		break;
	case DTYPE_0B15:
		return (uint8_t)(15*frac);
		break;
		// These are 0 or 1
	case DTYPE_MIX_FM:
	case DTYPE_ON_OFF:
	case DTYPE_0b1:
		return frac>0.5f;
		break;
	case DTYPE_MENU:
	{
		//get the used menu (upper 4 bit)
		const uint8_t menuId = pgm_read_word(&parameter_dtypes[paramNr]) >> 8;
		//get the number of entries
		uint8_t numEntries=getMaxEntriesForMenu(menuId);
		return (uint8_t)(frac * (numEntries-1));
	}

		break;
	// These ones are all in the 0-127 range
	case DTYPE_PM63:
	case DTYPE_NOTE_NAME:
	case DTYPE_0B127:
	default:
		return (uint8_t)(127*frac);
		break;
	}	
	return 0;
}
#endif
//-----------------------------------------------------------------
uint8_t menu_getDisplayParameterType(uint8_t pos)
{
	const uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT;
	const uint8_t activeParameter	= menuIndex & MASK_PARAMETER;
	const uint8_t isOn2ndPage		= ( activeParameter > 3) * 4;

	uint16_t paramNr;
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		paramNr				= pgm_read_word(&modMatrixPages[menu_activePage][activePage].bot1 +	pos + isOn2ndPage);
	}
	else
	{
		paramNr				= pgm_read_word(&menuPages[menu_activePage][activePage].bot1 +	pos + isOn2ndPage);
	}


	return parameter_dtypes[paramNr] & 0xFF;
}
//-----------------------------------------------------------------
uint8_t menu_parseKnobValue(uint8_t potNr, int16_t diff)
{
	screensaver_touch();
	preset_touchTimer(PRESET_KIT_TIMER);

	const uint8_t activePage		= (menuIndex&MASK_PAGE)>>PAGE_SHIFT;
	const uint8_t activeParameter	= menuIndex & MASK_PARAMETER;
	const uint8_t isOn2ndPage		= ( activeParameter > 3) * 4;

	uint16_t paramNr;
	if(buttonHandler_getMode() == SELECT_MODE_MOD)
	{
		paramNr				= pgm_read_word(&modMatrixPages[menu_activePage][activePage].bot1 +	potNr + isOn2ndPage);
	}
	else
	{
		paramNr				= pgm_read_word(&menuPages[menu_activePage][activePage].bot1 +	potNr + isOn2ndPage);
	}


	//-------------------------------------------------

	//leave edit mode if active
	editModeActive = 0;
	{
		//force complete repaint if edit mode is left
		menu_repaintAll();
	}

	if( (menu_activePage == SAVE_PAGE) || (menu_activePage == LOAD_PAGE)) {
		int16_t tmpValue = saveScreenPots[potNr] + diff;
		if(tmpValue < 0) tmpValue = 0;
		else if(tmpValue>127) tmpValue = 127;
		saveScreenPots[potNr] = tmpValue;

		menu_handleSaveScreenKnobValue(potNr, tmpValue);
		return 0;
	}



	//parameter fetch. given the value of the pot, and the parameter number
	// fetch the actual numeric value that the parameter should be set to
	uint8_t dtypeValue = limitPotValue(diff, paramNr);//getDtypeValue(potValue,paramNr);
#if !EURORACK_MODULE
	menu_processSpecialCaseValues(paramNr/*,&dtypeValue*/);
#endif

#if !EURORACK_MODULE
	//make changes temporary while an automation step is armed - save original value
	if((buttonHandler_resetLock==0) && buttonHandler_getArmedAutomationStep() != NO_STEP_SELECTED)
	{
		if(paramNr >= END_OF_SOUND_PARAMETERS)
		{
			buttonHandler_originalValue = activeParameters[paramNr];
		}
		else
		{
			buttonHandler_originalValue = activeParameters[paramNr];
		}
		buttonHandler_originalParameter = paramNr;
		buttonHandler_resetLock = 1;
	}
#endif

	//update parameter value to the new value
	//mainParameters[paramNr] = dtypeValue;
	if(paramNr >= END_OF_SOUND_PARAMETERS)
	{
		activeParameters[paramNr] = dtypeValue;
	}
	else
	{
		activeParameters[paramNr] = dtypeValue;
	}

	switch(parameter_dtypes[paramNr] & 0xFF)
	{

	case DTYPE_TARGET_SELECTION_VELO:
	{
	#if OLD_LFO_ROUTING
		//**Velo - limit param start and end to range for the target voice
		// this is a value from 1 to 6, so we adjust to be 0 based
		uint8_t value = dtypeValue;
		uint8_t voiceNr =  (paramNr - PAR_VEL_DEST_1);
		if(dtypeValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
			if(diff < 0) // going down, allow 0
				value=0;
			else // going up fix to start
				value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		} else if (dtypeValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
			value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		}

		activeParameters[paramNr] = value;
		//**VELO knob value range. convert to param before sending
		// the value we have is an index into modTargets, we need a parameter
		value = (uint8_t)pgm_read_word(&modTargets[value].param);

		/*
		uint8_t upper,lower;
		upper = (uint8_t)(((value&0x80)>>7) | ((((uint8_t)(paramNr - PAR_VEL_DEST_1))&0x3f)<<1));
		lower = value&0x7f;
		frontPanel_sendData(CC_VELO_TARGET,upper,lower);
		*/

		modNode_setDestination(&velocityModulators[(uint8_t)(paramNr - PAR_VEL_DEST_1)], value);
	#endif
		}
		break;

	case DTYPE_TARGET_SELECTION_CV:
	{
			//**limit param start and end to range for the target voice
			// this is a value from 1 to 6, so we adjust to be 0 based
			uint16_t value = dtypeValue;
			uint16_t cvNr=(paramNr - PAR_CV_DEST_1);
			uint8_t voiceNr = cvNr / 3;
#if NUM_MOD_SLOTS != 18
#error voice calculation needs to be adapted to  NUM_MOD_SLOTS number!!!
#endif
			if(dtypeValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
				if(diff < 0) // going down, allow 0
					value=0;
				else // going up fix to start
					value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
			} else if (dtypeValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
				value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
			}

			activeParameters[paramNr] = value;
			//**VELO knob value range. convert to param before sending
			// the value we have is an index into modTargets, we need a parameter
			value = pgm_read_word(&modTargets[value].param);

			/*
			uint8_t upper,lower;
			upper = (uint8_t)(((value&0x80)>>7) | ((((uint8_t)(paramNr - PAR_VEL_DEST_1))&0x3f)<<1));
			lower = value&0x7f;
			frontPanel_sendData(CC_VELO_TARGET,upper,lower);
			*/
			modNode_setDestination(&cvModulators[cvNr], value);
			//modNode_setDestination(&velocityModulators[(uint8_t)(paramNr - PAR_VEL_DEST_1)], value);
		}
		break;

	case DTYPE_VOICE_LFO:
	{
#if OLD_LFO_ROUTING
		// **LFOTARGFIX - ensure that the lfo mod target is pointing to the same type of modulation
		// on the new voice
		const uint8_t newTargVal=getModTargetIdxFromGapIdx((uint8_t)(dtypeValue-1),menu_TargetVoiceGapIndex);

		// update the lfo mod target if applicable
		activeParameters[PAR_TARGET_LFO1 + (paramNr - PAR_VOICE_LFO1)] = newTargVal;

		// determine the real param value given the index into modTargets
		const uint8_t value =  (uint8_t)pgm_read_word(&modTargets[newTargVal].param);

#if 0
		uint8_t upper,lower;
		upper = (uint8_t)(((value&0x80)>>7) | ((((uint8_t)(paramNr - PAR_VOICE_LFO1))&0x3f)<<1));
		lower = value&0x7f;
		frontPanel_sendData(CC_LFO_TARGET,upper,lower);
#else
		const uint8_t lfoNr = (uint8_t)(paramNr - PAR_VOICE_LFO1);
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
#endif
#endif
	}
		break;
	case DTYPE_TARGET_SELECTION_LFO:
	{

#if OLD_LFO_ROUTING
 		//**LFO - limit param start and end to range for the target voice (not the lfo number)
		// this is a value from 1 to 6, so we adjust to be 0 based
		uint8_t value = dtypeValue;
		uint8_t voiceNr =  (uint8_t)(activeParameters[PAR_VOICE_LFO1+(paramNr - PAR_TARGET_LFO1)]-1);
		if(dtypeValue < pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start)) {
			if(diff < 0) // going down, allow 0
				value=0;
			else // going up fix to start
				value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].start);
		} else if (dtypeValue > pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end)) {
			value = pgm_read_byte(&modTargetVoiceOffsets[voiceNr].end);
		}

		activeParameters[paramNr] = value;


		// **LFOTARGFIX - save the gap index
		menu_TargetVoiceGapIndex = getModTargetGapIndex(value);

		//**LFO convert to param value and send
		// determine the real param value given the index into modTargets
		value = (uint8_t)pgm_read_word(&modTargets[value].param);

#if 0
		uint8_t upper,lower;
		upper = (uint8_t)(((value&0x80)>>7) | ((((uint8_t)(paramNr - PAR_TARGET_LFO1))&0x3f)<<1));
		lower = value&0x7f;
		frontPanel_sendData(CC_LFO_TARGET,upper,lower);

#else

			const uint8_t lfoNr = (uint8_t)(paramNr - PAR_TARGET_LFO1);
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
#endif
#endif

	}
		break;

	default: // all other sound parameters are send as CC or CC2. anything else is handled specially
	{

		//-----------------
		uint8_t morph;

		//Do not morph audio routing or globals
		if((paramNr < END_OF_SOUND_PARAMETERS))
		{
			if((paramNr >= PAR_AUDIO_OUT1) && (paramNr <= PAR_AUDIO_OUT6)) //morph_index++;
			{
				//always use source kit (morph = 0) for audio routing
				morph = 0;
			}
			else
			{
				morph = activeParameters[PAR_MORPH];
				//morph = preset_morphValue;
				dtypeValue = preset_getMorphValue(paramNr, morph);
				//dtypeValue = interpolate(activeParameters[paramNr],mainParameters[activeParameters[PAR_MORPH_TARGET]][paramNr],morph);
			}
		}

		//-----------------


		if(paramNr<128) // => Sound Parameter below 128
			frontPanel_sendData(MIDI_CC,(uint8_t)paramNr,dtypeValue);
		//else if(paramNr>=128 && (paramNr < END_OF_SOUND_PARAMETERS)) // => Sound Parameter above 127
		else if(paramNr>=128 && (paramNr < 255)) // => Sound Parameter above 127
			frontPanel_sendData(CC_2,(uint8_t)(paramNr-128),dtypeValue);

		else if (paramNr>=255 && (paramNr < END_OF_SOUND_PARAMETERS)) // => Sound Parameter above 255
		{
			frontPanel_sendData(CC3,(uint8_t)(paramNr-255),dtypeValue);

		}
		else
			menu_parseGlobalParam(paramNr,dtypeValue);
		break;
	}
	}

	return 1;
}

//-----------------------------------------------------------------
// TODO this looks to be unused. should it be removed?
#if 0
void menu_sendAllParameters()
{
	uint16_t i;
	for(i=0;i<END_OF_SOUND_PARAMETERS;i++)
		//for(i=PAR_PAN5;i<PAR_MOD_OSC_GAIN1_CYM;i++)

	{
		//send parameter change to uart tx
		//since we are sending a big amount of data here we have to be sure 
		//that the uart tx buffer doesn't overflow
		//so we check the return value
		if(i<128) {
			frontPanel_sendData(MIDI_CC,(uint8_t)i,activeParameters[i]);
		} else {
			frontPanel_sendData(CC_2,(uint8_t)(i-128),activeParameters[i]);
		}	
		//frontPanel_sendData(MIDI_CC,i,parameters[i].value);

		//delay to not overflow the rx buffer on the cortex
		//TODO ACK scheme for speedup testen
		if((i&0x1f) == 0x1f) //every 32 steps
			DWT_Delay_us(1);
	}		
}
#endif
//----------------------------------------------------------------
uint8_t menu_getActivePage()
{
	return menu_activePage;
};
//----------------------------------------------------------------
uint8_t menu_getActiveVoice()
{
	return menu_activeVoice;
};
//----------------------------------------------------------------
void menu_setActiveVoice(uint8_t voiceNr)
{
	// **LFOTARGFIX - save the gap index
	menu_TargetVoiceGapIndex = getModTargetGapIndex(activeParameters[PAR_TARGET_LFO1+voiceNr]);
	menu_activeVoice = voiceNr;
};
//----------------------------------------------------------------
#if !EURORACK_MODULE
uint8_t menu_areMuteLedsShown()
{
	return menu_muteModeActive;
}
#endif
//----------------------------------------------------------------

// given a 0-127 number will return a midi note name
void setNoteName(uint8_t num, char *buf)
{
	uint8_t n = num%12; // get the note number (0 is C, 9 is A, 11 is B)
	// get the note letter ascii A=65, C=67. All repeat twice except E and B.
	// B is at the end, so only E presents a hurdle
	buf[0]=(char)((n>8 ? 60 : 67) + ((n+(n>4))/2));
	// determine natural or sharp (# if the note is 1, 3, 6, 8, 10)
	buf[1]=(n < 5 && (n&1)) || (n > 5 && !(n&1)) ? '#' : ' ';
	// determine the octave 0=48
	buf[2]=(char)(48+(num / 12));
}

/* fill up to 2 bytes of a buffer with string representation of a number */
// space padded unsigned
void numtostrpu_2Bytes(char *buf, uint8_t num, char pad)
{
    if(num > 99) {
    	//not possible for 2 char number
        buf[0]='>';
        buf[1]='>';
        return;
    }

    if(num > 9) {
        buf[0]=(char)('0'+(num / 10));
        num %=10;
    }
    else
    {
        buf[0]='0';
    }
    buf[1]=(char)('0'+num);
}

/* fill up to 3 bytes of a buffer with string representation of a number */
// space padded unsigned
void numtostrpu(char *buf, uint8_t num, char pad)
{
    if(num > 99) {
        buf[0]=(char)('0'+(num / 100));
        num %= 100;
    } else {
        buf[0]=pad;
    }
    if(num > 9) {
        buf[1]=(char)('0'+(num / 10));
        num %=10;
    } else if(buf[0]==pad) {
        buf[1]=pad;
    } else
        buf[1]='0';
    buf[2]=(char)('0'+num);
}

// space padded signed
void numtostrps(char *buf, int8_t num)
{

    if(num > 99) {
        buf[0]=(char)('0'+(num / 100));
        num %= 100;
    } else if(num < 0) {
        buf[0]='-';
        num = (int8_t)(-num);
    } else {
        buf[0]=' ';
    }

    if(num > 9) {
        buf[1]=(char)('0'+(num / 10));
        num %=10;
    } else if(buf[0] < '0' /* space or minus */) {
        buf[1]=' ';
    } else {
        buf[1]='0';
    }
    buf[2]=(char)('0'+num);
}

// non space padded unsigned
void numtostru(char *buf, uint8_t num)
{
    uint8_t b=0;
    if(num > 99) {
        buf[b++]=(char)('0'+(num / 100));
        num %= 100;
        if(num < 10)
            buf[b++]='0';
    }
    if(num > 9) {
        buf[b++]=(char)('0'+(num / 10));
        num %=10;
    }
    buf[b]=(char)('0'+num);
}

// non space padded signed
#if 0
void numtostrs(char *buf, int8_t num)
{
    uint8_t b=0;
    if(num > 99) {
        buf[b++]=(char)('0'+(num / 100));
        num %= 100;
        if(num < 10)
            buf[b++]='0';
    } else if(num < 0) {
        buf[b++]='-';
        num=(int8_t)(-num);
    }

    if(num > 9) {
        buf[b++]=(char)('0'+(num / 10));
        num %=10;
    }
    buf[b]=(char)('0'+num);
}
#endif

// make 1st 3 letters of buffer uppercase
void upr_three(char *buf)
{
	DISABLE_CONV_WARNING
	if(*buf >='a' && *buf <= 'z')
		(*buf) -=32;
	buf++;
	if(*buf >='a' && *buf <= 'z')
		(*buf) -=32;
	buf++;
	if(*buf >='a' && *buf <= 'z')
		(*buf) -=32;
	buf++;
	END_DISABLE_CONV_WARNING
}
