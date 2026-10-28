/*
 * SeqStep.h
 *
 * Created: 26.06.2012 16:46:31
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef SEQSTEP_H_
#define SEQSTEP_H_
#include "main.h"

/** a copy of the step struct from the sequencer.h file from the cortex src
used to load step data from sequencer and write it to the pattern preset*/
typedef struct StepStruct
{
	//uint8_t  	active;		// bit 0: on/off
	uint8_t 	volume;		// 0-127 volume -> 0x7f => lower 7 bit, upper bit => active <- active c an probably be removed
	uint8_t  	prob;		//step probability 7 bit [0:100]
	uint8_t		note;		// 7 bit

	//parameter automation
	uint8_t 	param1Nr;
	uint8_t 	param1Val; //probably 7

	uint8_t 	param2Nr;
	uint8_t 	param2Val; //probably 7
}Step;

typedef struct StepTiming
{
	uint8_t flam:4; // [0:5] => off, x2, x3, x4, x6, x8
	uint8_t shift:4; // [-7:7]
}StepTiming;


typedef enum StepPositionEnum
{
	STEP_PREV,
	STEP_CURRENT,
	STEP_NEXT
}StepPosition;

/*
typedef union
{
	uint32_t completeData;
	struct
	{
		uint8_t unused;
		uint8_t prevStep;
		uint8_t currentStep;
		uint8_t nextStep;
	}StepData;
}StepPattern;
*/





#endif /* SEQSTEP_H_ */
