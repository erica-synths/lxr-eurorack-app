/*
 * AudioCodecManager.h
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef AUDIOCODECMANAGER_H_
#define AUDIOCODECMANAGER_H_

#include "stm32f7xx_hal.h"
#include "config.h"
#include "globals.h"
#include "cs4344_cs5343.h"

#define DMA_MODE_ACTIVE 1
//-----------------------------------
int CodecInit();

#endif /* AUDIOCODECMANAGER_H_ */
