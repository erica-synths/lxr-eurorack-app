/*
 * adcPots.h
 *
 * Created: 27.04.2012 15:51:29
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef ADCPOTS_H_
#define ADCPOTS_H_

#include "main.h"
#include "Soundengine/DSPAudio/1PoleLp.h"


#define USE_FADER_VOLUME 1

#define ADC_BUF_LEN 13
extern INDTCMZ uint16_t adc_buf[ADC_BUF_LEN];



/*
#define ADC2_BUF_LEN 6
extern uint16_t adc2_buf[ADC2_BUF_LEN];
*/

//void adc_init(void);
 
//uint16_t adc_read( uint8_t channel );
//uint16_t adc_readAvg( uint8_t channel, uint8_t nsamples );
//check the connected potentiometers and parse their values
void adc_checkPots();

void adc_checkCv();
float adc_getCvValue(uint8_t idx);

//void adc_checkFaders();

//float adc_getFaderVolume(uint8_t idx);
 


#endif /* ADCPOTS_H_ */
