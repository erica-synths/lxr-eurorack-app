/*
 * SlopeEg2.h
 *
 *  Created on: 16.06.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef SLOPEEG2_H_
#define SLOPEEG2_H_



#include "stm32f7xx_hal.h"


#define EG_STOPPED 	0
#define EG_A		1
#define EG_D		2
#define EG_REPEAT	3

typedef struct SLOPE_EG2_Struct
{
	double 	attack;
	double 	decay;
	double 	slope;
	double 	invSlope;
	uint8_t repeat; 				/**<number of repetitions of the attack phase*/
	uint8_t repeatCnt;				/**<a counter for the already played repeats*/
	double 	value;
	uint8_t state;

} SlopeEg2;


void slopeEg2_init(SlopeEg2* eg);
void slopeEg2_trigger(SlopeEg2* eg);
double slopeEg2_calc(SlopeEg2* eg);
void slopeEg2_setAttack(SlopeEg2* eg, uint8_t data2, uint8_t isSync);
void slopeEg2_setDecay(SlopeEg2* eg, uint8_t data2, uint8_t isSync);
void slopeEg2_setSlope(SlopeEg2* eg, uint8_t data2);
double slopeEg2_calcDecay(uint8_t data2);


#endif /* SLOPEEG2_H_ */
