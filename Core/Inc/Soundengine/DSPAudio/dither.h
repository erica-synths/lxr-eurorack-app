/*
 * dither.h
 *
 * music dsp source code archive
 *
 *	paul kellett
 *	http://www.musicdsp.org/files/nsdither.txt
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */




#ifndef DITHER_H_
#define DITHER_H_
/*
Noise shaped dither  (March 2000)
-------------------

This is a simple implementation of highpass triangular-PDF dither with
2nd-order noise shaping, for use when truncating floating point audio
data to fixed point.

The noise shaping lowers the noise floor by 11dB below 5kHz (@ 44100Hz
sample rate) compared to triangular-PDF dither. The code below assumes
input data is in the range +1 to -1 and doesn't check for overloads!

To save time when generating dither for multiple channels you can do
things like this:  r3=(r1 & 0x7F)<<8; instead of calling rand() again.
*/

#include "stm32f7xx_hal.h"
#include "random.h"

#define BITS 16
#ifndef RAND_MAX
#define RAND_MAX 32767
#endif
#define DITHER_S (0.5f)              			//set to 0.0f for no noise shaping
#define DITHER_W 32768.f//(pow(2.0,BITS-1))   	//word length (usually bits=16)
#define DITHER_WI (1.0f/DITHER_W)
#define DITHER_D (DITHER_WI / RAND_MAX)     	//dither amplitude (2 lsb)
#define DITHER_O (DITHER_WI * 0.5f)         	//remove dc offset

typedef struct Dither_Struct
{
	  int16_t   r1, r2;                			//rectangular-PDF random numbers
	  float s1, s2;                				//error feedback buffers

	  float in, tmp;
	  int16_t   out;
} Dither;

int16_t dither_process(Dither* dither, float in);

#endif /* DITHER_H_ */
