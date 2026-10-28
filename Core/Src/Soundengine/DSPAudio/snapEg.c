/*
 * snapEg.c
 *
 *  Created on: 16.06.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "Soundengine/DSPAudio/snapEg.h"

#define SNAP_MAX_VALUE 24.f
#define SNAP_REDUCTION 0.2f
//-----------------------------------------------------------------------
void SnapEg_init(SnapEg* eg)
{
	eg->value = 0;
}
//-----------------------------------------------------------------------
void SnapEg_trigger(SnapEg* eg)
{
	eg->value = 1;
}
//-----------------------------------------------------------------------
float SnapEg_calc(SnapEg* eg, float time)
{
	float ret = 0;
	if(eg->value>0)
	{
		ret =  eg->value* eg->value * SNAP_MAX_VALUE;
		eg->value -= SNAP_REDUCTION*time;

	}

	return ret;
}
//-----------------------------------------------------------------------


