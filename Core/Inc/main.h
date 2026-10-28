/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"
#include "delay.h"

extern float debugWatch;

extern I2S_HandleTypeDef hi2s2;
extern I2S_HandleTypeDef hi2s3;

extern TIM_HandleTypeDef htim2;

/*
#define MAPLE_RELOAD_VAL     72000
#define CYCLES_PER_MICROSECOND  72
#define FUDGE                   42
*/
#define USE_SD_CARD 1

#define EURORACK_MODULE 1
#define SEQ_DEFAULT_NOTE 60

void Error_Handler(void);

//---- pin assignments ----
#define Button_6_Pin GPIO_PIN_2
#define Button_6_GPIO_Port GPIOE
#define LED_3_Pin GPIO_PIN_3
#define LED_3_GPIO_Port GPIOE
#define LED_1_Pin GPIO_PIN_4
#define LED_1_GPIO_Port GPIOE
#define Button_11_Pin GPIO_PIN_5
#define Button_11_GPIO_Port GPIOE
#define Button_10_Pin GPIO_PIN_6
#define Button_10_GPIO_Port GPIOE
#define Button_1_Pin GPIO_PIN_13
#define Button_1_GPIO_Port GPIOC
#define LED_9_Pin GPIO_PIN_14
#define LED_9_GPIO_Port GPIOC
#define LED_5_Pin GPIO_PIN_15
#define LED_5_GPIO_Port GPIOC
#define ADC_Pot_4B_Pin GPIO_PIN_0
#define ADC_Pot_4B_GPIO_Port GPIOC
#define ADC_Pot_4A_Pin GPIO_PIN_1
#define ADC_Pot_4A_GPIO_Port GPIOC
#define ADC_Pot_3B_Pin GPIO_PIN_2
#define ADC_Pot_3B_GPIO_Port GPIOC
#define ADC_Pot_3A_Pin GPIO_PIN_3
#define ADC_Pot_3A_GPIO_Port GPIOC
#define ADC_CV_5_Pin GPIO_PIN_0
#define ADC_CV_5_GPIO_Port GPIOA
#define ADC_CV_4_Pin GPIO_PIN_1
#define ADC_CV_4_GPIO_Port GPIOA
#define ADC_CV_3_Pin GPIO_PIN_2
#define ADC_CV_3_GPIO_Port GPIOA
#define ADC_CV_2_Pin GPIO_PIN_3
#define ADC_CV_2_GPIO_Port GPIOA
#define ADC_CV_1_Pin GPIO_PIN_4
#define ADC_CV_1_GPIO_Port GPIOA
#define LED_6_Pin GPIO_PIN_5
#define LED_6_GPIO_Port GPIOA
#define LED_7_Pin GPIO_PIN_6
#define LED_7_GPIO_Port GPIOA
#define LED_8_Pin GPIO_PIN_7
#define LED_8_GPIO_Port GPIOA
#define ADC_POT_2B_Pin GPIO_PIN_4
#define ADC_POT_2B_GPIO_Port GPIOC
#define ADC_POT_2A_Pin GPIO_PIN_5
#define ADC_POT_2A_GPIO_Port GPIOC
#define ADC_POT_1B_Pin GPIO_PIN_0
#define ADC_POT_1B_GPIO_Port GPIOB
#define ADC_POT_1A_Pin GPIO_PIN_1
#define ADC_POT_1A_GPIO_Port GPIOB
#define Button_4_Pin GPIO_PIN_2
#define Button_4_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_7
#define LCD_D4_GPIO_Port GPIOE
#define LCD_D5_Pin GPIO_PIN_8
#define LCD_D5_GPIO_Port GPIOE
#define LCD_D6_Pin GPIO_PIN_9
#define LCD_D6_GPIO_Port GPIOE
#define LCD_D7_Pin GPIO_PIN_10
#define LCD_D7_GPIO_Port GPIOE
#define LCD_E_Pin GPIO_PIN_11
#define LCD_E_GPIO_Port GPIOE
#define LCD_RS_Pin GPIO_PIN_12
#define LCD_RS_GPIO_Port GPIOE
#define Enc_A_Pin GPIO_PIN_13
#define Enc_A_GPIO_Port GPIOE
#define Enc_B_Pin GPIO_PIN_14
#define Enc_B_GPIO_Port GPIOE
#define Enc_Switch_Pin GPIO_PIN_15
#define Enc_Switch_GPIO_Port GPIOE
#define LED_2_Pin GPIO_PIN_10
#define LED_2_GPIO_Port GPIOB
#define LED_4_Pin GPIO_PIN_11
#define LED_4_GPIO_Port GPIOB
#define Button_2_Pin GPIO_PIN_14
#define Button_2_GPIO_Port GPIOB
#define Accent_1_Pin GPIO_PIN_8
#define Accent_1_GPIO_Port GPIOD
#define EXTI9_Trigger_7_Pin GPIO_PIN_9
#define EXTI9_Trigger_7_GPIO_Port GPIOD
#define EXTI9_Trigger_7_EXTI_IRQn EXTI9_5_IRQn
#define EXT10I_Trigger_3_Pin GPIO_PIN_10
#define EXT10I_Trigger_3_GPIO_Port GPIOD
#define EXT10I_Trigger_3_EXTI_IRQn EXTI15_10_IRQn
#define EXT11I_Trigger_2_Pin GPIO_PIN_11
#define EXT11I_Trigger_2_GPIO_Port GPIOD
#define EXT11I_Trigger_2_EXTI_IRQn EXTI15_10_IRQn
#define EXT12I_Trigger_6_Pin GPIO_PIN_12
#define EXT12I_Trigger_6_GPIO_Port GPIOD
#define EXT12I_Trigger_6_EXTI_IRQn EXTI15_10_IRQn
#define EXT13I_Trigger_5_Pin GPIO_PIN_13
#define EXT13I_Trigger_5_GPIO_Port GPIOD
#define EXT13I_Trigger_5_EXTI_IRQn EXTI15_10_IRQn
#define EXTI14_Trigger_4_Pin GPIO_PIN_14
#define EXTI14_Trigger_4_GPIO_Port GPIOD
#define EXTI14_Trigger_4_EXTI_IRQn EXTI15_10_IRQn
#define EXTI15_Trigger_1_Pin GPIO_PIN_15
#define EXTI15_Trigger_1_GPIO_Port GPIOD
#define EXTI15_Trigger_1_EXTI_IRQn EXTI15_10_IRQn
#define LED_13_Pin GPIO_PIN_8
#define LED_13_GPIO_Port GPIOA
#define LED_14_Pin GPIO_PIN_9
#define LED_14_GPIO_Port GPIOA
#define Button_13_Pin GPIO_PIN_11
#define Button_13_GPIO_Port GPIOA
#define Button_14_Pin GPIO_PIN_12
#define Button_14_GPIO_Port GPIOA
#define LED_12_Pin GPIO_PIN_11
#define LED_12_GPIO_Port GPIOC
#define SDIO_CS_Pin GPIO_PIN_0
#define SDIO_CS_GPIO_Port GPIOD
#define SD_CD_Pin GPIO_PIN_1
#define SD_CD_GPIO_Port GPIOD
#define Accent_5_Pin GPIO_PIN_3
#define Accent_5_GPIO_Port GPIOD
#define Accent_3_Pin GPIO_PIN_4
#define Accent_3_GPIO_Port GPIOD
#define Accent_4_Pin GPIO_PIN_5
#define Accent_4_GPIO_Port GPIOD
#define Accent_2_Pin GPIO_PIN_6
#define Accent_2_GPIO_Port GPIOD
#define Accent_6_Pin GPIO_PIN_7
#define Accent_6_GPIO_Port GPIOD
#define Button_12_Pin GPIO_PIN_3
#define Button_12_GPIO_Port GPIOB
#define LED_11_Pin GPIO_PIN_4
#define LED_11_GPIO_Port GPIOB
#define Button_5_Pin GPIO_PIN_6
#define Button_5_GPIO_Port GPIOB
#define LED_10_Pin GPIO_PIN_7
#define LED_10_GPIO_Port GPIOB
#define Button_3_Pin GPIO_PIN_8
#define Button_3_GPIO_Port GPIOB
#define Button_7_Pin GPIO_PIN_9
#define Button_7_GPIO_Port GPIOB
#define Button_9_Pin GPIO_PIN_0
#define Button_9_GPIO_Port GPIOE
#define Button_8_Pin GPIO_PIN_1
#define Button_8_GPIO_Port GPIOE

// Helper to declare variable in CCM memory, pre-loaded from flash at startup
#define INDTCM __attribute__ ((section(".dtcm")))

// Helper to declare variable in CCM memory that is zeroed at startup
#define INDTCMZ __attribute__ ((section(".dtcmz")))

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
