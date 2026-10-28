/*
 * VerbModDelay.c
 *
 *  Created on: 31.08.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/DSPAudio/FX/VerbTools/VerbModDelay.h"
#include "Soundengine/DSPAudio/FX/Reverb.h"
#include <math.h>

void VerbModDelay_init(VerbModDelay *d)
{
	d->readPtr=0;
	d->length = 256;
	d->writePtr=d->length;

	initOnePole(&d->lp);
	setOnePoleFreq(&d->lp, 0.0001308911269084528);
	d->roomSz = 1;
	d->frac = 0;
	d->depth=0;
	d->modVal=0;
}

inline uint32_t VerbModDelay_getMaxSize(VerbModDelay *d)
 {
   return MAX_DELAY_SIZE;
 }

 inline uint32_t VerbModDelay_getLength(VerbModDelay *d)
 {
   return d->length;
 }

 void VerbModDelay_setLength(VerbModDelay *d, uint32_t l)
 {
	 d->length = l;
 }

 inline void VerbModDelay_setRoomSize(VerbModDelay *d, float sz)
 {
	 d->roomSz = sz;
	 VerbModDelay_calcLength(d);
 }

 inline void VerbModDelay_setModDepth(VerbModDelay *d, float dep)
 {
	 d->depth = dep;

 }

 inline void VerbModDelay_setModValue(VerbModDelay *d, float v)
 {
	 d->modVal = v;
	 VerbModDelay_calcLength(d);
 }

 inline float VerbModDelay_read(VerbModDelay *d)
 {
	  uint32_t r = d->readPtr;
	  __fp16 result = d->ramBuffer[r];
     d->readPtr = (r + 1) & (MAX_DELAY_SIZE - 1);
     return (float)result;
 }

 inline float VerbModDelay_readNext(VerbModDelay *d)
 {
	  uint32_t r = d->readPtr;
     r = (r + 1) & (MAX_DELAY_SIZE - 1);
     __fp16 result = d->ramBuffer[r];

     return (float)result;
 }

 inline void VerbModDelay_write(VerbModDelay *d,float in) {
	  uint32_t w = d->writePtr;
	  d->ramBuffer[w] = (__fp16)in;
	  d->writePtr = (w + 1) & (MAX_DELAY_SIZE - 1);
 }


 float fnms(float a, float b, float c)
{
	return c - a*b;
}

float lerp(float a, float b, float t) {
  //return a + t * (b - a);
	//fma(a, b, c) = a*b + c
	//fnms(a, b, c) = -(a*b - c) = c - a*b
	return fma(t, b, fnms(t, a, a));

}

 float  interpolated_read( float a, float b, float x) {
	return a*(1-x) + b*(x);
 }

 inline void VerbModDelay_calcLength(VerbModDelay *d)
 {
   float scaledSize = d->length*d->roomSz;
   float filteredPos = calcOnePole(&d->lp, scaledSize + d->depth*(d->modVal));

   float pos = d->writePtr-filteredPos;
   if(pos<0)pos+=MAX_DELAY_SIZE;
   uint32_t itg = pos;
   d->frac = pos-itg;
   d->readPtr  = itg & (MAX_DELAY_SIZE - 1);
 }

 inline float VerbModDelay_interpolatedRead(VerbModDelay *d)
 {
   float nextSample = VerbModDelay_readNext(d);
   float out = lerp(VerbModDelay_read(d), nextSample, d->frac);
   return out;

 }

 inline float VerbModDelay_interpolatedReadTap(VerbModDelay *d, float percent)
 {
	uint32_t offset = percent/100.f * d->length;
	uint32_t r1 = d->readPtr;
	uint32_t r2 = d->readPtr;
	r1 = (r1 - offset) & (MAX_DELAY_SIZE - 1);
	r2 = (r1 - offset +1) & (MAX_DELAY_SIZE - 1);

   float out = lerp((float)d->ramBuffer[r1], (float)d->ramBuffer[r2], d->frac);
   return out;

 }

 inline float VerbModDelay_process(VerbModDelay *d, const float input)
 {
   //calc position
//	 VerbModDelay_calcLength(d);

   //process delay
	 VerbModDelay_write(d, input);

     float nextSample = VerbModDelay_readNext(d);
     float out = lerp(VerbModDelay_read(d), nextSample, d->frac);
    // int16_t out = interpolated_read(read(), nextSample, frac);
    // int16_t out = read();

     return out;
 }

 inline void VerbModDelay_setBuffer(VerbModDelay *d,volatile __fp16 *buffer)
 {
	  d->ramBuffer = buffer;
 }
