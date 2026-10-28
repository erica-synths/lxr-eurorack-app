/*
 * DIO.c
 *
 *  Created on: Mar 22, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "IO/DIO.h"
#include "buttonHandler.h"
//SPI_HandleTypeDef *spi1;
//SPI_HandleTypeDef hspi1;


uint8_t INDTCMZ din_inputData[NUM_INPUTS];
uint8_t INDTCMZ din_inputData_old[NUM_INPUTS];
uint8_t dout_outputData[NUM_OUTS/8];

//uint8_t button43_old;
#if 0
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}
#endif


void dio_init()
{
	//MX_SPI1_Init();


	//button43_old = 0;
}

void dio_mute(uint8_t voice, uint8_t onOff)
{
#pragma message "implement dio_mute for triggers"
}


void dio_process()
{
#if !EURORACK_MODULE
	//SCB_CleanDCache();
	//send and receive via shift registers

	//read data 165
	DIO_LD_GPIO_Port->BSRR = DIO_LD_Pin<<16; //reset LD pin
	DIO_LD_GPIO_Port->BSRR = DIO_LD_Pin; //set LD pin

	HAL_SPI_Receive(spi1, din_inputData, NUM_INPUTS/8, 100);

	//write data 595
	DIO_LD_GPIO_Port->BSRR = DIO_LD_Pin<<16; //reset LD pin
	//HAL_SPI_TransmitReceive(&hspi1, dout_outputData, din_inputData, NUM_INPUTS/8, 100);
	HAL_SPI_Transmit(spi1, dout_outputData, NUM_OUTS/8, 100);
	DIO_LD_GPIO_Port->BSRR = DIO_LD_Pin; //set LD pin




	//--- handle buttons ---

	for(int i=0; i<NUM_INPUTS/8; i++)
	{


		//did button group change?
		if(din_inputData[i] != din_inputData_old[i])
		{
			//check which buttons changed in group
			for(int bitPos=0;bitPos<8;bitPos++)
			{
				if( (din_inputData_old[i] & (1<<bitPos)) !=  (din_inputData[i] & (1<<bitPos)))
				{
					if((din_inputData[i] & (1<<bitPos)))
					{
						//button pressed
						buttonHandler_buttonPressed(i*8 + bitPos);
						din_inputData_old[i] |= (1<<bitPos);
					}
					else
					{
						//button released
						buttonHandler_buttonReleased(i*8 + bitPos);
						din_inputData_old[i] &= ~(1<<bitPos);
					}
				}
			}
		}
	}

	//Handle lone non shift register button
	const uint8_t currentValue = SW_43_GPIO_Port->IDR & SW_43_Pin;
	if(currentValue != button43_old)
	{
		button43_old = currentValue;
		if(currentValue == 0)
		{
			buttonHandler_buttonReleased(BUT_PAGE_DOWN);
		}
		else
		{
			buttonHandler_buttonPressed(BUT_PAGE_DOWN);
		}
	}
#else


	//read buttons
	din_inputData[0] = (Button_1_GPIO_Port->IDR & Button_1_Pin)!=0;
	din_inputData[1] = (Button_2_GPIO_Port->IDR & Button_2_Pin)!=0;
	din_inputData[2] = (Button_3_GPIO_Port->IDR & Button_3_Pin)!=0;
	din_inputData[3] = (Button_4_GPIO_Port->IDR & Button_4_Pin)!=0;
	din_inputData[4] = (Button_5_GPIO_Port->IDR & Button_5_Pin)!=0;
	din_inputData[5] = (Button_6_GPIO_Port->IDR & Button_6_Pin)!=0;
	din_inputData[6] = (Button_7_GPIO_Port->IDR & Button_7_Pin)!=0;
	din_inputData[7] = (Button_8_GPIO_Port->IDR & Button_8_Pin)!=0;
	din_inputData[8] = (Button_9_GPIO_Port->IDR & Button_9_Pin)!=0;
	din_inputData[9] = (Button_10_GPIO_Port->IDR & Button_10_Pin)!=0;
	din_inputData[10] = (Button_11_GPIO_Port->IDR & Button_11_Pin)!=0;
	din_inputData[11] = (Button_12_GPIO_Port->IDR & Button_12_Pin)!=0;

	din_inputData[12] = (Button_13_GPIO_Port->IDR & Button_13_Pin)!=0;
	din_inputData[13] = (Button_14_GPIO_Port->IDR & Button_14_Pin)!=0;

	//process buttons
	for(uint8_t i=0; i<NUM_INPUTS;i++)
	{
		if(din_inputData[i] != din_inputData_old[i])
		{
			din_inputData_old[i] = din_inputData[i];
			if(din_inputData[i] == 0)
			{
				buttonHandler_buttonReleased(i);
			}
			else
			{
				buttonHandler_buttonPressed(i);
			}
		}
	}

	//process dout LED data
	//	HAL_SPI_Transmit(spi1, dout_outputData, NUM_OUTS/8, 100);
	if(dout_outputData[0] & 0x01)	LED_1_GPIO_Port->BSRR = LED_1_Pin;
	else							LED_1_GPIO_Port->BSRR = LED_1_Pin << 16;

	if(dout_outputData[0] & 0x02)	LED_2_GPIO_Port->BSRR = LED_2_Pin;
	else							LED_2_GPIO_Port->BSRR = LED_2_Pin << 16;

	if(dout_outputData[0] & 0x04)	LED_3_GPIO_Port->BSRR = LED_3_Pin;
	else							LED_3_GPIO_Port->BSRR = LED_3_Pin << 16;

	if(dout_outputData[0] & 0x08)	LED_4_GPIO_Port->BSRR = LED_4_Pin;
	else							LED_4_GPIO_Port->BSRR = LED_4_Pin << 16;

	if(dout_outputData[0] & 0x10)	LED_5_GPIO_Port->BSRR = LED_5_Pin;
	else							LED_5_GPIO_Port->BSRR = LED_5_Pin << 16;

	if(dout_outputData[0] & 0x20)	LED_6_GPIO_Port->BSRR = LED_6_Pin;
	else							LED_6_GPIO_Port->BSRR = LED_6_Pin << 16;

	if(dout_outputData[0] & 0x40)	LED_7_GPIO_Port->BSRR = LED_7_Pin;
	else							LED_7_GPIO_Port->BSRR = LED_7_Pin << 16;

	if(dout_outputData[0] & 0x80)	LED_8_GPIO_Port->BSRR = LED_8_Pin;
	else							LED_8_GPIO_Port->BSRR = LED_8_Pin << 16;

	if(dout_outputData[1] & 0x01)	LED_9_GPIO_Port->BSRR = LED_9_Pin;
	else							LED_9_GPIO_Port->BSRR = LED_9_Pin << 16;

	if(dout_outputData[1] & 0x02)	LED_10_GPIO_Port->BSRR = LED_10_Pin;
	else							LED_10_GPIO_Port->BSRR = LED_10_Pin << 16;

	if(dout_outputData[1] & 0x04)	LED_11_GPIO_Port->BSRR = LED_11_Pin;
	else							LED_11_GPIO_Port->BSRR = LED_11_Pin << 16;

	if(dout_outputData[1] & 0x08)	LED_12_GPIO_Port->BSRR = LED_12_Pin;
	else							LED_12_GPIO_Port->BSRR = LED_12_Pin << 16;

	if(dout_outputData[1] & 0x10)	LED_13_GPIO_Port->BSRR = LED_13_Pin;
	else							LED_13_GPIO_Port->BSRR = LED_13_Pin << 16;

	if(dout_outputData[1] & 0x20)	LED_14_GPIO_Port->BSRR = LED_14_Pin;
	else							LED_14_GPIO_Port->BSRR = LED_14_Pin << 16;

#endif

}



