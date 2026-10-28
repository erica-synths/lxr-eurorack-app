/*
 * CV_Processor.h
 *
 *  Created on: Apr 20, 2022
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_SOUNDENGINE_MODMATRIX_PROCESSOR_H_
#define INC_SOUNDENGINE_MODMATRIX_PROCESSOR_H_

#define NUM_MOD_SLOTS 18

void CV_Init();
void CV_process();
float CV_getBpm();

#endif /* INC_SOUNDENGINE_MODMATRIX_PROCESSOR_H_ */
