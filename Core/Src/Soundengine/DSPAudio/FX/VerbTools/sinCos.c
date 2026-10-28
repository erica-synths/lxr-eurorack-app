/*
  ==============================================================================

    sinCos.cpp
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

#include "Soundengine/DSPAudio/FX/VerbTools/sinCos.h"
#include <math.h>

void SineCosine_init(SineCosine *sc)
{
	  sc->s[0] = 0.5f;
	  sc->s[1] = 0.f;

	  SineCosine_setFreq(sc, 0.5);
}
void SineCosine_calc(SineCosine *sc)
{
	  sc->s[0] = sc->s[0] - sc->a*sc->s[1];
	  sc->s[1] = sc->s[1] + sc->a*sc->s[0];
}
float SineCosine_getSine(SineCosine *sc)
{
	return sc->s[0];
}
float SineCosine_getCosine(SineCosine *sc)
{
	  return sc->s[1];
}

void SineCosine_setFreq(SineCosine *sc, float f)
{
	  sc->a = 2.f*(float)sinf(M_PI*f/REAL_FS);
}

