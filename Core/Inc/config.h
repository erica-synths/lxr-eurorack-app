/*
 * config.h
 *
 * Created: 23.10.2012 10:15:14
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define USE_SD_CARD 1		//set to "1" if the SD reader is connected to the AVR, "0" if the cortex uses it
#define USE_DRUM_MAP_GENERATOR 0


#define UART_DEBUG_ECHO_MODE 0
#define DEBUG_CRASH_MODE 0

#endif /* CONFIG_H_ */
