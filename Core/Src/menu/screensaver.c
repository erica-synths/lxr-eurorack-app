/*
 * screensaver.c
 *
 * Created: 25.01.2013 17:08:02
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 
//to expand the lifetime of OLED displays, we need a timeout var to go to
//screensaver mode if the controls are not touched for a while
#include "lcd.h"
#include "menu/screensaver.h"

#include "menu/menu.h"
#include "delay.h"
#include "graphic.h"
#include "Soundengine/Soundengine.h"

#define SCREENSAVER_ACTIVE 1

//stuff for activation logic
volatile uint32_t screensaver_timer = 0;
#define SCREENSAVER_TIMEOUT 2//[minutes]
#define SCREENSAVER_SYSTICK (uint32_t)(((SCREENSAVER_TIMEOUT*60) *1000))/// (1.f / ((F_CPU/1024.f)/256.f))))
volatile uint8_t screensaver_Active=0;
volatile uint32_t screensaver_Timeout=SCREENSAVER_SYSTICK;


//stuff for fancy drawing
static uint32_t screensaver_processTime = 0;
#define SCREENSAVER_PROCESS_SPEED 500//((0.5) / (1.f / ((F_CPU/1024.f)/256.f)))
enum
{
	CELL_NEW_POS,
	CELL_OFF,
	CELL_OFF_WAIT,
	CELL_1_SET_CURSOR,
	CELL_1_PRINT,
	CELL_1_WAIT,
	CELL_2_SET_CURSOR,
	CELL_2_PRINT,
	CELL_2_WAIT,
	CELL_3_SET_CURSOR,
	CELL_3_PRINT,
	CELL_3_WAIT,


};

uint8_t screensaver_activeCell = 0;
uint8_t screensaver_cellState = CELL_OFF;

uint16_t screensaver_rng =  0;	//*<temporary var for rng generation*/
//------------------------------------------------------------------
uint8_t screensaver_pseudoRng()
{
	screensaver_rng++;
	screensaver_rng *= 3;
	return (screensaver_rng >> 2) & 0xFF;
}
//------------------------------------------------------------------
void screensaver_touch()
{
	screensaver_timer = 0;
	screensaver_Timeout = SCREENSAVER_SYSTICK;

	if(screensaver_Active)
	{
		//while(sendDisplayBuffer() != 0); //wrong! we wan't to clean the screensaver pipeline, noit the normal menu one!
#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Start_IT(&htim2);	//calc sample interrupt - prio 1
#endif
		while(screensaver_process() != 0) soundengine_process();

#if USE_INTERRUPT_ONLY_DURING_SAVE_PROJ
	HAL_TIM_Base_Stop_IT(&htim2);	//calc sample interrupt - prio 1
#endif
		//lcd_turnOn(1,0);
		screensaver_Active = 0;
		menu_repaintAll();
	}	
}
//------------------------------------------------------------------
//draw some fancy stuff on the LCD


Framebuffer framebuffer[2][8];
uint8_t x=0;
uint8_t y = 0;

uint8_t screensaver_process()
{

#if PLASMA_SCREENSAVER
	switch(screensaver_cellState)
	{
	case 0:
		if(lcd_clear_nonblock())
		{
			screensaver_cellState++;
		}
		break;

	case 1:
		//print custom characters
		lcd_home();
		lcd_clear();
		lcd_setcursor(0,1);
		lcd_data(0);
		lcd_data(1);
		lcd_data(2);
		lcd_data(3);
		lcd_data(0);
		lcd_data(1);
		lcd_data(2);
		lcd_data(3);
		lcd_data(0);
		lcd_data(1);
		lcd_data(2);
		lcd_data(3);
		lcd_data(0);
		lcd_data(1);
		lcd_data(2);
		lcd_data(3);

		lcd_setcursor(0,2);
		lcd_data(4);
		lcd_data(5);
		lcd_data(6);
		lcd_data(7);
		lcd_data(4);
		lcd_data(5);
		lcd_data(6);
		lcd_data(7);
		lcd_data(4);
		lcd_data(5);
		lcd_data(6);
		lcd_data(7);
		lcd_data(4);
		lcd_data(5);
		lcd_data(6);
		lcd_data(7);
		screensaver_cellState++;
		break;

	case 2:
	{
		graphic_tick();
		//if(HAL_GetTick() >= screensaver_processTime )
		{
		//update framebuffer

		//send framebuffer
	    // Startposition des Zeichens einstellen


			lcd_command( LCD_SET_CGADR | (uint8_t)(0<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[0][i].chars.char1 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(1<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[0][i].chars.char2 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(2<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[0][i].chars.char3 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(3<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[0][i].chars.char4 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(4<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[1][i].chars.char1 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(5<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[1][i].chars.char2 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(6<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[1][i].chars.char3 );
			}

			lcd_command( LCD_SET_CGADR | (uint8_t)(7<<3) );
			// Bitmuster uebertragen
			for ( uint8_t i=0; i<8; i++ )
			{
				lcd_data( framebuffer[1][i].chars.char4 );
			}

			screensaver_cellState++;
		}
		}
	break;

	case 3:
		//generate new gfx
#if 0
	{


		uint8_t row = y>7?1:0;

		framebuffer[row][y%8].itg |= (1<<(19-x));

		x++;
		if(x>=20)
		{
			x=0;
			y++;
			if(y>=16) y=0;
		}
	}
#elif 1
		for(uint8_t y=0;y<16;y++)
		{
			uint8_t row = y>7?1:0;

			for(uint8_t x=0;x<20;x++)
			{

				uint8_t value = graphic_calcPlasma(x,y);
				if(value)
				{
					//set pixel
					framebuffer[row][y%8].itg |= (1<<(19-x));
				}
				else
				{
					//remove pixel
					framebuffer[row][y%8].itg &= ~(1<<(19-x));
				}
			}
		}
#else
		graphic_drawLineArt(framebuffer);
#endif
		screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);


		screensaver_cellState = 2;
		break;

	}





#else
	char text[2];
	text[1] = 0;

	while(sendDisplayBuffer() != 0);

	switch(screensaver_cellState)
	{
		default:
			break;

		case CELL_NEW_POS:
			if(HAL_GetTick() >= screensaver_processTime )
			{
				screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);


				screensaver_activeCell = screensaver_pseudoRng()&0x1f; //value between 0 and 31

				screensaver_cellState = CELL_OFF;
				return 0;
			}
			break;

			case CELL_OFF:
				//lcd_clear();
				if(lcd_clear_nonblock())
				{
					screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);
					screensaver_cellState = CELL_OFF_WAIT;
					return 0;
				}
				break;

			case CELL_OFF_WAIT:
				if(HAL_GetTick() >= screensaver_processTime )
				{
					screensaver_cellState = CELL_1_SET_CURSOR;
					return 0;
				}
				break;

			case CELL_1_SET_CURSOR:
				if(screensaver_activeCell>15)
				{
					if(lcd_setcursor_nonBlock((uint8_t)(screensaver_activeCell-16),2))
					{
						screensaver_cellState = CELL_1_PRINT;
						return 0;
					}
				}
				else
				{
					if(lcd_setcursor_nonBlock(screensaver_activeCell,1))
					{
						screensaver_cellState = CELL_1_PRINT;
						return 0;
					}
				}
			break;

			case CELL_1_PRINT:
				text[0] = '.'; //.
				//lcd_string(text);
				if(lcd_data_nonBlock(text[0]))
				{
					screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);
					screensaver_cellState = CELL_1_WAIT;
					return 0;
				}
				break;

			case CELL_1_WAIT:
				if(HAL_GetTick() >= screensaver_processTime )
				{
					screensaver_cellState = CELL_2_SET_CURSOR;
					return 0;
				}
				break;

			case CELL_2_SET_CURSOR:
				if(screensaver_activeCell>15)
				{
					if(lcd_setcursor_nonBlock((uint8_t)(screensaver_activeCell-16),2))
					{
						screensaver_cellState = CELL_2_PRINT;
						return 0;
					}
				}
				else
				{
					if(lcd_setcursor_nonBlock(screensaver_activeCell,1))
					{

						screensaver_cellState = CELL_2_PRINT;
						return 0;
					}
				}
			break;

			case CELL_2_PRINT:
				text[0] = 'o'; //o
				//lcd_string(text);
				if(lcd_data_nonBlock(text[0]))
				{
					screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);
					screensaver_cellState = CELL_2_WAIT;
					return 0;
				}

			break;

			case CELL_2_WAIT:
				if(HAL_GetTick() >= screensaver_processTime )
				{
					screensaver_cellState = CELL_3_SET_CURSOR;
					return 0;
				}
				break;

			case CELL_3_SET_CURSOR:
				if(screensaver_activeCell>15)
				{
					if(lcd_setcursor_nonBlock((uint8_t)(screensaver_activeCell-16),2))
					{
						screensaver_cellState = CELL_3_PRINT;
						return 0;
					}
				}
				else
				{
					if(lcd_setcursor_nonBlock(screensaver_activeCell,1))
					{
						screensaver_cellState = CELL_3_PRINT;
						return 0;
					}
				}
			break;

			case CELL_3_PRINT:
			text[0] = '*'; //pop
			//lcd_string(text);
			if(lcd_data_nonBlock(text[0]))
			{
				screensaver_processTime = ( HAL_GetTick() + SCREENSAVER_PROCESS_SPEED);
				screensaver_cellState = CELL_NEW_POS;
				return 0;
			}

			break;
	}


	return 1;
#endif
}
//------------------------------------------------------------------
void screensaver_check()
{
	if(activeParameters[PAR_SCREENSAVER_ON_OFF])
	{
		if( (!screensaver_Active) )
		{
			if(screensaver_timer >= screensaver_Timeout)
			{
				//lcd_turnOn(0,0);
				screensaver_Active = 1;	
				screensaver_cellState = 0;
			}
		}
		else
		{
			screensaver_process();
		}	
	}
};
//------------------------------------------------------------------
