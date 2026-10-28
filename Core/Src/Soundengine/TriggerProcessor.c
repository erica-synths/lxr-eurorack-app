/*
 * TriggerOut.c
 *
 *  Created on: 06.05.2013
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "main.h"
#include "stm32f7xx_it.h"

#include <Soundengine/TriggerProcessor.h>
#include "string.h"
//#include "Soundengine/Sequencer/sequencer.h"
#include "limits.h"
#include "Soundengine/config.h"
#include "Soundengine/DSPAudio/medianFilter.h"

#include "main.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"

uint8_t trigger_dividerClockOut1 = PRE_4_PPQ_OUT;
//uint8_t trigger_dividerClockOut2 = PRE_4_PPQ;
uint8_t trigger_prescalerClockInput = PRE_4_PPQ_IN;

uint8_t trigger_prescaleCounterClockInput = 0;

uint8_t trigger_nextPulseOut1 = 0xff;
//uint8_t trigger_nextPulseOut2 = 0xff;

/*
uint32_t trigger_pulseTimes[NUM_PINS];
uint8_t trigger_pulseActive[NUM_PINS];
uint8_t trigger_gateMode = 0;
*/




volatile float trigger_phase = 0;
volatile float trigger_phaseInc = 0;
//static uint8_t trigger_phaseWrapCounter = 1;	// count how many times the phase increased by 1.0
												// with a preascaler of 8 (4ppq) 1 clock pulse
												// has to increment the sequencer by 8 steps
#define COUNTER_DURATION (1.0f/42000000l)

//--------------------------------------------------
#if 0
void EXTI9_5_IRQHandler()
{
	//reset in
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		if(seq_getExtSync())
		{
			const uint16_t pinState = (GPIOA->IDR & GPIO_Pin_8);
			if(!pinState)
			{
				//reset pin is high -> stop and reset sequencer
				seq_setRunning(0);
			} else {
				//reset pin is low -> start sequencer
				seq_setRunning(1);
			}
		}

		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	//clock in falling edge (inverted due to input transistor)
	else if(EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
		if(seq_getExtSync())
		{
			uint32_t counter = medianFilter(TIM_GetCounter(TIM2));

			//time for a 128th substep
			float t =  (counter) * COUNTER_DURATION / (trigger_prescalerClockInput);
			const float f = (1.f/t);
			trigger_phaseInc = (1/(REAL_FS/f)) ;

			//divide incoming clocks with prescaler
			//if(trigger_prescaleCounterClockInput % trigger_prescalerClockInput == 0)
			{
				if(seq_isRunning()!=0)
				{
					seq_triggerNextMasterStep(trigger_prescalerClockInput);
					//reset phase counter
					trigger_phase = 0;
					trigger_phaseWrapCounter = 1;

				}
			}
			//reset measurement timer
		}
		TIM_SetCounter(TIM2,0);
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}
#else

#if !HANDLE_SEQ_IN_CLOCK_INTERRUPT
uint8_t trigger_clockReceived = 0;
#endif

#define WINDOW_SIZE 8
#define WINDOW_MASK 0x07
#if !EURORACK_MODULE
static float sync_window[WINDOW_SIZE];
#endif
float trigger_timeLast = 0;
uint8_t syncTrigger_writePos = 0;
uint8_t trigger_clockCnt = 0;

//---------------------------------------------------------
uint16_t trigger_calcBpm(float timePerPulse)
{
	// 1 Minute has 60 * 1000 [ms]
	//we have 24 pulses per quarter
	//120 bpm = 120 quarters per minute

	float quarterDuration = 24 * (timePerPulse/4);
	float bpm = (60 * 1000) / quarterDuration;

	return bpm>0?bpm:1;
}
/*
void trigger_ResetPosition()
{
	trigger_clockCnt = 0;

}
*/

//Clock In Interrupt
void EXTI4_IRQHandler(void)
{
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);

#if !EURORACK_MODULE
		if(seq_getExtSync())
		{
#if !HANDLE_SEQ_IN_CLOCK_INTERRUPT


			//Fix for external sync LFO sync calculation

			uint8_t syncFactor = 1;
			if(trigger_prescalerClockInput == PRE_4_PPQ_IN)
			{
				// 4ppq * 6 = 24ppq
				syncFactor = 6;
				trigger_clockReceived++;
			}
			else if(trigger_prescalerClockInput == PRE_24_PPQ_IN)
			{
				syncFactor = 1;

				if(trigger_clockCnt == 0)
				{
					trigger_clockReceived++;
				}

				trigger_clockCnt++;
				if(trigger_clockCnt >= 6)
				{
					trigger_clockCnt = 0;

				}
			}


				float time = systick_ticks - trigger_timeLast;
				trigger_timeLast = systick_ticks;

				sync_window[syncTrigger_writePos++] = time/syncFactor;
				syncTrigger_writePos &= WINDOW_MASK;

				//calc average time in [ms] between midi clock pulses
				int i;
				float avg=0;
				for(i=0;i<WINDOW_SIZE;i++)
				{
					avg += sync_window[i];
				}
				avg /= WINDOW_SIZE;
				//based on 24ppq midi resolution
				seq_setBpm(trigger_calcBpm(avg));

#else
			/*
			uint32_t counter = medianFilter(__HAL_TIM_GetCounter(&htim2));

			//time for a 128th substep
			float t =  (counter) * COUNTER_DURATION / (trigger_prescalerClockInput);
			const float f = (1.f/t);
			trigger_phaseInc = (1/(REAL_FS/f)) ;

			//divide incoming clocks with prescaler
			//if(trigger_prescaleCounterClockInput % trigger_prescalerClockInput == 0)
			{
				if(seq_isRunning()!=0)
				{
					seq_triggerNextMasterStep(trigger_prescalerClockInput);
					//reset phase counter
					trigger_phase = 0;
					trigger_phaseWrapCounter = 1;

				}
			}
			//reset measurement timer
			 *
			 */

			if(seq_isRunning())
			{
				if(trigger_prescaleCounterClockInput % trigger_prescalerClockInput == 0)
				{
					seq_nextStep();
				}
				trigger_prescaleCounterClockInput++;
			}
#endif

		}
#endif
		//__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
	}
}

uint8_t Trigger_getAccent(uint8_t voice)
{
	//Input signal inverted -> transistor input... 1=Off, 0=On
	uint16_t pinState = 1;
	switch(voice)
	{
	case 0:
		pinState = (Accent_1_GPIO_Port->IDR & Accent_1_Pin);
		break;

	case 1:
		pinState = (Accent_2_GPIO_Port->IDR & Accent_2_Pin);
		break;

	case 2:
		pinState = (Accent_3_GPIO_Port->IDR & Accent_3_Pin);
		break;

	case 3:
		pinState = (Accent_4_GPIO_Port->IDR & Accent_4_Pin);
		break;

	case 4:
		pinState = (Accent_5_GPIO_Port->IDR & Accent_5_Pin);
		break;

	case 5:
	case 6:
		pinState = (Accent_6_GPIO_Port->IDR & Accent_6_Pin);
		break;

	default:
		break;
	}

	if(pinState==0)
	{
		return 127;
	}
	else
		return LOW_VOLUME;

}

static uint8_t voiceTriggered[6] = {0,0,0,0,0,0};

uint8_t Trigger_getVoiceTriggered(uint8_t voice) {
	if(voiceTriggered[voice]) {
		return 1;
	}

	return 0;
}

void Trigger_resetVoiceTriggered(uint8_t voice) {
	voiceTriggered[voice] = 0;
}

void EXTI15_10_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);

		voiceTriggered[2] = 1;
	}

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);

		voiceTriggered[1] = 1;
	}

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);

		voiceTriggered[5] = 1;
	}

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);

		voiceTriggered[4] = 1;
	}

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);

		voiceTriggered[3] = 1;
	}

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);

		voiceTriggered[0] = 1;
	}
	/*
	  // EXTI line interrupt detected
	  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET)
	  {
	    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
	    HAL_GPIO_EXTI_Callback(GPIO_Pin);
	  }

	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
	  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
	  */
}
//Reset In Interrupt

void EXTI9_5_IRQHandler(void)
{

	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_9) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_9);

		const uint8_t voice = 6;
		const uint8_t vel = Trigger_getAccent(voice);
		voiceControl_noteOn(voice , SEQ_DEFAULT_NOTE, vel);
	}
#if !EURORACK_MODULE
	/* EXTI line interrupt detected */
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_5) != RESET)
	{


		if(seq_getExtSync())
		{
			const uint16_t pinState = (Reset_In_EXTI5_GPIO_Port->IDR & Reset_In_EXTI5_Pin);
			if(!pinState)
			{
				//reset pin is high -> stop and reset sequencer
				seq_setRunning(0);
				trigger_ResetPosition();
				seq_resetPosition();
				trigger_prescaleCounterClockInput = 0;
			} else {
				//reset pin is low -> start sequencer
				seq_setRunning(1);
			}
		}

		__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_5);
	}
	#endif
}
#endif
//--------------------------------------------------
void trigger_setPin(uint8_t index, uint8_t isOn)
{
#if !EURORACK_MODULE

	switch(index)
	{
	case TRIGGER_1:
	case TRIGGER_2:
	case TRIGGER_3:
	case TRIGGER_4:
	case TRIGGER_5:
	case TRIGGER_6:
	case TRIGGER_7:
	case CLOCK_1:
		/*
		#define Clock_Out_Pin GPIO_PIN_1
		#define Clock_Out_GPIO_Port GPIOA
		*/
		if(isOn)
		{
			Clock_Out_GPIO_Port->ODR |= (Clock_Out_Pin) ;
		} else {
			Clock_Out_GPIO_Port->ODR &= ~(Clock_Out_Pin) ;
		}

		break;


	case CLOCK_2:
	case TRIGGER_RESET:
		/*
		if(!isOn)
		{
			GPIOA->ODR |= (PIN_CLOCK_2<<(index-CLOCK_2)) ;
		} else {
			GPIOA->ODR &= ~(PIN_CLOCK_2<<(index-CLOCK_2)) ;
		}*/
		break;

	case TRIGGER_ALL:
		/*
		if(!isOn)
		{
			GPIOD->ODR |= (PIN_TRACK_1) ;
			GPIOD->ODR |= (PIN_TRACK_1<<1) ;
			GPIOD->ODR |= (PIN_TRACK_1<<2) ;
			GPIOD->ODR |= (PIN_TRACK_1<<3) ;
			GPIOD->ODR |= (PIN_TRACK_1<<4) ;
			GPIOD->ODR |= (PIN_TRACK_1<<5) ;
			GPIOD->ODR |= (PIN_TRACK_1<<6) ;
		} else {
			GPIOD->ODR &= ~(PIN_TRACK_1) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<1) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<2) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<3) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<4) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<5) ;
			GPIOD->ODR &= ~(PIN_TRACK_1<<6) ;
		}
		*/
		break;

	default:
		break;
	}
#endif
}
//--------------------------------------------------
/*
void trigger_tick()
{
	int i;
	for(i=0;i<NUM_PINS;i++)
	{
		if( (trigger_pulseActive[i]) && (systick_ticks > trigger_pulseTimes[i]) )
		{
			trigger_setPin(i,0);
			trigger_pulseActive[i] = 0;
		}
	}
}
*/
//--------------------------------------------------
/*
void trigger_pulsePin(uint8_t index)
{
	trigger_pulseTimes[index] = systick_ticks + PULSE_LENGTH;

	trigger_pulseActive[index] = 1;
	trigger_setPin(index,1);
}
*/


//--------------------------------------------------
#if 0
void trigger_init()
{

//	memset(trigger_pulseTimes,0,NUM_PINS*sizeof(uint32_t));
//	memset(trigger_pulseActive,0,NUM_PINS);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;


	//trigger output
	GPIO_InitStructure.GPIO_Pin = PIN_TRACK_1 | PIN_TRACK_2| PIN_TRACK_3 | PIN_TRACK_4 | PIN_TRACK_5 | PIN_TRACK_6 | PIN_TRACK_7 | PIN_CLOCK_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = PIN_CLOCK_2 | PIN_RESET;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//-------------- trigger input ---------------------------------------
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	 /* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//---- Reset In (PA8)------------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = PIN_RESET_IN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //maybe floating?
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Connect EXTI Line8 to PA8 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

	/* Configure EXTI Line8 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line8 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);



	//---- Clock In (PC9)------------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = PIN_CLOCK_IN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //maybe floating?
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Connect EXTI Line9 to PC9 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);

	/* Configure EXTI Line9 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line9 Interrupt  */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//---- Timer 2 ------------------------------------------------------
	//Timer 2 (32-bit) for time measuring of external clock pulses (trigger IO)
	TIM_TimeBaseInitTypeDef TIM_TimeBase_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	//HSE 168MHz, APB1 Prescaler = 4 => 42MHz
	TIM_TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBase_InitStructure.TIM_Period = 0xffffffff;
	TIM_TimeBase_InitStructure.TIM_Prescaler = 1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBase_InitStructure);

	TIM_Cmd(TIM2, ENABLE);

	//set trigger outs to low
	int i;
	for(i=0;i<NUM_PINS;i++)
	{
		trigger_setPin(i,0);
	}
}
#endif
//--------------------------------------------------
#if VOICE_TRIGGER_OUTS
void trigger_triggerVoice(uint8_t voice, triggerMode mode)
{
	switch(mode)
	{
	case TRIGGER_PULSE:
		trigger_pulsePin(TRIGGER_1 + voice);
		break;

	case TRIGGER_ON:
		trigger_setPin(TRIGGER_1 + voice,1);
		break;

	case TRIGGER_OFF:
		trigger_setPin(TRIGGER_1 + voice,0);
		break;
	}

}
#endif
//--------------------------------------------------
//cnt [0:23] for a 16 nothe
// => 96ppq (96/4 = 24pulses per 16th note
void trigger_clockTick(uint8_t onOff, uint8_t cnt)
{
#if 1
	uint8_t prescaler = 1;

	switch(trigger_dividerClockOut1)
	{
		case PRE_2_PPQ_OUT:

			prescaler = 48;
			break;

		default:
		case PRE_4_PPQ_OUT:
			prescaler = 24;
			break;

		case PRE_8_PPQ_OUT:
			prescaler = 12;
			break;

		case PRE_16_PPQ_OUT:
			prescaler = 6;
			break;

		case PRE_24_PPQ_OUT:
			prescaler = 4;
			break;

		case PRE_48_PPQ_OUT:
			prescaler = 2;
			break;
	}


	//if(onOff)
	//{

	if(trigger_nextPulseOut1%prescaler==0)
	{
		trigger_setPin(CLOCK_1,1);
	}
	else
	{
		trigger_setPin(CLOCK_1,0);
	}

		trigger_nextPulseOut1++;
		trigger_nextPulseOut1 = trigger_nextPulseOut1%96;

#else
	if(pos==1)
	{
		trigger_nextPulseOut1 = 0;
		//trigger_nextPulseOut2 = 0;
	}

	if(pos >= trigger_nextPulseOut1)
	{
		//trigger_pulsePin(CLOCK_1);
		trigger_setPin(CLOCK_1,1);
		trigger_nextPulseOut1 += trigger_dividerClockOut1;
	} else trigger_setPin(CLOCK_1,0);

	/*
	if(pos >= trigger_nextPulseOut2)
	{
		trigger_setPin(CLOCK_2,1);
		trigger_nextPulseOut2 += trigger_dividerClockOut2;
	}else trigger_setPin(CLOCK_2,0);
	*/
#endif
}
//--------------------------------------------------
void trigger_reset(uint8_t value)
{
	trigger_setPin(TRIGGER_RESET,1-value);

	if(value) {
		trigger_nextPulseOut1 = 0;
		//trigger_nextPulseOut2 = 0;
	}
	else
	{
		trigger_setPin(CLOCK_1,0);
	}
}
//--------------------------------------------------
/*
void trigger_tickPhaseCounter()
{
	int i;
	for(i=0;i<OUTPUT_DMA_SIZE;i++)
	{
		//only run one cycle of the phase counter
		// it is reset by the next incoming master clock pulse
		if( (trigger_phase + trigger_phaseInc) < trigger_prescalerClockInput)
		{
			trigger_phase += trigger_phaseInc;
		}
		if(trigger_phase >= trigger_phaseWrapCounter)
		{
			trigger_phaseWrapCounter++;
			// trigger a step
			// -> set time to next step to immediately (trigger next step)
			seq_setDeltaT(-1);
		}
	}
}
*/
//--------------------------------------------------
/*
uint8_t trigger_isGateModeOn()
{
	return trigger_gateMode;
}
//--------------------------------------------------
void trigger_setGatemode(uint8_t onOff)
{
	trigger_gateMode = onOff;
}
*/
//--------------------------------------------------
void trigger_allOff()
{
	trigger_setPin(TRIGGER_ALL, 0);
}

