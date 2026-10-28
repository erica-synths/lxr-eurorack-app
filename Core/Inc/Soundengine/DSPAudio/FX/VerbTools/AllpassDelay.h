/*
  ==============================================================================

    AllpassDelay.h
    Created: 21 Jul 2014 7:29:52pm
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

#ifndef ALLPASSDELAY_H_INCLUDED
#define ALLPASSDELAY_H_INCLUDED

#include "VerbDelay.h"
#include "VerbModDelay.h"



#define INITIAL_ALLPASS_COFF 0.6


typedef struct AllpassDelayStruct
{
	  float g;
	  float out,in;
	  VerbDelay delay;


} AllpassDelay;
void AllpassDelay_init(AllpassDelay *d);
float AllpassDelay_process(AllpassDelay *d, float input);
void AllpassDelay_setCoefficient(AllpassDelay *d, float coff);
void AllpassDelay_setDelay(AllpassDelay *d, float del);
void AllpassDelay_setBuffer(AllpassDelay *d, volatile __fp16 *buffer);
void AllpassDelay_setRoomSize(AllpassDelay *d, float sz);

typedef struct AllpassModDelayStruct
{
	  float g;
	  float out,in;
	  VerbModDelay delay;


} AllpassModDelay;

void AllpassModDelay_init(AllpassModDelay *d);
float AllpassModDelay_process(AllpassModDelay *d, float input);
void AllpassModDelay_setCoefficient(AllpassModDelay *d, float coff);
void AllpassModDelay_setDelay(AllpassModDelay *d, float del);
void AllpassModDelay_setBuffer(AllpassModDelay *d, volatile __fp16 *buffer);
void AllpassModDelay_setRoomSize(AllpassModDelay *d, float sz);
void AllpassModDelay_setModDepth(AllpassModDelay *d,float depth);
void AllpassModDelay_setModValue(AllpassModDelay *d,float v);

/*
template<typename T>
class ApDelay
{
public:
	ApDelay();
  virtual ~ApDelay();
  
  virtual T process(const T input)=0;
  void setCoefficient(float coff);
  virtual void setDelay(float d)=0;

protected:
  float g;
  T out,in;
};
//#########################################################
template<typename T>
class AllpassDelay : public ApDelay<T>
{
public:
  AllpassDelay() : ApDelay<T>() { };

  void setDelay(float d);

  T process(const T input);

  inline void setRoomSize(float sz)
  {
   delay.setRoomSize(sz);
  }

  inline void setBuffer(volatile T *buffer)
  {
	  delay.setBuffer(buffer);
  }


private:
  VerbDelay <T,MAX_ALLPASS_SIZE> delay;
};

//############################################################
*/
/*
template<typename T>
class AllpassModDelay : public ApDelay<T>
{
public:
	AllpassModDelay() : ApDelay<T>() { };

  void setDelay(float d);
  T process(const T input);

  inline void setRoomSize(float sz)
  {
   delay.setRoomSize(sz);
  }

  inline void setBuffer(volatile T *buffer)
  {
	  delay.setBuffer(buffer);
  }

  inline void setModDepth(float d)
  {
    delay.setModDepth(d);
  }

  inline void setModValue(float v)
  {
    delay.setModValue(v);
	delay.calcLength();

  }

  VerbModDelay <T,MAX_ALLPASS_SIZE> delay;
};
*/

#endif  // ALLPASSDELAY_H_INCLUDED
