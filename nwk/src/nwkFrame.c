/**
 * \file nwkFrame.c
 *
 * \brief Frame buffers management implementation
 *
 * Copyright (C) 2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 * $Id: nwkFrame.c 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nwkPrivate.h"

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_FRAME_STATE_FREE = 0x00,
};

/*****************************************************************************
*****************************************************************************/
static NwkFrame_t nwkFrameFrames[NWK_BUFFERS_AMOUNT];

/*****************************************************************************
*****************************************************************************/
void nwkFrameInit(void)
{
  for (int i = 0; i < NWK_BUFFERS_AMOUNT; i++)
    nwkFrameFrames[i].state = NWK_FRAME_STATE_FREE;
}

/*****************************************************************************
*****************************************************************************/
NwkFrame_t *nwkFrameAlloc(uint8_t size)
{
  for (int i = 0; i < NWK_BUFFERS_AMOUNT; i++)
  {
    if (NWK_FRAME_STATE_FREE == nwkFrameFrames[i].state)
    {
      nwkFrameFrames[i].size = sizeof(NwkFrameHeader_t) + size;
      return &nwkFrameFrames[i];
    }
  }
  return NULL;
}

/*****************************************************************************
*****************************************************************************/
void nwkFrameFree(NwkFrame_t *frame)
{
  frame->state = NWK_FRAME_STATE_FREE;
}

/*****************************************************************************
*****************************************************************************/
NwkFrame_t *nwkFrameByIndex(uint8_t i)
{
  return &nwkFrameFrames[i];
}

/*****************************************************************************
*****************************************************************************/
void nwkFrameCommandInit(NwkFrame_t *frame)
{
  frame->tx.status = NWK_SUCCESS_STATUS;
  frame->tx.timeout = 0;
  frame->tx.control = 0;
  frame->tx.confirm = NULL;

  frame->data.header.nwkFcf.ackRequest = 0;
  frame->data.header.nwkFcf.securityEnabled = 0;
  frame->data.header.nwkFcf.linkLocal = 0;
  frame->data.header.nwkFcf.reserved = 0;
  frame->data.header.nwkSeq = ++nwkIb.nwkSeqNum;
  frame->data.header.nwkSrcAddr = nwkIb.addr;
  frame->data.header.nwkDstAddr = 0;
  frame->data.header.nwkSrcEndpoint = 0;
  frame->data.header.nwkDstEndpoint = 0;
}
