/*
 * encoder.h
 *
 * Rotary encoder (one full quadrature cycle per detent) and its push button.
 *
 * encoder_callback() samples the inputs and must be called every 1 ms (SysTick).
 * The main loop reads the results with encode_read4() and encode_readButton().
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */
#ifndef ENCODER_H_
#define ENCODER_H_

#include "main.h"

/** Raw input levels (1 = pin high). Pins are defined in main.h. */
#define PHASE_A         ((Enc_A_GPIO_Port->IDR & Enc_A_Pin) != 0)
#define PHASE_B         ((Enc_B_GPIO_Port->IDR & Enc_B_Pin) != 0)
#define ENCODER_BUTTON  ((Enc_Switch_GPIO_Port->IDR & Enc_Switch_Pin) != 0)

/** Takes the current encoder position as the resting (detent) position.
 *  Optional: the first encoder_callback() does the same. */
void encode_init(void);

/** Samples encoder and button. Call every 1 ms. */
void encoder_callback(void);

/** Returns +1 or -1 for one detent turned since the last call, 0 if none.
 *  Further detents are returned by the following calls (up to 4 are queued). */
int8_t encode_read4(void);

/** Returns 1 while the (debounced) encoder button is pressed, else 0. */
uint8_t encode_readButton(void);

#endif /* ENCODER_H_ */
