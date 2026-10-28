/*
 * VelocityModulation.c
 *
 *  Created on: 06.01.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#include "Soundengine/DSPAudio/modulationNode.h"
#include "Soundengine/DSPAudio/DrumVoice.h"
#include "Soundengine/DSPAudio/CymbalVoice.h"
#include "Soundengine/DSPAudio/HiHat.h"
#include "Soundengine/DSPAudio/Snare.h"
//#include "Soundengine/Sequencer/sequencer.h"
#include "menu/menu.h"
#include "Soundengine/MIDI/MidiMessages.h"
#include "Soundengine/MIDI/MidiParser.h"
#include "frontPanelParser_ui.h"
#include "Preset/PresetManager.h"

//INDTCMZ ModulationNode velocityModulators[6];

 //-----------------------------------------------------------------------
void modNode_init(ModulationNode* vm)
{
	vm->lastVal = 0;
	vm->amount = 0.f;
	modNode_setDestination(vm,0);
}
//-----------------------------------------------------------------------
static void modNode_setOriginalValueChanged(ModulationNode* vm, uint16_t idx)
{
	// TODO this code is repeated (approximately. not exactly) inside modNode_setDestination. can we combine?


	if(vm->destination == idx) { //parameter is active in this modulator
		const Parameter *p=&parameterArray[idx];

			//update orig value
			switch(p->type)
			{
				case TYPE_UINT8:
					vm->originalValue.itg = *((uint8_t*)p->ptr);
					break;

				case TYPE_SPECIAL_F:
					break;

				case TYPE_SPECIAL_P:
				case TYPE_SPECIAL_FILTER_F:
				case TYPE_FLT:
					vm->originalValue.flt = *((float*)p->ptr);
					break;

				case TYPE_UINT32:
					vm->originalValue.itg = *((uint32_t*)p->ptr);
					break;

				default:
					break;
			}
		}
}
//-----------------------------------------------------------------------
// This is called when a user changes a parameter value on the front. It saves
// the new value as originalValue. Since the value changes as modulation happens,
// we need to restore to the original value from time to time
void modNode_originalValueChanged(uint16_t idx)
{
	uint8_t i;
#if OLD_LFO_ROUTING
	for(i=0;i<6;i++)
	{
		modNode_setOriginalValueChanged(&velocityModulators[i],idx);
	}
#endif
	//CV mods
	for(i=0;i<NUM_MOD_SLOTS;i++)
	{
		modNode_setOriginalValueChanged(&cvModulators[i],idx);
	}

#if OLD_LFO_ROUTING
	modNode_setOriginalValueChanged(&voiceArray[0].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&voiceArray[1].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&voiceArray[2].lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&snareVoice.lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&cymbalVoice.lfo.modTarget,idx);
	modNode_setOriginalValueChanged(&hatVoice.lfo.modTarget,idx);
#endif
}
//-----------------------------------------------------------------------
IN_ITCM_RAM void modNode_resetTargets()
{

	uint8_t i;
#if OLD_LFO_ROUTING
	for(i=0;i<6;i++)
	{
		paramArray_setParameter(velocityModulators[i].destination,velocityModulators[i].originalValue);
	}
#endif
	//CV mods
	for(i=0;i<NUM_MOD_SLOTS;i++)
	{
		if(cvModulators[i].destination != 0)
			paramArray_setParameter(cvModulators[i].destination,cvModulators[i].originalValue);
	}
#if OLD_LFO_ROUTING
	paramArray_setParameter(voiceArray[0].lfo.modTarget.destination,voiceArray[0].lfo.modTarget.originalValue);
	paramArray_setParameter(voiceArray[1].lfo.modTarget.destination,voiceArray[1].lfo.modTarget.originalValue);
	paramArray_setParameter(voiceArray[2].lfo.modTarget.destination,voiceArray[2].lfo.modTarget.originalValue);
	paramArray_setParameter(snareVoice.lfo.modTarget.destination,snareVoice.lfo.modTarget.originalValue);
	paramArray_setParameter(cymbalVoice.lfo.modTarget.destination,cymbalVoice.lfo.modTarget.originalValue);
	paramArray_setParameter(hatVoice.lfo.modTarget.destination,hatVoice.lfo.modTarget.originalValue);
#endif

}
//-----------------------------------------------------------------------
#if OLD_LFO_ROUTING
IN_ITCM_RAM void modNode_reassignVeloMod()
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		modNode_updateValue(&velocityModulators[i], velocityModulators[i].lastVal);
	}
}
#endif
//-----------------------------------------------------------------------
// set a modulation destination to one of the sound parameters.
// This is called when the mod target changes or is initialized.
// The target's actual value needs to be preserved because it will be modulated.
void modNode_setDestination(ModulationNode* vm, uint16_t dest)
{
	//TODO check if this interrupts other modulations too much
	//is needed to really get the original value
	modNode_resetTargets();

	//restore old value to original TODO isn't this already being done above for all nodes?
	//paramArray_setParameter(vm->destination,vm->originalValue);

	//update dest param
	vm->destination = dest;

	const Parameter *p = &parameterArray[vm->destination];

	// **PATROT we want to avoid reading from invalid memory
	if(!p->ptr)
		return;

	//get new original parameter value
	switch(p->type)
	{
		case TYPE_NULL:
			//should not get here, but somehow it happens with not set parameterArray fields
			break;

		case TYPE_UINT8:
			vm->originalValue.itg = *((uint8_t*)p->ptr);
			break;
		case TYPE_SPECIAL_F: // TODO whats up with this???
			vm->originalValue.flt = 1;//*((float*)parameterArray[vm->destination].ptr);
			break;

		case TYPE_SPECIAL_FILTER_F:
		case TYPE_FLT:
			vm->originalValue.flt = *((float*)p->ptr);
			break;

		case TYPE_UINT32:
			vm->originalValue.itg = *((uint32_t*)p->ptr);
			break;

		case TYPE_SPECIAL_P:
		default:
			break;
	}
}
//-----------------------------------------------------------------------
// This is called to actually modulate the value for a modulation node
void modNode_updateValue(ModulationNode* vm, float val)
{
	Parameter const *p = &parameterArray[vm->destination];

	vm->lastVal = val;

	// **PATROT avoid setting this if it's not set to something good
	if(!p->ptr)
		return;

#if 1

	MidiMsg msg;

	if(vm->destination == PAR_MORPH)
	{
		float originalValue = activeParameters[vm->destination]/255.f;
		//val is [0:1] make [-1:1] for bipolar modulation around original value
		float newValue = originalValue + vm->amount * (val*2.f-1.f);
		newValue = newValue < 0.f ? 0 : newValue > 1.f ? 1.f : newValue;


		//*((uint8_t*)p->ptr)
		//preset_morphValue = newValue*255;
		preset_morph(newValue*255, 0);
	}
	else
	{
		float originalValue = activeParameters[vm->destination]/127.f;
		//val is [0:1] make [-1:1] for bipolar modulation around original value

		float newValue;
#if 0
		if((vm->destination >= PAR_CV_1VOCT_1) && (vm->destination <= PAR_CV_1VOCT_6))
		{
			newValue = originalValue + vm->amount * (val);
			newValue = newValue < 0.f ? 0 : newValue > 1.f ? 1.f : newValue;
		}
		else
#endif
		{
			newValue = originalValue + vm->amount * (val*2.f-1.f);
			newValue = newValue < 0.f ? 0 : newValue > 1.f ? 1.f : newValue;
		}



		if(vm->destination < 128)
		{
			/* comes from cortex midi handler
			 * stupid offset!
			 * TODO fix this stupid offset			 *
			 * 		frontParser_midiMsg.data1 += 1;

					//because hh slope on front is 127 and on cortex is 0 wrap data1 at 127
					frontParser_midiMsg.data1 &= 0x7f;
			 *
			 */
			msg.status = MIDI_CC;
			msg.data1 = vm->destination;

			msg.data1 += 1;
			//msg.data1 &= 0x7f;
			midiParser_ccHandler(msg,0, 1, newValue);


		}
		else if (vm->destination <= 255 )
		{
			msg.status = CC_2;
			msg.data1 = vm->destination-128; //stupid offset only in CC not CC2

			//msg.data1 &= 0x7f;
			midiParser_ccHandler(msg,0, 1, newValue);

		}
		else
		{
#pragma message "nothing over 255 is morphed (mod matrix stuff)"
		}
	}

#else
	//old scheme with unipolar hack modulation :-p
	switch(p->type)
	{
	case TYPE_UINT8:
		(*((uint8_t*)p->ptr)) = (*((uint8_t*)p->ptr)) * vm->amount * val + (1.f-vm->amount) * (*((uint8_t*)p->ptr));
		break;

	case TYPE_UINT32:
		(*((uint32_t*)p->ptr)) = (*((uint32_t*)p->ptr)) * vm->amount * val + (1.f-vm->amount) * (*((uint32_t*)p->ptr));
		break;

	case TYPE_FLT:
		(*((float*)p->ptr)) = (*((float*)p->ptr)) * vm->amount * val + (1.f-vm->amount) * (*((float*)p->ptr));
		break;

	case TYPE_SPECIAL_F:
		(*((float*)p->ptr)) = (*((float*)p->ptr)) * vm->amount * val + (1.f-vm->amount) * (*((float*)p->ptr));
		break;

	case TYPE_SPECIAL_P:
	case TYPE_SPECIAL_FILTER_F:
	default:
		break;

	}
#endif
}

