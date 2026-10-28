/*
 * FIFO.h
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#ifndef FIFO_H_
#define FIFO_H_

#include "globals.h"

#define BUFFER_SIZE 64 // muss 2^n betragen (8, 16, 32, 64 ...)
#define BUFFER_MASK (BUFFER_SIZE-1) // Klammern auf keinen Fall vergessen

typedef struct FifoStruct
{
	  volatile uint8_t data[BUFFER_SIZE];
	  volatile uint8_t read; // zeigt auf das Feld mit dem ältesten Inhalt
	  volatile uint8_t write; // zeigt immer auf leeres Feld
}Fifo;


void fifo_init(Fifo* fifo);

uint8_t fifo_bufferIn(Fifo* fifo, uint8_t byte);

uint8_t fifo_bufferOut(Fifo* fifo, uint8_t *pByte);

void fifo_clear(Fifo* fifo);


//--
#define BUFFER_SIZE_BIG 64 // muss 2^n betragen (8, 16, 32, 64 ...)
#define BUFFER_MASK_BIG (BUFFER_SIZE_BIG-1) // Klammern auf keinen Fall vergessen

typedef struct FifoStructBig
{
	volatile  uint8_t data[BUFFER_SIZE_BIG];
	volatile uint8_t read; // zeigt auf das Feld mit dem ältesten Inhalt
	volatile uint8_t write; // zeigt immer auf leeres Feld
}FifoBig;


void fifoBig_init(FifoBig* fifo);

uint8_t fifoBig_bufferIn(FifoBig* fifo, uint8_t byte);

uint8_t fifoBig_bufferOut(FifoBig* fifo, uint8_t *pByte);

void fifoBig_clear(FifoBig* fifo);


#endif /* FIFO_H_ */
