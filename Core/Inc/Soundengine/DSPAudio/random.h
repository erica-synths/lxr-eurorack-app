/*
 * random.h
 *
 *  Created on: 07.04.2012
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef RANDOM_H_
#define RANDOM_H_

#include "stm32f7xx_hal.h"

extern RNG_HandleTypeDef hrng;

void initRng();

uint32_t GetRngValue();

#endif /* RANDOM_H_ */
