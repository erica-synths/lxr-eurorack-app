/*
 * uart.h
 *
 * Created: 26.04.2012 14:27:22
 *  Author: Julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */ 


#ifndef UART_H_
#define UART_H_


#include "main.h"
//#include <avr/interrupt.h>

#include "Soundengine/FIFO.h"



//-------------------------------------------
#define ACK 1
#define NACK -1
//-------------------------------------------
#define ACK 1
#define NACK -1
//-------------------------------------------


void uart_init();
uint8_t uart_putc(unsigned char c);
void uart_puts (char *s);
uint8_t uart_getc(uint8_t *data);

//waits for a ACK message on the uart
uint8_t uart_waitAck();
//check if there is some data received an parse it
void uart_checkAndParse();


#endif /* UART_H_ */
