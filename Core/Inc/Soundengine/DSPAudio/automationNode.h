/*
 * automationNode.h
 *
 *  Created on: 12.02.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef AUTOMATIONNODE_H_
#define AUTOMATIONNODE_H_
//-------------------------------------------------------------
#include "stm32f7xx_hal.h"
#include "Soundengine/MIDI/MidiParser.h"
//-------------------------------------------------------------
typedef struct AutomationStruct
{
	uint16_t	destination;	/**< dest param nr */
} AutomationNode;
//-------------------------------------------------------------
void autoNode_init(AutomationNode* node);
void autoNode_setDestination(AutomationNode* node, uint16_t dest);
void autoNode_updateValue(AutomationNode* node, uint8_t val);

#endif /* AUTOMATIONNODE_H_ */
