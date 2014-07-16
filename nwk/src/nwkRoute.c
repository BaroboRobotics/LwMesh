/**
 * \file nwkRoute.c
 *
 * \brief Routing implementation
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
 * $Id: nwkRoute.c 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "nwk.h"
#include "nwkPrivate.h"
#include "sysTypes.h"

#ifdef NWK_ENABLE_ROUTING

/*****************************************************************************
*****************************************************************************/
#define NWK_ROUTE_UNKNOWN           0xffff
#define NWK_ROUTE_TRANSIT_MASK      0x8000

/*****************************************************************************
*****************************************************************************/
typedef struct NwkRouteTableRecord_t
{
  uint16_t   dst;
  uint16_t   nextHop;
  uint8_t    score;
  uint8_t    lqi;
} NwkRouteTableRecord_t;

/*****************************************************************************
*****************************************************************************/
static void nwkRouteTxFrameConf(NwkFrame_t *frame);
static void nwkRouteSendRouteError(uint16_t src, uint16_t dst);
static void nwkRouteErrorConf(NwkFrame_t *frame);

/*****************************************************************************
*****************************************************************************/
static NwkRouteTableRecord_t nwkRouteTable[NWK_ROUTE_TABLE_SIZE];

/*****************************************************************************
*****************************************************************************/
void nwkRouteInit(void)
{
  for (uint8_t i = 0; i < NWK_ROUTE_TABLE_SIZE; i++)
    nwkRouteTable[i].dst = NWK_ROUTE_UNKNOWN;
}

/*****************************************************************************
*****************************************************************************/
static NwkRouteTableRecord_t *nwkRouteFindRecord(uint16_t dst)
{
  for (uint8_t i = 0; i < NWK_ROUTE_TABLE_SIZE; i++)
    if (nwkRouteTable[i].dst == dst)
      return &nwkRouteTable[i];

  if (NWK_ROUTE_UNKNOWN == dst)
    return &nwkRouteTable[NWK_ROUTE_TABLE_SIZE - 1];

  return NULL;
}

/*****************************************************************************
*****************************************************************************/
void nwkRouteRemove(uint16_t dst)
{
  NwkRouteTableRecord_t *rec;

  rec = nwkRouteFindRecord(dst);
  if (rec)
    rec->dst = NWK_ROUTE_UNKNOWN;
}

/*****************************************************************************
*****************************************************************************/
void nwkRouteFrameReceived(NwkFrame_t *frame)
{
  NwkRouteTableRecord_t *rec;
  NwkFrameHeader_t *header = &frame->data.header;

  if ((header->macSrcAddr & NWK_ROUTE_TRANSIT_MASK) &&
      (header->macSrcAddr != header->nwkSrcAddr))
    return;

  if (0xffff == header->macDstPanId)
    return;

  rec = nwkRouteFindRecord(header->nwkSrcAddr);
  if (rec)
  {
    if (rec->nextHop != header->macSrcAddr && frame->rx.lqi > rec->lqi)
    {
      rec->nextHop = header->macSrcAddr;
      rec->score = NWK_ROUTE_DEFAULT_SCORE;
    }
  }
  else
  {
    rec = nwkRouteFindRecord(NWK_ROUTE_UNKNOWN);

    rec->dst = header->nwkSrcAddr;
    rec->nextHop = header->macSrcAddr;
    rec->score = NWK_ROUTE_DEFAULT_SCORE;
  }

  rec->lqi = frame->rx.lqi;
}

/*****************************************************************************
*****************************************************************************/
void nwkRouteFrameSent(NwkFrame_t *frame)
{
  NwkRouteTableRecord_t *rec;

  rec = nwkRouteFindRecord(frame->data.header.nwkDstAddr);
  if (NULL == rec)
    return;

  if (NWK_SUCCESS_STATUS == frame->tx.status)
  {
    rec->score = NWK_ROUTE_DEFAULT_SCORE;
  }
  else
  {
    rec->score--;
    if (0 == rec->score)
    {
      rec->dst = NWK_ROUTE_UNKNOWN;
      return;
    }
  }

  if ((rec - &nwkRouteTable[0]) > 0)
  {
    NwkRouteTableRecord_t *prev = rec - 1;
    NwkRouteTableRecord_t tmp;

    tmp = *prev;
    *prev = *rec;
    *rec = tmp;
  }
}

/*****************************************************************************
*****************************************************************************/
uint16_t nwkRouteNextHop(uint16_t dst)
{
  if (0xffff == dst)
    return NWK_ROUTE_UNKNOWN;

  for (uint8_t i = 0; i < NWK_ROUTE_TABLE_SIZE; i++)
    if (nwkRouteTable[i].dst == dst)
      return nwkRouteTable[i].nextHop;

  return NWK_ROUTE_UNKNOWN;
}

/*****************************************************************************
*****************************************************************************/
void nwkRouteFrame(NwkFrame_t *frame)
{
  if (NWK_ROUTE_UNKNOWN != nwkRouteNextHop(frame->data.header.nwkDstAddr))
  {
    frame->tx.confirm = nwkRouteTxFrameConf;
    frame->tx.control = NWK_TX_CONTROL_ROUTING;
    nwkTxFrame(frame);
  }
  else
  {
    nwkRouteSendRouteError(frame->data.header.nwkSrcAddr, frame->data.header.nwkDstAddr);
    nwkFrameFree(frame);
  }
}

/*****************************************************************************
*****************************************************************************/
static void nwkRouteTxFrameConf(NwkFrame_t *frame)
{
  nwkFrameFree(frame);
}

/*****************************************************************************
*****************************************************************************/
static void nwkRouteSendRouteError(uint16_t src, uint16_t dst)
{
  NwkFrame_t *frame;
  NwkRouteErrorCommand_t *command;

  if (NULL == (frame = nwkFrameAlloc(sizeof(NwkRouteErrorCommand_t))))
    return;

  nwkFrameCommandInit(frame);

  frame->tx.confirm = nwkRouteErrorConf;

  frame->data.header.nwkDstAddr = src;

  command = (NwkRouteErrorCommand_t *)frame->data.payload;

  command->id = NWK_COMMAND_ROUTE_ERROR;
  command->srcAddr = src;
  command->dstAddr = dst;

  nwkTxFrame(frame);
}

/*****************************************************************************
*****************************************************************************/
static void nwkRouteErrorConf(NwkFrame_t *frame)
{
  nwkFrameFree(frame);
}

/*****************************************************************************
*****************************************************************************/
void nwkRouteErrorReceived(NWK_DataInd_t *ind)
{
  NwkRouteErrorCommand_t *command = (NwkRouteErrorCommand_t *)ind->data;

  nwkRouteRemove(command->dstAddr);
}

/*****************************************************************************
*****************************************************************************/
uint16_t NWK_RouteNextHop(uint16_t dst)
{
  return nwkRouteNextHop(dst);
}

#endif // NWK_ENABLE_ROUTING
