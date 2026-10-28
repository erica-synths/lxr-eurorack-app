/*
 * PresetManager.h
 *
 * Created: 09.05.2012 14:11:49
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef PRESETMANAGER_H_
#define PRESETMANAGER_H_

#include "main.h"
#include "string.h"
#include "fatfs.h"
#include "Parameters.h"
#include "InitKit.h"


// autosave timer
// after a kit/pat is changed, wait a while, then autosave
typedef enum
{
	PRESET_KIT_TIMER,
} PresetTimer;


FRESULT preset_LoadTmpKit();
void preset_initTmpFolder();
void preset_touchTimer(PresetTimer type);
void preset_processTimer();
//FRESULT preset_clearTmpFolder();
void preset_resetAllSaveTimers();
//load all manually saved kits and patterns found in the tmp folder
FRESULT loadTmpFolderFiles();

//#define f_unmount(path) f_mount(0, path, 0)

enum
{
	KIT_NAMES,
	PATTERN_NAMES,
	SONG_NAMES,
	PROJECT_NAMES,
	NUM_SAVEFILE_NAMES,
	UNUSED_NAMES
};

extern uint8_t preset_morphValue;
extern char savefileNames[NUM_KIT_SLOTS][NUM_SAVEFILE_NAMES][8];
extern char currentFolder[13];
//extern char presetNameBuffer[8];
//extern uint8_t activePresetNr; //currently loaded kit

void preset_init(uint8_t presetNr);

#define SKIP_NEXT_KIT_AUTOLOAD 2


/** save the parameters of all 6 voices + LFO settings to a file on the SD card 
if isMorph==1 the sound will be loaded into the morph buffer
*/

//load/save drumset from/to SD card
FRESULT preset_saveDrumsetSD(uint8_t presetNr, uint8_t isMorph);
FRESULT preset_loadDrumsetSD(uint8_t presetNr);
//copy drumset to/from edit buffer
void preset_loadDrumsetRam(uint8_t presetNr, uint8_t isMorph, uint8_t sendImmediate);
void preset_saveDrumsetRam(uint8_t presetNr, uint8_t isMorph);


FRESULT preset_saveGlobals();
uint8_t preset_loadGlobals();
void preset_createGlobals();

FRESULT preset_loadLastUsedPresetNrFromSD(uint8_t* presetNr);
FRESULT preset_saveLastUsedPresetNrToSD(uint8_t presetNr);

void preset_loadDefaultGlobals();

#if !EURORACK_MODULE
FRESULT preset_saveProject(uint8_t presetNr);
FRESULT preset_loadProject(uint8_t presetNr);
FRESULT preset_createProject(uint8_t presetNr);
void preset_loadProjectNames(uint8_t projectNr);
#endif

#if !EURORACK_MODULE
FRESULT preset_loadSongSD(uint8_t songNr);
FRESULT preset_saveSongSD(uint8_t songNr);
#endif
//char* preset_loadName(uint8_t presetNr, uint8_t nameBufferIdx, uint8_t what);

/** save a pattern set to the sd card */


FRESULT preset_savePatternRAM(uint8_t presetNr);
FRESULT preset_loadPatternRAM(uint8_t presetNr);


//uint8_t preset_loadPatternTemp(uint8_t presetNr);
//void preset_savePatternTemp(uint8_t presetNr);
/** morph pattern linear to �pattern buffer 2*/
void preset_morph(uint8_t morph, uint8_t forceResetIndex);
void preset_morph_tick(); //handler for non blocking morph
uint8_t preset_getMorphValue(uint16_t index, uint8_t morph);
void preset_sendDrumsetParameters();
void preset_sendDrumsetParametersImmediate();

uint8_t preset_isNameEmpty(char* name);

#endif /* PRESETMANAGER_H_ */
