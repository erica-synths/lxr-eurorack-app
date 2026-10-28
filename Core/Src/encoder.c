/*
 * encoder.c
 *
 * Rotary encoder and push-button input.
 *
 * Rotation: the two phase signals form a 2-bit Gray code. Each 1 ms sample is
 * compared with the previous one through a transition table that yields one
 * quarter step forwards or backwards (or nothing for no change, or for an
 * impossible jump of two states caused by contact bounce or a missed sample).
 * The encoder has one full cycle (four quarter steps) per detent and always
 * rests in the same state, so a detent is counted whenever that rest state is
 * reached after moving at least half a cycle. This re-synchronises at every
 * detent, so bounce or a missed sample cannot shift the count permanently.
 *
 * Button: sampled every 10 ms; a new level is accepted once it has been
 * stable for BUTTON_STABLE_SAMPLES samples in a row.
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */
#include "encoder.h"

#define BUTTON_SAMPLE_PERIOD_MS  10
#define BUTTON_STABLE_SAMPLES    4

/* encode_read4() hands out one detent per call: several menu handlers only
 * bounds-check steps of +/-1. Detents turned while the main loop is busy (e.g.
 * SD card access) are queued, up to this many, so the value doesn't keep
 * moving long after the knob has stopped. */
#define MAX_PENDING_DETENTS      4

/* Quarter-step movement for (previous state << 2 | new state), state = A<<1 | B.
 * Forward (+1) is the sequence 00 -> 01 -> 11 -> 10 -> 00. */
static const int8_t quarterStepTable[16] = {
	 0, +1, -1,  0,
	-1,  0,  0, +1,
	+1,  0,  0, -1,
	 0, -1, +1,  0,
};

static uint8_t encState;          /* last sampled phase state, bits: A<<1 | B */
static uint8_t encRestState;      /* phase state at a detent */
static uint8_t encInitialised;
static int8_t  encQuarterSteps;   /* movement since the last detent */
static volatile int8_t encDetents;   /* detents not yet read by the main loop */

static uint8_t btnTimer;
static uint8_t btnStableCount;
static uint8_t btnRawLast = 1;    /* 1 = released (input pulled high) */
static volatile uint8_t btnPressed;

static uint8_t readPhases(void)
{
	return (uint8_t)((PHASE_A << 1) | PHASE_B);
}

void encode_init(void)
{
	encState = readPhases();
	encRestState = encState;
	encQuarterSteps = 0;
	encDetents = 0;
	encInitialised = 1;
}

static void sampleRotation(void)
{
	const uint8_t state = readPhases();

	if (!encInitialised)
	{
		encode_init();
		return;
	}
	if (state == encState)
		return;

	encQuarterSteps = (int8_t)(encQuarterSteps + quarterStepTable[(encState << 2) | state]);
	encState = state;

	if (state == encRestState)
	{
		/* Arrived at a detent: at least half a cycle in one direction counts. */
		if (encQuarterSteps >= 2 && encDetents < MAX_PENDING_DETENTS)
			encDetents++;
		else if (encQuarterSteps <= -2 && encDetents > -MAX_PENDING_DETENTS)
			encDetents--;
		encQuarterSteps = 0;
	}
}

static void sampleButton(void)
{
	if (++btnTimer < BUTTON_SAMPLE_PERIOD_MS)
		return;
	btnTimer = 0;

	const uint8_t raw = (uint8_t)ENCODER_BUTTON;
	if (raw != btnRawLast)
	{
		btnRawLast = raw;
		btnStableCount = 1;
	}
	else if (btnStableCount < BUTTON_STABLE_SAMPLES)
	{
		if (++btnStableCount == BUTTON_STABLE_SAMPLES)
			btnPressed = (uint8_t)(raw == 0);  /* active low */
	}
}

void encoder_callback(void)
{
	sampleRotation();
	sampleButton();
}

int8_t encode_read4(void)
{
	int8_t result = 0;

	/* encDetents is also changed by encoder_callback() in the SysTick interrupt. */
	const uint32_t primask = __get_PRIMASK();
	__disable_irq();
	if (encDetents > 0)
	{
		encDetents--;
		result = 1;
	}
	else if (encDetents < 0)
	{
		encDetents++;
		result = -1;
	}
	__set_PRIMASK(primask);

	return result;
}

uint8_t encode_readButton(void)
{
	return btnPressed;
}
