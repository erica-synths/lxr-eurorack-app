/*
 * VerbModDelay.h
 *
 *  Created on: 30.07.2014
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef VERBMODDELAY_H_
#define VERBMODDELAY_H_

#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/1PoleLp.h"

//#define MIN(a,b) (((a)<(b))?(a):(b))
//#define MAX(a,b) (((a)>(b))?(a):(b))


typedef struct VerbModDelayStruct
{
  volatile __fp16 *ramBuffer;
  uint32_t length;

  uint32_t readPtr;
  uint32_t writePtr;
  OnePoleFilter lp;
  float frac;
  float depth;
  float modVal;
  float roomSz;
} VerbModDelay;

void VerbModDelay_init(VerbModDelay *d);
uint32_t VerbModDelay_getMaxSize(VerbModDelay *d);
uint32_t VerbModDelay_getLength(VerbModDelay *d);
void VerbModDelay_setLength(VerbModDelay *d, uint32_t l);
void VerbModDelay_setRoomSize(VerbModDelay *d, float sz);
void VerbModDelay_setModDepth(VerbModDelay *d, float dep);
void VerbModDelay_setModValue(VerbModDelay *d, float v);
float VerbModDelay_read(VerbModDelay *d);
float VerbModDelay_readNext(VerbModDelay *d);
void VerbModDelay_write(VerbModDelay *d,float in);
void VerbModDelay_calcLength(VerbModDelay *d);
float VerbModDelay_interpolatedRead(VerbModDelay *d);
float VerbModDelay_interpolatedReadTap(VerbModDelay *d, float percent);
float VerbModDelay_process(VerbModDelay *d, const  float in);
void VerbModDelay_setBuffer(VerbModDelay *d,volatile __fp16 *buffer);
#if 0
template<typename T, uint32_t size>
class VerbModDelay
{
public:
  VerbModDelay()
  {
    readPtr=0;
    length = 256;
    writePtr=length;
    /*
    for(int i=0;i<size;i++) {
    	ramBuffer[i]=0;
    }
    */
    lp.setFreq(0.0001308911269084528);
    roomSz = 1;
    frac = 0;
    depth=0;
    modVal=0;
  };
  ~VerbModDelay(){};

  inline uint32_t getMaxSize()
  {
    return size;
  }

  inline uint32_t getLength()
  {
    return length;
  }

  void setLength(uint32_t l)
  {
    length = l;
  }

  inline void setRoomSize(float sz)
  {
    roomSz = sz;
  }

  inline void setModDepth(float d)
  {
    depth = d;
  }

  inline void setModValue(float v)
  {
    modVal = v;
  }

  inline T read()
  {
	  uint32_t r = readPtr;
      T result = ramBuffer[r];
      readPtr = (r + 1) & (size - 1);
      return result;
  }

  inline T readNext()
  {
	  uint32_t r = readPtr;
      r = (r + 1) & (size - 1);
      T result = ramBuffer[r];

      return result;
  }

  inline void write(T in) {
	  uint32_t w = writePtr;
      ramBuffer[w] = in;
      writePtr = (w + 1) & (size - 1);
  }

  T inline interpolated_read(T a, T b, float x) {
	return a*(1-x) + b*(x);
  }

  inline void calcLength()
  {
    float scaledSize = length*roomSz;
    float filteredPos = lp.process(scaledSize + depth*(modVal));

    float pos = writePtr-filteredPos;
    if(pos<0)pos+=size;
    uint32_t itg = pos;
    frac = pos-itg;
    readPtr  = itg & (size - 1);
  }

  inline T interpolatedRead()
  {
    T nextSample = readNext();
    T out = interpolated_read(read(), nextSample, frac);
    return out;

  }

  inline T interpolatedReadTap(float percent)
  {
	uint32_t offset = percent/100.f * length;
	uint32_t r1 = readPtr;
	uint32_t r2 = readPtr;
	r1 = (r1 - offset) & (size - 1);
	r2 = (r1 - offset +1) & (size - 1);

    T out = interpolated_read(ramBuffer[r1], ramBuffer[r2], frac);
    return out;

  }

  inline T process(const T input)
  {
    //calc position
    calcLength();

    //process delay
      write(input);

      T nextSample = readNext();
      T out = interpolated_read(read(), nextSample, frac);
     // T out = interpolated_read(read(), nextSample, frac);
     // T out = read();

      return out;
  }

  inline void setBuffer(volatile T *buffer)
  {
	  ramBuffer = buffer;
  }

private:
  volatile T *ramBuffer;
  uint32_t length;

  uint32_t readPtr;
  uint32_t writePtr;
  OnePoleLp lp;
  float frac;
  float depth;
  float modVal;
  float roomSz;


};
#endif
#endif /* VERBMODDELAY_H_ */
