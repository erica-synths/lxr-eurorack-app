/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */



#ifndef GLOBAL_VARS_FILE
#define GLOBAL_VARS_FILE

#include "stm32f7xx.h"
#include "Soundengine/config.h"
// global vars
extern INDTCMZ volatile uint32_t systick_ticks;
//extern int16_t audioOutBuffer[2];
extern volatile  uint8_t bCurrentSampleValid;
#define SAMPLE_VALID  0xff
#define FILTER_SHAPER -0.9f
/*
typedef union
{
	int16_t word[2];
	int32_t doubleword;
}simdBuffer;
*/

#endif
