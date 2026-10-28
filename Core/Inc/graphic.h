/*
 * graphic.h
 *
 *  Created on: 08.06.2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_GRAPHIC_H_
#define INC_GRAPHIC_H_

#include "main.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
//framebuffer...
//5x4 = 20 bit top row
//5x4 = 20 bit bottm row
typedef union
{
	struct
	{
		unsigned 	char4:5;
		unsigned 	char3:5;
		unsigned 	char2:5;
		unsigned 	char1:5;
		unsigned 	unused:12;

	} chars;
	uint32_t itg;
}Framebuffer;

#define GFX_SIZE_X 20
#define GFX_SIZE_Y 16


void graphic_tick();
void graphic_clearBuffer(Framebuffer* fb, uint16_t size);
uint8_t graphic_calcPlasma(uint8_t x, uint8_t y);
void graphic_drawLineArt(Framebuffer fb[][8]);

void graphic_setPoint(uint8_t x, uint8_t y, Framebuffer fb[][8]);
void graphic_line(int x0, int y0, int x1, int y1, Framebuffer fb[][8]) ;


#endif /* INC_GRAPHIC_H_ */
