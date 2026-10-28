/*
 * graphic.c
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


#include "graphic.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>




float time=0;
#define SCALE 0.5f

uint8_t gfx_point_x = 5;
uint8_t gfx_point_y = 0;
uint8_t gfx_point_dir_x = 0;
uint8_t gfx_point_dir_y = 0;

void graphic_tick()
{
	time+= 0.1f;


}

void graphic_drawLineArt(Framebuffer fb[][8])
{

	//calc bouncing point
	if(gfx_point_dir_x)
	{
		if(gfx_point_x < GFX_SIZE_X-1)
			gfx_point_x++;
		else
			gfx_point_dir_x=0;
	}
	else
	{
		if(gfx_point_x > 0)
			gfx_point_x--;
		else
			gfx_point_dir_x = 1;

	}



	if(gfx_point_dir_y)
	{
		if(gfx_point_y < GFX_SIZE_Y-1)
			gfx_point_y++;
		else
			gfx_point_dir_y=0;
	}
	else
	{
		if(gfx_point_y > 0)
			gfx_point_y--;
		else
		{
			gfx_point_dir_y = 1;
		}
	}

	/*
	gfx_point_x++;
	if(gfx_point_x >= GFX_SIZE_X)
	{
		gfx_point_x = 0;
		gfx_point_y++;
		if(gfx_point_y >= GFX_SIZE_Y)
		{
			gfx_point_y = 0;
		}
	}
	*/


	//clear fb
	graphic_clearBuffer(fb[0], sizeof(Framebuffer)*8);
	graphic_clearBuffer(fb[1], sizeof(Framebuffer)*8);

	//draw new frame
	graphic_line(0,0, gfx_point_x,gfx_point_y,fb);
	graphic_line(0,GFX_SIZE_Y-1, gfx_point_x,gfx_point_y,fb);

	//graphic_setPoint(gfx_point_x, gfx_point_y, fb);

	graphic_line(gfx_point_x ,gfx_point_y , GFX_SIZE_X-1, 0,fb);
	graphic_line(gfx_point_x ,gfx_point_y , GFX_SIZE_X-1, GFX_SIZE_Y-1,fb);

}

void graphic_clearBuffer(Framebuffer* fb, uint16_t size)
{
	memset(fb, 0, size);
}

uint8_t graphic_calcPlasma(uint8_t x, uint8_t y)
{
	float value = sinf(x*SCALE+time);

	float value2 = sinf(  SCALE*(x+sinf(time/2) + y*cosf(time/3)) +time  );

	float cx = x+0.5*sinf(time/5);
	float cy = y+0.5*cosf(time/3);
	float val3 = sinf(sqrt(SCALE*(cx*cx+cy*cy+1))+time);
	return (value2+value+val3)/3.f >0.0f;
}

void swap(int *a, int *b)
{
	*a=*a^*b;
	*b=*a^*b;
	*a=*b^*a;
}


void graphic_setPoint(uint8_t x, uint8_t y, Framebuffer fb[][8])
{
	uint8_t row = y>7?1:0;
	fb[row][y%8].itg |= (1<<(19-x));

}

void graphic_line(int x0, int y0, int x1, int y1, Framebuffer fb[][8]) {
    uint8_t steep = 0;
    if (abs(x0-x1)<abs(y0-y1)) {
        swap(&x0, &y0);
        swap(&x1, &y1);
        steep = 1;
    }
    if (x0>x1) {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    float derror = fabs(dy/(float)(dx));
    float error = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++) {
        if (steep) {
        	graphic_setPoint(y,x,fb);
            //image.set(y, x, color);
        } else {
            //image.set(x, y, color);
        	graphic_setPoint(x,y,fb);
        }
        error += derror;
        if (error>.5) {
            y += (y1>y0?1:-1);
            error -= 1.;
        }
    }
}
