/*
 * adcPots.c
 *
 * Created: 27.04.2012 16:15:14
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 
#include "IO/adcPots.h"
#include <string.h>
#include "menu/menu.h"
#include "math.h"
#include "stdlib.h"

#define HYSTERSIS     40
#define MIN( a, b ) (a < b) ? a : b
#define USE_ATAN2 0


INDTCMZ uint16_t adc_buf[ADC_BUF_LEN];
uint16_t adc_buf_old[ADC_BUF_LEN];

/*
uint16_t adc2_buf[ADC2_BUF_LEN];
uint16_t adc2_buf_old[ADC2_BUF_LEN];
*/
#if EURORACK_MODULE
float adc_CvValues[5];
#else
float adc_faderValues[6];
#endif
uint16_t adc_potValues[4];
//uint16_t adc_potAvg[4];

uint8_t adc_idx = 0;
uint8_t adc2_idx = 0;


//------------------------------------------------------------------------
#if AVR
void adc_init(void) {
 

  memset((void*)adc_potValues,0,8);
  memset((void*)adc_potAvg,0,8);
  uint16_t result;
 
  // AVCC as ref voltage
  ADMUX =  (1<<REFS0);
  
  // single conversion
  ADCSRA = (1<<ADPS2) |(1<<ADPS1) /*| (1<<ADPS0)*/;     // adc prescaler div 64 => 8mhz clock = 125 kHz (must be between 50 and 200kHz)
  ADCSRA |= (1<<ADEN);                  // ADC enable
 
  // dummy readout
  ADCSRA |= (1<<ADSC);                  // single readout
  while (ADCSRA & (1<<ADSC) ) {}        // wait to finish
  // read result 
  result = ADCW;
  (void)result; //ommit unused but set warning

}
//------------------------------------------------------------------------
uint16_t adc_read( uint8_t channel )
{

  //select channel
  ADMUX = (uint8_t)((ADMUX & ~(0x1F)) | (channel & 0x1F));
  ADCSRA |= (1<<ADSC);            // single readout
  while (ADCSRA & (1<<ADSC) ) {}  // wait to finish
  return ADCW;                    // read and return result

}
 //------------------------------------------------------------------------
uint16_t adc_readAvg( uint8_t channel, uint8_t nsamples )
{

  uint32_t sum = 0;
 
  for (uint8_t i = 0; i < nsamples; ++i ) {
    sum += adc_read( channel );
  }
  
  return (uint16_t)( sum / nsamples );

}
#endif

float atan2_approximation1(float y, float x)
{
    //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
    //Volkan SALMA

    const float ONEQTR_PI = M_PI / 4.0;
	const float THRQTR_PI = 3.0 * M_PI / 4.0;
	float r, angle;
	float abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if ( x < 0.0f )
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if ( y < 0.0f )
		return( -angle );     // negate if in quad III or IV
	else
		return( angle );


}
#if USE_ATAN2
#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f
// |error| < 0.005
float atan2_approximation2( float y, float x )
{
	if ( x == 0.0f )
	{
		if ( y > 0.0f ) return PIBY2_FLOAT;
		if ( y == 0.0f ) return 0.0f;
		return -PIBY2_FLOAT;
	}
	float atan;
	float z = y/x;
	if ( fabs( z ) < 1.0f )
	{
		atan = z/(1.0f + 0.28f*z*z);
		if ( x < 0.0f )
		{
			if ( y < 0.0f ) return atan - PI_FLOAT;
			return atan + PI_FLOAT;
		}
	}
	else
	{
		atan = PIBY2_FLOAT - z/(z*z + 0.28f);
		if ( y < 0.0f ) return atan - PI_FLOAT;
	}
	return atan;
}
//------------------------------------------------------------------------
#endif
uint16_t calcPosition(uint16_t a, uint16_t b)
{
#if USE_ATAN2
	float frac_a, frac_b;

	frac_a = (((float)a)/0xfff) * 2.f -1.f;
	frac_b = (((float)b)/0xfff) * 2.f -1.f;
	//const float ret = atan2f(frac_b, frac_a) * (180.f/M_PI) + 180.f ;
	const float ret = atan2_approximation1(frac_b, frac_a) * (180.f/M_PI) + 180.f ;

	return ret;
#else
	const float frac_a = (((float)a)/0xfff) * 180.f;
	const float frac_b = (((float)b)/0xfff) * 180.f;

	if(frac_a > 90)
		return frac_b;
	else
		return 360-frac_b;
#endif
}
//------------------------------------------------------------------------

void adc_checkPots()
{
#if AVR
	//adc_potAvg
	
	//if((adcCnt++)&0x07 == 0x00)
	{
		for(uint8_t i=0;i<4;i++)
		{	
		
			uint16_t newValue = adc_readAvg(i,4);//adc_read(i);
		
		   if ((newValue > ( adc_potValues[i] + HYSTERSIS)) ||
			   ( adc_potValues[i] > (newValue + HYSTERSIS)))
		   {
			   adc_potValues[i] = newValue;
			   menu_parseKnobValue(i,(uint8_t)(newValue>>2));
			   menu_repaintAll();
		   } 
		}
	}
#else

	//uint8_t i = adc_idx;
	for(uint8_t i=0;i<4;i++)
	{
		if( ((adc_buf[i*2]>>4) > ((adc_buf_old[i*2]>>4) + HYSTERSIS)) ||
			((adc_buf_old[i*2]>>4) > ((adc_buf[i*2]>>4) + HYSTERSIS)))
		{

			//slow down menu dtype
			const uint8_t paramDType = menu_getDisplayParameterType(i);
			uint8_t shiftAmount = 1;
			if(paramDType == DTYPE_MENU) shiftAmount = 4;


			adc_buf_old[i*2] = adc_buf[i*2];
			//>> 1 to reduce resolution and increase pot travel
			uint16_t newValue = calcPosition(adc_buf[i*2]>>4, adc_buf[i*2+1]>>4) >>shiftAmount;

		   if (newValue !=  adc_potValues[i] )
		   {
			   int16_t diff = newValue - adc_potValues[i];
			   if((paramDType == DTYPE_MENU)) diff *= 2;
			   adc_potValues[i] = newValue;

			   if( abs(diff) < 40 )
			   {
				   if(menu_parseKnobValue(i,diff>0?1:-1))
				   {
					  // menu_repaintAll();
					   menu_repaint();
					   //menu_fastRefreshDisplay();
					   /*
					   sendDisplayBuffer();
					   sendDisplayBuffer();
					   sendDisplayBuffer();
					   sendDisplayBuffer();
					   sendDisplayBuffer();
					   sendDisplayBuffer();
					   */
				   }
			   }
		   }
		}
	}

	adc_idx++;
	if(adc_idx>=4)adc_idx=0;
#endif
};
//------------------------------------------------------------------------
#if EURORACK_MODULE
void adc_checkCv()
{
	uint8_t i = adc2_idx+8;



	{
		if( ((adc_buf[i]>>4) > ((adc_buf_old[i]>>4) + (HYSTERSIS/8))) ||
			((adc_buf_old[i]>>4) > ((adc_buf[i]>>4) + (HYSTERSIS/8))))
		{
			//CVs are inverted by analogue frontend
			float newValue = 1.f - ((adc_buf[i]>>4) / 4096.f);
			adc_buf_old[i] = adc_buf[i];

			//if (newValue !=  adc_faderValues[i] )
			//{
		//	if(newValue < 0.01f) newValue = 0.f;
			adc_CvValues[i-8] = newValue;
			//}
		}
	}

	adc2_idx++;
	if(adc2_idx>=5)adc2_idx=0;
}

float adc_getCvValue(uint8_t idx)
{
	return adc_CvValues[idx];
}
#else
void adc_checkFaders()
{
#if USE_FADER_VOLUME
	uint8_t i = adc2_idx+8;
	//for(uint8_t i=0;i<4;i++)
	{
		if( ((adc_buf[i]>>4) > ((adc_buf_old[i]>>4) + HYSTERSIS)) ||
			((adc_buf_old[i]>>4) > ((adc_buf[i]>>4) + HYSTERSIS)))
		{
			float newValue = (adc_buf[i]>>4) / 4096.f;
			adc_buf_old[i] = adc_buf[i];

			//if (newValue !=  adc_faderValues[i] )
			//{
			if(newValue < 0.01f) newValue = 0.f;
				adc_faderValues[i-8] = newValue * newValue;
			//}
		}
	}

	adc2_idx++;
	if(adc2_idx>=6)adc2_idx=0;

#endif
};
//------------------------------------------------------------------------
float adc_getFaderVolume(uint8_t idx)
{
#if USE_FADER_VOLUME
	return adc_faderValues[idx];
#else
	return 1.f;
#endif
}
#endif
//------------------------------------------------------------------------

