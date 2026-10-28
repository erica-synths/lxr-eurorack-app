/*
  ==============================================================================

    Delay.h
    Created: 21 Jul 2014 7:30:01pm
    Author:  julian

  ==============================================================================
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
*/

#ifndef VERB_DELAY_H_INCLUDED
#define VERB_DELAY_H_INCLUDED

#include "Soundengine/globals.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/FX/FxRamBuffer.h"

//#define MIN(a,b) (((a)<(b))?(a):(b))
//#define MAX(a,b) (((a)>(b))?(a):(b))
//#define MAX_ALLPASS_SIZE (2048*2)

typedef struct VerbDelayStruct
{
	  volatile __fp16 *ramBuffer;
	  uint16_t length;

	  uint16_t readPtr;
	  uint16_t writePtr;
	  float roomSz;

}VerbDelay;

void VerbDelay_init(VerbDelay* d);

uint16_t VerbDelay_getMaxSize(VerbDelay* d);

 uint16_t VerbDelay_getLength(VerbDelay* d);

 void VerbDelay_setLength(VerbDelay* d, uint16_t l);

 void VerbDelay_setRoomSize(VerbDelay* d, float sz);

 float VerbDelay_read(VerbDelay* d);

 void VerbDelay_write(VerbDelay* d, float in) ;
 float VerbDelay_ReadTap(VerbDelay* d, float percent);
 float VerbDelay_ReadTapSample(VerbDelay* d, uint16_t idx);
 float VerbDelay_process(VerbDelay* d, const float input);

 void VerbDelay_setBuffer(VerbDelay* d, volatile __fp16 *buffer);
 void VerbDelay_calcLength(VerbDelay* d);

#if 0

template<typename int16_t, uint16_t size>
class VerbDelay
{
public:
  VerbDelay()
  {
    readPtr=0;
    length = 256;
    writePtr=length;
    
    /*
    for(int i=0;i<size;i++) {
    	ramBuffer[i]=0;
    }
    */
    roomSz = 1;
  };
  ~VerbDelay(){};
  
  inline uint16_t getMaxSize()
  {
    return size;
  }
  
  inline uint16_t getLength()
  {
    return length;
  }
  
  void setLength(uint16_t l)
  {
    length = l;
    calcLength();
    /*
    uint16_t pos = writePtr-length;
    readPtr  = pos & (size - 1);
    */
  }
  
  inline void setRoomSize(float sz)
  {
    roomSz = sz;
    calcLength();
  }
  
  inline T read() 
  {
      uint16_t r = readPtr;
      T result = ramBuffer[r];
      readPtr = (r + 1) & (size - 1);
      return result;
  }
  

  inline void write(T in) {
      uint16_t w = writePtr;
      ramBuffer[w] = in;
      writePtr = (w + 1) & (size - 1);
  }
    
  inline T ReadTap(float percent)
  {
    uint16_t offset = percent/100.f * length;
    uint16_t r1 = readPtr;
    r1 = (r1 - offset) & (size - 1);
    T out = ramBuffer[r1];
    return out;
    
  }
  
  inline T process(const T input)
  {
	  calcLength();
    //process delay
      write(input);
      return read();
  }

  inline void setBuffer(volatile T *buffer)
  {
	  ramBuffer = buffer;
  }
  inline void calcLength()
  {
    float scaledSize = length*roomSz;
    float pos = writePtr-scaledSize;
    if(pos<0)pos+=size;
    uint16_t itg = pos;
    readPtr  = itg & (size - 1);
  }

private:
  volatile T *ramBuffer;
  uint16_t length;
  
  uint16_t readPtr;
  uint16_t writePtr;
  float roomSz;
};
#endif

#endif  // DELAY_H_INCLUDED
