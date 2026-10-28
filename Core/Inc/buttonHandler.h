/*
 * buttonHandler.h
 *
 * Created: 24.04.2012 16:02:07
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef BUTTONHANDLER_H_
#define BUTTONHANDLER_H_

#include "main.h"

extern uint8_t buttonHandler_selectedStep;
extern uint16_t buttonHandler_originalParameter;	//saves parameter number for step automation reset (stgep assign)
extern uint8_t buttonHandler_originalValue; 
extern uint8_t buttonHandler_resetLock;

#define BUTTON_TIMEOUT 2000//(38) //~500[ms]
#define NO_STEP_SELECTED -1
/**indicates which of the 4 select button modes is active.
0 = voice select
select the active voice
sequencer buttons set steps

1 = mute
mute the different voices
sequencer buttons used for performance?
manual roll? pattern selection?

2 = sequencer mode
select buttons controll 8 substeps for each step
sequencer buttons display the 16 main steps and can be used to access step specific settings (probability, velocity etc)

3 = LFOs
*/
/*
#define SELECT_MODE_VOICE		0x00
#define SELECT_MODE_PERF		0x01
#define SELECT_MODE_LOAD		0x02
#define SELECT_MODE_SAVE		0x03

#define SELECT_MODE_STEP		0x04
#define SELECT_MODE_PAT_GEN		0x05
//#define SELECT_MODE_SOM_GEN		0x06
#define SELECT_MODE_MENU		0x07
#define SELECT_MODE_PATTERN		0x08
*/

enum SelectModes
{
	SELECT_MODE_VOICE,
	SELECT_MODE_PERF,
	SELECT_MODE_MOD,

	SELECT_MODE_LOAD,
	SELECT_MODE_SAVE,
#if !EURORACK_MODULE
	SELECT_MODE_STEP,
	SELECT_MODE_PAT_GEN,
#endif
	SELECT_MODE_FX,

	SELECT_MODE_MENU,

	SELECT_MODE_SAVE_MORPH,
#if !EURORACK_MODULE
	SELECT_MODE_PATTERN,
	SELECT_MODE_SONG,

	SELECT_MODE_LAST_STEP_TRACK,
	SELECT_MODE_LAST_STEP_ALL,
#endif

};
		
enum SeqButtonFunctions
{
	SEQ_DRUM1,
	SEQ_DRUM2,
	SEQ_DRUM3,
	SEQ_SNARE,
	SEQ_CLP,
	SEQ_CL_HH,
	SEQ_OP_HH,
	SEQ_FX,
#if !EURORACK_MODULE
	SEQ_KIT,
	SEQ_MORPH,
	SEQ_SONG,
	SEQ_PROJECT,
	SEQ_CONFIG,
	SEQ_RELOAD,
	SEQ_LAST_STEP,

	SEQ_PATTERN,
#endif
};

enum ButtonNumbers
{
#if EURORACK_MODULE
	BUT_EDIT,
	BUT_PERF, //performance
	BUT_MOD, //MOD
	BUT_FX, //FX

	BUT_1,
	BUT_2,
	BUT_3,
	BUT_4,
	BUT_5,
	BUT_6,
	BUT_7,
	BUT_8,

	BUT_SHIFT,
	BUT_SAVE_LOAD




#else
	BUT_PAGE_UP,
	BUT_VOICE_1,
	BUT_VOICE_2,
	BUT_VOICE_3,
	BUT_VOICE_4,
	BUT_VOICE_5,
	BUT_VOICE_6,
	BUT_VOICE_7,



	BUT_SELECT5,
	BUT_SELECT6,
	BUT_SELECT7,
	BUT_SELECT8,
	BUT_SEQ16,
	BUT_SEQ15,
	BUT_SEQ14,
	BUT_SEQ13,

	BUT_SELECT1,
	BUT_SELECT2,
	BUT_SELECT3,
	BUT_SELECT4,
	BUT_SEQ12,
	BUT_SEQ11,
	BUT_SEQ10,
	BUT_SEQ9,

	BUT_MODE1 ,
	BUT_MODE2 ,
	BUT_MODE3 ,
	BUT_MODE4 ,
	BUT_SEQ8,
	BUT_SEQ7,
	BUT_SEQ6,
	BUT_SEQ5,
	
	BUT_SHIFT,
	BUT_START_STOP,
	BUT_REC,
	BUT_COPY,
	BUT_SEQ4,
	BUT_SEQ3,
	BUT_SEQ2,
	BUT_SEQ1,

	BUT_PAGE_DOWN	//not in shift register!
#endif
};

//--------------------------------------------------------
void buttonHandler_init();
//--------------------------------------------------------
void buttonHandler_tick();
//--------------------------------------------------------
void buttonHandler_buttonPressed(uint8_t buttonNr);
//--------------------------------------------------------
uint8_t buttonHandler_getMutedVoices();
//--------------------------------------------------------
void buttonHandler_buttonReleased(uint8_t buttonNr);
//--------------------------------------------------------
//void buttonHandler_toggleEuklidMode();
//--------------------------------------------------------
/** return the menu state selected by mode2 button*/
uint8_t buttonHandler_getMode();
//--------------------------------------------------------
void buttonHandler_setMode(uint8_t mode);
//--------------------------------------------------------


#if !EURORACK_MODULE
uint8_t buttonHandler_getCopy();
//--------------------------------------------------------
uint8_t buttonHandler_getShift();

//--------------------------------------------------------
uint8_t buttonHandler_getLoad();
//--------------------------------------------------------
uint8_t buttonHandler_getSave();
//--------------------------------------------------------
int8_t buttonHandler_getArmedAutomationStep();
#else
uint8_t buttonHandler_getDrumButton();
uint8_t buttonHandler_getManualButton();
uint8_t buttonHandler_getSaveButton ();
uint8_t buttonHandler_getShiftButton ();
//--------------------------------------------------------
#endif
//--------------------------------------------------------
void buttonHandler_muteVoice(uint8_t voice, uint8_t isMuted);
//--------------------------------------------------------
void buttonHandler_showMuteLEDs();
//--------------------------------------------------------
void buttonHandler_setRunStopState(uint8_t running);
#endif /* BUTTONHANDLER_H_ */
