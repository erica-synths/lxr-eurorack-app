/*
 * FIFO.c
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "Soundengine/FIFO.h"
#include "string.h"

//-------------------------------------------------------------------------
void fifo_init(Fifo* fifo)
{
	memset((void*)fifo->data,0,BUFFER_SIZE);
	fifo->read = 0;
	fifo->write = 0;
}
//-------------------------------------------------------------------------
uint8_t fifo_bufferIn(Fifo* fifo, uint8_t byte)
{
  uint8_t next = ((fifo->write + 1) & BUFFER_MASK);
  if (fifo->read == next)
    return 0;
  fifo->data[fifo->write] = byte;
  fifo->write = next;
  return 1;
}
//-------------------------------------------------------------------------
uint8_t fifo_bufferOut(Fifo* fifo, uint8_t *pByte)
{
  if (fifo->read == fifo->write)
    return 0;
  *pByte = fifo->data[fifo->read];
  fifo->read = (fifo->read+1) & BUFFER_MASK;
  return 1;
}
//-------------------------------------------------------------------------
void fifo_clear(Fifo* fifo)
{
	fifo->read = fifo->write;
}
//-------------------------------------------------------------------------
void fifoBig_init(FifoBig* fifo)
{
	memset((void*)fifo->data,0,BUFFER_SIZE_BIG);
	fifo->read = 0;
	fifo->write = 0;
}
//-------------------------------------------------------------------------
uint8_t fifoBig_bufferIn(FifoBig* fifo, uint8_t byte)
{
  uint8_t next = ((fifo->write + 1) & BUFFER_MASK_BIG);
  if (fifo->read == next)
    return 0;
  fifo->data[fifo->write] = byte;
  fifo->write = next;
  return 1;
}
//-------------------------------------------------------------------------
uint8_t fifoBig_bufferOut(FifoBig* fifo, uint8_t *pByte)
{
  if (fifo->read == fifo->write)
    return 0;
  *pByte = fifo->data[fifo->read];
  fifo->read = (fifo->read+1) & BUFFER_MASK_BIG;
  return 1;
}
//-------------------------------------------------------------------------
void fifoBig_clear(FifoBig* fifo)
{
	fifo->read = fifo->write;
}
//----------------------------------------------------
