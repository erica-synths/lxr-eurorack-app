/*
 * valueShaper.h
 *
 *  Created on: 25.01.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef VALUESHAPER_H_
#define VALUESHAPER_H_
//-------------------------------------------------------------
#include "stm32f7xx_hal.h"
#include <math.h>
//-------------------------------------------------------------
//shape is a value between -1 and 1
static inline float valueShaperI2F(uint8_t data2, float shape)
{

	const float k = 2*shape/(1.0001f-shape);
	const float val = (data2)/127.f;
	return  ((1+k)*val/(1+k*fabsf(val)));
}
//-------------------------------------------------------------
static inline float valueShaperF2F(float val, float shape)
{
	const float k = 2*shape/(1.0001f-shape);
	return  ((1+k)*val/(1+k*fabsf(val)));
}
//-------------------------------------------------------------
#endif /* VALUESHAPER_H_ */
