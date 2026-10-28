/*
 * dither.c
 *
 *  Created on: 20.06.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

/*
 This is a simple implementation of highpass triangular-PDF dither (a good general-purpose
dither) with optional 2nd-order noise shaping (which lowers the noise floor by 11dB below
0.1 Fs).
The code assumes input data is in the range +1 to -1 and doesn't check for overloads!

To save time when generating dither for multiple channels you can re-use lower bits of a
previous random number instead of calling rand() again. e.g. r3=(r1 & 0x7F)<<8;
 */

#include "Soundengine/DSPAudio/dither.h"

int16_t dither_process(Dither* dither, float in)
{

	dither->r2 = dither->r1;                               						//can make HP-TRI dither by
	dither->r1 = GetRngValue();//rand();                          				//subtracting previous rand()

	in += DITHER_S * (dither->s1 + dither->s1 - dither->s2);            		//error feedback
	dither->tmp = in + DITHER_O + DITHER_D * (float)(dither->r1 - dither->r2); 	//dc offset and dither

	dither->out = (int)(DITHER_W * dither->tmp);                				//truncate downwards
	if(dither->tmp<0.0f) dither->out--;                  						//this is faster than floor()

	dither->s2 = dither->s1;
	dither->s1 = in - DITHER_WI * (float)dither->out;           				//error

	return dither->out;
}
