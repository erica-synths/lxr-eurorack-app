/*
 * screensaver.h
 *
 * Created: 25.01.2013 17:07:52
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef SCREENSAVER_H_
#define SCREENSAVER_H_
uint8_t screensaver_process();


void screensaver_touch();
void screensaver_check();


extern volatile uint8_t screensaver_Active;
extern volatile uint32_t screensaver_timer;

#endif /* SCREENSAVER_H_ */
