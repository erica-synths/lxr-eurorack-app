/*
 * FxRamBuffer.h
 *
 *  Created on: 30.08.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_SOUNDENGINE_DSPAUDIO_FX_FXRAMBUFFER_H_
#define INC_SOUNDENGINE_DSPAUDIO_FX_FXRAMBUFFER_H_

#include "Soundengine/globals.h"
#include "Soundengine/config.h"

//32768 * 2 = 65536 byte = 64kb => 0,74	sec mono delay or 0,37	sec stereo delay
#define FX_BUFFER_SIZE 0x7FFF//32768 //must be power of to for masking!

//For delay an in16_t buffer would be more performant, but since the reverb uses flt16, we can save on an initial buffer clear to prevent NaN injection poisoning the buffer
extern INDTCMZ __fp16 FX_BUFFER[FX_BUFFER_SIZE+1];

//because we have 17 delay units using the memory in the reverb
//must be masking compatible pow2-1
#define MAX_ALLPASS_SIZE 0x800//(FX_BUFFER_SIZE/18)

#endif /* INC_SOUNDENGINE_DSPAUDIO_FX_FXRAMBUFFER_H_ */
