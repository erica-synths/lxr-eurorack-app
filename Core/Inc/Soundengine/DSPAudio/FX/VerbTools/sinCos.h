/*
  ==============================================================================

    sinCos.h
    Created: 22 Jul 2014 11:20:23am
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

#ifndef SINCOS_H_INCLUDED
#define SINCOS_H_INCLUDED

#include "Soundengine/globals.h"
#include "Soundengine/config.h"

typedef struct SinCosStruct
{

	  float a;
	  float s[2];

} SineCosine;

void SineCosine_init(SineCosine *sc);
void SineCosine_calc(SineCosine *sc);
float SineCosine_getSine(SineCosine *sc);
float SineCosine_getCosine(SineCosine *sc);

void SineCosine_setFreq(SineCosine *sc, float f);

#if 0
class SineCosine
{
public:
  SineCosine();
  ~SineCosine();
  
  void calc();
  float getSine();
  float getCosine();
  
  void setFreq(float f);
  
private:
  float a;
  float s[2];
  
};

#endif
#endif  // SINCOS_H_INCLUDED
