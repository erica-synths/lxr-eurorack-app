/*
 * snapEg.h
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


#ifndef SNAPEG_H_
#define SNAPEG_H_

#include "stm32f7xx_hal.h"

typedef struct Snap_EG_Struct
{
	float value;
} SnapEg;


void SnapEg_init(SnapEg* eg);
void SnapEg_trigger(SnapEg* eg);
float SnapEg_calc(SnapEg* eg, float time);



#endif /* SNAPEG_H_ */
