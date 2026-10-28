/*
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */


#include "Soundengine/AudioCodecManager.h"

#include <string.h>
#include "Soundengine/datatypes.h"
//------------------------------------------------------------------
#if DMA_MODE_ACTIVE
INDTCMZ int16_t dma_buffer[OUTPUT_DMA_SIZE*4];
INDTCMZ int16_t dma_buffer2[OUTPUT_DMA_SIZE*4];
#endif

volatile INDTCMZ uint8_t bCurrentSampleValid = 0;
//int16_t audioOutBuffer[2];
//------------------------------------------------------------------
int CodecInit()
{
    dma_buffer[0] = 32756;
    codec_initCsCodec((uint32_t)dma_buffer, OUTPUT_DMA_SIZE*2,(uint32_t)dma_buffer2, OUTPUT_DMA_SIZE*2);
    return 0;
}
//------------------------------------------------------------------
