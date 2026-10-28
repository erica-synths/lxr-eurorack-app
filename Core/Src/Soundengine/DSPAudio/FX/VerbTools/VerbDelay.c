/*
 * VerbDelay.c
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
#include "Soundengine/DSPAudio/FX/VerbTools/VerbDelay.h"


void VerbDelay_init(VerbDelay* d)
{
    d->readPtr=0;
    d->length = 256;
    d->writePtr=d->length;
    d->roomSz = 1;
}

inline uint16_t VerbDelay_getMaxSize(VerbDelay* d)
 {
   return MAX_ALLPASS_SIZE;
 }

 inline uint16_t VerbDelay_getLength(VerbDelay* d)
 {
   return d->length;
 }

 void VerbDelay_setLength(VerbDelay* d, uint16_t l)
 {
	 d->length = l;
	 VerbDelay_calcLength(d);
   /*
   uint16_t pos = writePtr-length;
   readPtr  = pos & (size - 1);
   */
 }

 inline void VerbDelay_setRoomSize(VerbDelay* d, float sz)
 {
	 d->roomSz = sz;
	 VerbDelay_calcLength(d);
 }

 inline float VerbDelay_read(VerbDelay* d)
 {
     __fp16 result = d->ramBuffer[d->readPtr];
     d->readPtr = (d->readPtr + 1) & (MAX_ALLPASS_SIZE - 1);
     return (float)result;
 }


 inline void VerbDelay_write(VerbDelay* d, float in) {
     d->ramBuffer[d->writePtr] = (__fp16)in;
     d->writePtr = (d->writePtr + 1) & (MAX_ALLPASS_SIZE - 1);
 }

 inline float VerbDelay_ReadTap(VerbDelay* d, float percent)
 {
   const uint16_t offset = percent/100.f * d->length;
   uint16_t r1 = d->readPtr;
   r1 = (r1 - offset) & (MAX_ALLPASS_SIZE - 1);
   __fp16 out = d->ramBuffer[r1];
   return (float)out;

 }

 float VerbDelay_ReadTapSample(VerbDelay* d, uint16_t idx)
 {

   uint16_t r1 = d->readPtr;
   r1 = (r1 - idx) & (MAX_ALLPASS_SIZE - 1);
   __fp16 out = d->ramBuffer[r1];
   return (float)out;
 }


 inline float VerbDelay_process(VerbDelay* d, const float input)
 {
	// VerbDelay_calcLength(d);
   //process delay
	 VerbDelay_write(d, input);
     return VerbDelay_read(d);
 }

 inline void VerbDelay_setBuffer(VerbDelay* d, volatile __fp16 *buffer)
 {
	 d->ramBuffer = buffer;
 }
 inline void VerbDelay_calcLength(VerbDelay* d)
 {
   const float scaledSize = d->length*d->roomSz;
   float pos = d->writePtr-scaledSize;
   if(pos<0)pos+=MAX_ALLPASS_SIZE;
   const uint16_t itg = pos;
   d->readPtr  = itg & (MAX_ALLPASS_SIZE - 1);
 }
