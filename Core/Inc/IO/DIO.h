/*
 * DIO.h
 *
 *  Created on: Mar 22, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_IO_DIO_H_
#define INC_IO_DIO_H_

#include "main.h"

//number of connected outputs (multiple of 8!!!)
#define NUM_OUTS 16


#define NUM_INPUTS 14


//extern INDTCMZ uint8_t din_inputData[NUM_INPUTS/8];
extern uint8_t dout_outputData[NUM_OUTS/8];

void dio_init();
void dio_process();
void dio_mute(uint8_t voice, uint8_t onOff);
#endif /* INC_IO_DIO_H_ */
