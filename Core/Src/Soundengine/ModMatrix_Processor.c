/*
 * CV_Processor.c
 *
 *  Created on: Apr 20, 2022
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include <Soundengine/ModMatrix_Processor.h>
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/modulationNode.h"
#include "IO/adcPots.h"
#include "menu/menu.h"
#include "Soundengine/DSPAudio/CymbalVoice.h"
#include "Soundengine/DSPAudio/DrumVoice.h"
#include "Soundengine/DSPAudio/Snare.h"
#include "Soundengine/DSPAudio/HiHat.h"
#include <Soundengine/TriggerProcessor.h>

INDTCMZ ModulationNode cvModulators[NUM_MOD_SLOTS];

void CV_Init()
{
	for(uint8_t i=0;i<NUM_MOD_SLOTS;i++)
	{
		modNode_init(&cvModulators[i]);
	}
}

void CV_process()
{

	for(uint8_t i=0;i<NUM_MOD_SLOTS;i++)
	{
		if(activeParameters[PAR_CV_MOD_AMT_1 + i] == 0) continue; //skip zero amount modulations
		cvModulators[i].amount = (activeParameters[PAR_CV_MOD_AMT_1 + i])/100.f;


		float cv = 0;

		switch(activeParameters[PAR_CV_SRC_1 + i])
		{
		case 0: //{"cv1"},
		case 1: //{"cv2"},
		case 2: //{"cv3"},
		case 3: //{"cv4"},
		case 4: //{"cv5"},
			cv = adc_getCvValue(activeParameters[PAR_CV_SRC_1 + i]);

			break;

		case 5: //{"lf1"},
			cv = voiceArray[0].lfo.value;
			break;

		case 6: //{"lf2"},
			cv = voiceArray[1].lfo.value;
			break;
		case 7: //{"lf3"},
			cv = voiceArray[2].lfo.value;
			break;
		case 8: //{"lf4"},
			cv = snareVoice.lfo.value;
			break;
		case 9: //{"lf5"},
			cv = cymbalVoice.lfo.value;
			break;
		case 10: //{"lf6"},
			cv = hatVoice.lfo.value;
			break;

		case 11: //{"ac1"},
			cv = Trigger_getAccent(0)/127.f;//voiceArray[0].velocity;
			break;
		case 12: //{"ac2"},
			cv = Trigger_getAccent(1)/127.f;//voiceArray[1].velocity;
			break;
		case 13: //{"ac3"},
			cv = Trigger_getAccent(2)/127.f;//voiceArray[2].velocity;
			break;
		case 14: //{"ac4"},
			cv = Trigger_getAccent(3)/127.f;//snareVoice.velocity;
			break;
		case 15: //{"ac5"},
			cv = Trigger_getAccent(4)/127.f;//cymbalVoice.velocity;
			break;
		case 16: //{"ac6"}
			cv = Trigger_getAccent(5)/127.f;//hatVoice.velocity;
			break;

			break;
		}

		//float cv = adc_getCvValue(activeParameters[PAR_CV_SRC_1 + i]);
		modNode_updateValue(&cvModulators[i],cv);
	}
}

float CV_getBpm()
{
 return 0.f;
}
