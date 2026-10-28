/*
 * ledHandler.c
 *
 * Created: 25.04.2012 16:04:30
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 

#include "ledHandler.h"
#include "buttonHandler.h"
#include <string.h>
#include "menu/menu.h"
#include "main.h"
#include "Soundengine/globals.h"
//#include "Soundengine/Sequencer/sequencer.h"

//since we need an array to store the start time we have to limit the number of simultanousely pulsable LEDs
#define NUM_OF_PULSABLE_LEDS 8	//MAX 8 at the moment (because of led_pulsingLeds)
#define LED_PULSE_TIME_MS 50		/**<time a pulsed LED stays on in [ms]*/
#define LED_PULSE_TIME ((uint16_t)(LED_PULSE_TIME_MS*4))

#define NUM_OF_BLINKABLE_LEDS 4
#define LED_BLINK_TIME_MS 250
#define LED_BLINK_TIME ((uint16_t)(LED_BLINK_TIME_MS*4))

volatile uint8_t led_currentStepLed = 0;

/** we have to distinguish between 2 led modes.
there are the normal set value functions, and there are temporary operations.
For example the menu state or step indicator lights are normal mode
stuff like the chaselight or pulsing a select LED on voice trigger are temporary.
to make it possible that the temporary led control does not influence the normal/default LED state
the normal/default state is also saved in this array, while the temporary functions like flashing only alter the dout register directly.
this way a temporary led action can be undone by resetting its value to the led_originalLedState[] value
*/
volatile uint8_t led_originalLedState[NUM_OUTS/8];
volatile uint32_t led_pulseEndTime[NUM_OF_PULSABLE_LEDS];		/**< stores the time a LED was activated*/
volatile uint8_t led_pulseLedNumber[NUM_OF_PULSABLE_LEDS];	/**< stores the corresponding LED number*/
volatile uint8_t led_pulsingLeds;


//volatile uint16_t led_nextBlinkTime[NUM_OF_BLINKABLE_LEDS];		/**< stores the time a LED was activated*/
uint32_t led_nextBlinkTime=0;
volatile uint8_t led_blinkLedNumber[NUM_OF_BLINKABLE_LEDS];	/**< stores the corresponding LED number*/
volatile uint8_t led_blinkingLeds;
uint8_t led_blinkToggleState = 0;							//to synchronize all blinking LEDs
uint8_t led_blinkPrescaler = 0;
//--------------------------------------------
void led_init()
{
	//init bitfield to zero
	led_pulsingLeds = 0;	
	memset((void*)led_originalLedState,0,NUM_OUTS/8);
};
//--------------------------------------------

#if !EURORACK_MODULE
void led_clearSelectLeds()
{
	/*
	const uint8_t arrayPos = LED_PART_SELECT1/8;
	dout_outputData[arrayPos] = 0;
	led_originalLedState[arrayPos] = 0;
	*/

	dout_outputData[1] &= ~(0x0f);
	dout_outputData[2] &= ~(0x0f);

	led_originalLedState[1] &= ~(0x0f);
	led_originalLedState[2] &= ~(0x0f);

}
#endif
//--------------------------------------------
#if !EURORACK_MODULE
void led_initPatternSelectLeds()
{
	const uint8_t bank = menu_playedPattern/16;
	const uint8_t ledIdx = led_idx2StepLed(menu_playedPattern); //led_idx2SelectLed(menu_playedPattern);

	// a blinking LED shows the viewed pattern if different from the played pattern
	if(menu_playedPattern != menu_getViewedPattern())
	{
		led_setBlinkLed((uint8_t)(ledIdx) ,1);
	}		
	else
	{
		//no playing pattern is shown, so clear the seq leds.
		led_clearAllBlinkLeds();
		led_clearSequencerLeds();
	}

	//Show current pattern on step seq buttons if in current bank
	if(bank == menu_activePatternBank)
	{
		led_setValue(1,ledIdx);
	}

	//Bar /Bank buttons
	switch(menu_activePatternBank)
	{
		case 0:
			led_setValue(0, LED_PAGE_DOWN);
			led_setValue(0, LED_PAGE_UP);
			break;

		case 1:
			led_setValue(0, LED_PAGE_DOWN);
			led_setValue(1, LED_PAGE_UP);
			break;

		case 2:
			led_setValue(1, LED_PAGE_DOWN);
			led_setValue(0, LED_PAGE_UP);
			break;

		case 3:
			led_setValue(1, LED_PAGE_DOWN);
			led_setValue(1, LED_PAGE_UP);
			break;
	}

};
#endif
//--------------------------------------------
void led_clearLeds_1To8()
{
#if !EURORACK_MODULE
	uint8_t arrayPos = LED_VOICE1/8;
	dout_outputData[arrayPos] &= 0x80;
	led_originalLedState[arrayPos] &= 0x80;
#else


	dout_outputData[0] &= 0x0F;
	led_originalLedState[0] &= 0x0F;

	dout_outputData[1] &= 0xF0;
	led_originalLedState[1] &= 0xF0;
/*
	LED_1_GPIO_Port->BSRR = LED_1_Pin << 16;
	LED_2_GPIO_Port->BSRR = LED_2_Pin << 16;
	LED_3_GPIO_Port->BSRR = LED_3_Pin << 16;
	LED_4_GPIO_Port->BSRR = LED_4_Pin << 16;
	LED_5_GPIO_Port->BSRR = LED_5_Pin << 16;
	LED_6_GPIO_Port->BSRR = LED_6_Pin << 16;
	LED_7_GPIO_Port->BSRR = LED_7_Pin << 16;
	*/


#endif

}
//--------------------------------------------
#if !EURORACK_MODULE
void led_setActivePage(uint8_t pageNr)
{
	uint8_t ledIdx = led_idx2SelectLed(pageNr);

	//pageNr =(uint8_t)(pageNr + LED_PART_SELECT1);
	uint8_t arrayPos = ledIdx/8;
	uint8_t bitPos = ledIdx&0x7;
	
	//set the whole byte to clear the other leds
	//dout_outputData[arrayPos] = (uint8_t)(1<<bitPos);

	led_clearSelectLeds();
	dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);


	//since this is not a temp. change, store in led_originalLedState as well
	led_originalLedState[arrayPos] |= (uint8_t)(1<<bitPos);
};
#endif
//-------------------------------------------
uint8_t led_idx2SelectLed(uint8_t idx)
{
	uint8_t ledIdx = 0;
	switch(idx)
	{
		case 0:
			ledIdx = LED_1;
			break;
		case 1:
			ledIdx = LED_2;
			break;
		case 2:
			ledIdx = LED_3;
			break;
		case 3:
			ledIdx = LED_4;
			break;
		case 4:
			ledIdx = LED_5;
			break;
		case 5:
			ledIdx = LED_6;
			break;
		case 6:
			ledIdx = LED_7;
			break;
		case 7:
			ledIdx = LED_8;
			break;
	}

#if EURORACK_MODULE
//	ledIdx++;
#endif
	return ledIdx;
}
//---------------------------------------------

void led_setActiveSelectButton(uint8_t butNr)
{

	uint8_t ledIdx = led_idx2SelectLed(butNr);

	//butNr = (uint8_t)(butNr + LED_PART_SELECT1);
	const uint8_t arrayPos = ledIdx/8;
	const uint8_t bitPos = ledIdx&0x7;
	
	led_clearLeds_1To8();
	dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);

	//since this is not a temp. change, store in led_originalLedState as well
	led_originalLedState[arrayPos] |= (uint8_t)(1<<bitPos);

};
//---------------------------------------------
void led_setActiveVoiceLeds(uint8_t pattern)
{
#if !EURORACK_MODULE
	uint8_t arrayPos = LED_VOICE1/8;
	//clear upper 7 bits
	dout_outputData[arrayPos] &= 0x1;
	led_originalLedState[arrayPos] &= 0x1;
	//set the upper 7 bits
	dout_outputData[arrayPos] =  (uint8_t)(dout_outputData[arrayPos] | ( (pattern<<1)&0xFE));
	//since this is not a temp. change, store in led_originalLedState as well
	led_originalLedState[arrayPos] =  (uint8_t)(led_originalLedState[arrayPos] | (pattern<<1&0xFE));
#else
	for(uint8_t i=0; i<= 6; i++)
	{
		if(pattern & 1<<i)
		{
			led_setValue(1, LED_2+i);
		}
		else
		{
			led_setValue(0, LED_2+i);
		}
	}

#endif
}
//---------------------------------------------
void led_setActiveVoiceLedsTemp(uint8_t voiceNr)
{
	for(uint8_t i=1; i<= 7;  i++)
		{
			if(voiceNr == i)
			{
				led_setValueTemp(1, LED_1+i);
			}
			else
			{
				led_setValueTemp(0,LED_1+i);
			}
		}
}
//---------------------------------------------
void led_setActiveVoice(uint8_t voiceNr)
{
#if !EURORACK_MODULE
	voiceNr =  (uint8_t)(voiceNr + LED_VOICE1);
	uint8_t arrayPos = voiceNr/8;
	uint8_t bitPos = voiceNr&0x7;
	
	//clear upper 7 bit (1st bit is bar_up led)
	dout_outputData[arrayPos] &= 0x1;
	led_originalLedState[arrayPos] &= 0x1;
	//set active LED
	dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
	//since this is not a temp. change, store in led_originalLedState as well
	led_originalLedState[arrayPos] |= (uint8_t)(1<<bitPos);
#else
	for(uint8_t i=0; i<= 6;  i++)
	{
		if(voiceNr == i)
		{
			led_setValue(1, LED_1+i);
		}
		else
		{
			led_setValue(0,LED_1+i);
		}
	}
#endif
	

};
//---------------------------------------------
void led_setValue(uint8_t val, uint8_t ledNr)
{
#if !EURORACK_MODULE
	if(ledNr == LED_PAGE_DOWN)
	{
		if(val)
		{
			LED_43_GPIO_Port->ODR |= LED_43_Pin;
		}
		else
		{
			LED_43_GPIO_Port->ODR &= ~LED_43_Pin;
		}
	}
	else
#endif
	{
		uint8_t arrayPos = ledNr/8;
		uint8_t bitPos = ledNr&0x7;
		//clear the led
		dout_outputData[arrayPos] &= (uint8_t)~(1<<bitPos);
		led_originalLedState[arrayPos] &= (uint8_t)~(1<<bitPos);

		//set new value
		if(val)
		{
			dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
			led_originalLedState[arrayPos] |= (uint8_t)(1<<bitPos);

		}
	}

	return;
}
//-----------------------------------------------
void led_setValueTemp(uint8_t val, uint8_t ledNr)
{
	uint8_t arrayPos = ledNr/8;
	uint8_t bitPos = ledNr&0x7;
	//clear the led
	dout_outputData[arrayPos] &= (uint8_t)~(1<<bitPos);
	
	//set new value
	if(val)
	{
		dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
	}	
	
	return;
};
//-----------------------------------------------
void led_clearAll()
{
	memset(dout_outputData,0,NUM_OUTS/8);
	memset((void*)led_originalLedState,0,NUM_OUTS/8);
};
//-----------------------------------------------
void led_toggle(uint8_t ledNr)
{
	uint8_t arrayPos = ledNr/8;
	uint8_t bitPos = ledNr&0x7;

	uint8_t oldValue = (uint8_t)(dout_outputData[arrayPos]&(1<<bitPos));
	//clear the led
	dout_outputData[arrayPos] &= (uint8_t)~(1<<bitPos);
	led_originalLedState[arrayPos] &= (uint8_t)~(1<<bitPos);

	//set new value
	if(!oldValue)
	{
		dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
		led_originalLedState[arrayPos] |= (uint8_t)(1<<bitPos);
			
	}	
	return;	
};
//--------------------------------------------------------------------------
void led_toggleTemp(uint8_t ledNr)
{
	uint8_t arrayPos = ledNr/8;
	uint8_t bitPos = ledNr&0x7;
	uint8_t oldValue = (uint8_t)(dout_outputData[arrayPos]&(1<<bitPos));
	//clear the led
	dout_outputData[arrayPos] &= (uint8_t)~(1<<bitPos);
	//set new value
	if(!oldValue)
	{
		dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
	}		
	return;
};
//-----------------------------------------------------------------------------
#if !EURORACK_MODULE
uint8_t led_idx2StepLed(uint8_t idx)
{
	uint8_t ledNr;

	switch(idx%16)
	{
	    default:
		case 0:
			ledNr = LED_1;
			break;
		case 1:
			ledNr = LED_2;
			break;
		case 2:
			ledNr = LED_3;
			break;
		case 3:
			ledNr = LED_4;
			break;
		case 4:
			ledNr = LED_5;
			break;
		case 5:
			ledNr = LED_6;
			break;
		case 6:
			ledNr = LED_7;
			break;
		case 7:
			ledNr = LED_8;
			break;
		case 8:
			ledNr = LED_STEP9;
			break;
		case 9:
			ledNr = LED_STEP10;
			break;
		case 10:
			ledNr = LED_STEP11;
			break;
		case 11:
			ledNr = LED_STEP12;
			break;
		case 12:
			ledNr = LED_STEP13;
			break;
		case 13:
			ledNr = LED_STEP14;
			break;
		case 14:
			ledNr = LED_STEP15;
			break;
		case 15:
			ledNr = LED_STEP16;
			break;
	}
	return ledNr;
}

//--------------------------------------------------
void led_setActive_step(uint8_t stepNr)
{
#if 0
	stepNr /= 8; //128 steps auf 16 reduzieren
#endif
	uint8_t ledNr;

	ledNr = led_idx2StepLed(stepNr);
	//ledNr = (uint8_t)(stepNr+LED_STEP1);
		
	if(led_currentStepLed != ledNr)
	{
		led_reset(led_currentStepLed);
		led_currentStepLed = ledNr;
		led_toggleTemp(ledNr);
	}		
};
//-------------------------------------------------
void led_clearActive_step()
{
	led_reset(led_currentStepLed);
};
#endif
//-------------------------------------------------
#if !EURORACK_MODULE
void led_updateSeqLeds()
{
	//prevent steps showing up in manual trigger edit mode  (perf)
	if(buttonHandler_getMode() != SELECT_MODE_PERF)
	{
		uint8_t trackNr = menu_getActiveVoice();
		uint8_t patternNr = menu_shownPattern;
		for(uint8_t i=0; i<16; i++)
		{
			uint8_t isActive = seq_isMainStepActive(trackNr,menu_activeBar*16+i,patternNr);
			led_setValue(isActive,led_idx2StepLed(i));
		}

		//a query seq track also updated track length, so do it here?
		activeParameters[PAR_TRACK_LENGTH] = seq_getTrackLength(trackNr);

		//we also update the bar/page buttons here
		led_updateBarLeds();
	}
}

//-------------------------------------------------
void led_updateBarLeds()
{
	switch(menu_activeBar)
	{
		case 0:
			led_setValue(0, LED_PAGE_DOWN);
			led_setValue(0, LED_PAGE_UP);
			break;

		case 1:
			led_setValue(0, LED_PAGE_DOWN);
			led_setValue(1, LED_PAGE_UP);
			break;

		case 2:
			led_setValue(1, LED_PAGE_DOWN);
			led_setValue(0, LED_PAGE_UP);
			break;

		case 3:
			led_setValue(1, LED_PAGE_DOWN);
			led_setValue(1, LED_PAGE_UP);
			break;
	}
}
//-------------------------------------------------
void led_clearSequencerLeds()
{
	for(int i=0;i<16;i++)
	{
		uint8_t ledNr;
		ledNr = led_idx2StepLed(i);
		led_setValue(0,ledNr);
	}
};
//-------------------------------------------------
void led_clearSequencerLeds9_16()
{
	for(uint8_t i=0;i<8;i++)
	{
		uint8_t ledNr;
		ledNr = led_idx2StepLed(i+8);
		led_setValue(0,ledNr);
	}
};
//-------------------------------------------------
void led_clearSequencerLeds1_8()
{
	for(uint8_t i=0;i<8;i++)
	{
		uint8_t ledNr;
		ledNr = led_idx2StepLed(i);
		led_setValue(0,ledNr);
	}
};
#endif
//-------------------------------------------------
void led_setPage(uint8_t page)
{
	switch(page)
	{
	case VOICE1_PAGE:
		break;
	case VOICE2_PAGE:
		break;
	case VOICE3_PAGE:
		break;
	case VOICE4_PAGE:
		break;
	case VOICE5_PAGE:
		break;
	case VOICE6_PAGE:
		break;
	case VOICE7_PAGE:
		break;
	case MENU_MIDI_PAGE: // this is the global settings page
		break;
	case LOAD_PAGE:
		break;
	case SAVE_PAGE:
		break;
	case PERF_PAGE:
		break;
	case SEQ_PAGE:
		break;
	case EUKLID_PAGE:
		break;
	case PATTERN_SETTINGS_PAGE:
		break;
	case RECORDING_PAGE:
		break;
	case FX_MODE_PAGE:
		break;

	default:break;

	}
}

//-------------------------------------------------
void led_setMode2(uint8_t status)
{

#if EURORACK_MODULE

	led_setValue(0,LED_EDIT);
	led_setValue(0,LED_PERF);
	led_setValue(0,LED_MOD);
	led_setValue(0,LED_FX);
	led_setValue(0,LED_SAVE_LOAD);
	led_setValue(0,LED_SHIFT);


	led_setBlinkLed(LED_EDIT,0);
	led_setBlinkLed(LED_PERF,0);
	led_setBlinkLed(LED_MOD,0);
	led_setBlinkLed(LED_FX,0);
	led_setBlinkLed(LED_SAVE_LOAD, 0);
	led_setBlinkLed(LED_SHIFT, 0);


	switch(status)
		{
			case 0: //voice
				led_setValue(1,LED_EDIT);
			break;

			case 1: //perf
				led_setValue(1,LED_PERF);
			break;

			case 2: //perf
				led_setValue(1,LED_MOD);
			break;

			case 3: //load
				led_setValue(1,LED_SAVE_LOAD);
			break;

			case 4: //save
				led_setBlinkLed(LED_SAVE_LOAD,1);
			break;

			case 5: //FX
				led_setValue(1, LED_FX);
			break;

			case 6: //config
				led_setBlinkLed(LED_SHIFT,1);
				led_setBlinkLed(LED_SAVE_LOAD,1);
			break;


			default:
			break;
		}

#else
	led_setValue(0,LED_MODE1);
	led_setValue(0,LED_MODE2);
	led_setValue(0,LED_MODE3);
	led_setValue(0,LED_MODE4);	
	
	led_setBlinkLed(LED_MODE1,0);
	led_setBlinkLed(LED_MODE2,0);
	led_setBlinkLed(LED_MODE3,0);
	led_setBlinkLed(LED_MODE4,0);
			
	switch(status)
	{
		case 0:
			led_setValue(1,LED_MODE1);
		break;
		
		case 1:
			led_setValue(1,LED_MODE2);
		break;
		
		case 2: //load
			led_setValue(1,LED_MODE3);
		break;
		
		case 3: //save
			led_setValue(1,LED_MODE4);
		break;
		
		case 4:
			led_setBlinkLed(LED_MODE1,1);
		break;
		
		case 5:
			led_setBlinkLed(LED_MODE2,1);
		break;
		
		case 6:
			
		break;
		
		case 7:
			//led_setBlinkLed(LED_MODE4,1);
		break;
		
		default:
		break;
	}
#endif
};

//-------------------------------------------------------
void led_setMode2Leds(uint8_t value)
{
#if !EURORACK_MODULE
	led_clearSelectLeds();

	dout_outputData[LED_PART_SELECT1/8] |= value;
	led_originalLedState[LED_PART_SELECT1/8] |= value;
#else
	switch(value)
	{
	case 0:
		led_setValue(1, LED_EDIT);
		break;

	case 1:
		led_setValue(1, LED_PERF);
		break;

	case 2:
		led_setValue(1, LED_MOD);
		break;

	case 3:
		led_setValue(1, LED_FX);
		break;

	case 4: //config
		led_setValue(1, LED_FX);
		led_setValue(1, LED_MOD);
		break;

	default:break;
	}
#endif
};
//-------------------------------------------------------
void led_pulseLed(uint8_t ledNr)
{
	//search for a free pulse slot
	for(int i=0;i<NUM_OF_PULSABLE_LEDS;i++)
	{
		if( !(led_pulsingLeds & (1<<i)))
		{
			//we found a free slot	
			
			//store led number
			led_pulseLedNumber[i] = ledNr;
			//set slot to active
			led_pulsingLeds |= (uint8_t)(1<<i);
			//set light up time
			led_pulseEndTime[i] = systick_ticks  +LED_PULSE_TIME;
			//turn on LED
			led_toggleTemp(ledNr);
			
			break;
		}
	}		
};
//-------------------------------------------------------
void led_clearAllBlinkLeds()
{
	for(int i=0;i<NUM_OF_BLINKABLE_LEDS;i++)
	{
		//reset led
		led_reset(led_blinkLedNumber[i]);
		//set slot to inactive
		led_blinkingLeds &= (uint8_t)~(1<<i);
	}
};
//-------------------------------------------------------
void led_setBlinkLed(const uint8_t ledNr, const uint8_t onOff)
{
	if(onOff)
	{
		//--- turn on blinking ---
		//search for a free blink slot
		for(int i=0;i<NUM_OF_BLINKABLE_LEDS;i++)
		{
			if( !(led_blinkingLeds & (1<<i)))
			{
				//we found a free slot	
			
				//store led number
				led_blinkLedNumber[i] = ledNr;
				//set slot to active
				led_blinkingLeds |= (uint8_t)(1<<i);
				//turn on LED
				led_toggleTemp(ledNr);
			
				break;		
			}
		}	
	}		
	else
	{
		//--- turn off blinking ---
		//search the slot with matching ledNr
		for(int i=0;i<NUM_OF_BLINKABLE_LEDS;i++)
		{
			if(led_blinkLedNumber[i] == ledNr)
			{
				//we found the matching slot
				//set slot to inactive
				led_blinkingLeds &= (uint8_t)~(1<<i);
				led_reset(ledNr);
			}
		}
	}
};
//-------------------------------------------------------
void led_resetBlinkTimer()
{
	led_blinkToggleState = 0;							//to synchronize all blinking LEDs
	led_blinkPrescaler = 0;
}
void led_blinkHandler()
{

	//if(led_blinkPrescaler==0)
	//second condition is to prevent hanging LEDs if a systick overflow occurs
	if( (systick_ticks > led_nextBlinkTime) || ( (led_nextBlinkTime-systick_ticks) > LED_BLINK_TIME*2) )
	{
		led_nextBlinkTime = systick_ticks + LED_BLINK_TIME;
		led_blinkToggleState = 1- led_blinkToggleState;
		for(int i=0;i<NUM_OF_BLINKABLE_LEDS;i++)
		{
			//if the led is active at the moment
			if(led_blinkingLeds & (1<<i))
			{
				if(led_blinkToggleState)
					led_setValueTemp(1,led_blinkLedNumber[i]);
				else
					led_setValueTemp(0,led_blinkLedNumber[i]);

			}
		}

	}
//	led_blinkPrescaler++;
	//if(led_blinkPrescaler>=2)led_blinkPrescaler=0;
}
//-------------------------------------------------------
void led_tickHandler()
{
	for(int i=0;i<NUM_OF_PULSABLE_LEDS;i++)
	{
		//if the led is active at the moment
		if(led_pulsingLeds & (1<<i))
		{
			//if the on time has passed
			if(systick_ticks > led_pulseEndTime[i])
			{
					//set slot to inactive
					led_pulsingLeds &= (uint8_t)~(1<<i);
					// reset LED
					led_reset(led_pulseLedNumber[i]);
			}
		}			
	}
	
}

//--------------------------------------------------------------------------
void led_reset(uint8_t ledNr)
{
	uint8_t arrayPos = ledNr/8;
	uint8_t bitPos = ledNr&0x7;
		
	const uint8_t ledValue = (uint8_t)(led_originalLedState[arrayPos] & (1<<bitPos));
		
	if(ledValue!=0)
	{
		dout_outputData[arrayPos] |= (uint8_t)(1<<bitPos);
	}	
	else
	{
		dout_outputData[arrayPos] &= (uint8_t)~(1<<bitPos);
	}	 
};
//--------------------------------------------------------------------------
