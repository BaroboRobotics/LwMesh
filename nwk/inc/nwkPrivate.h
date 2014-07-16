/**
 * \file nwkPrivate.h
 *
 * \brief Network layer private interface
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
 * $Id: nwkPrivate.h 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#ifndef _NWK_PRIVATE_H_
#define _NWK_PRIVATE_H_

#include <stdint.h>
#include "nwk.h"
#include "sysTypes.h"

/*****************************************************************************
*****************************************************************************/
#define NWK_SECURITY_MIC_SIZE    4
#define NWK_SECURITY_KEY_SIZE    16
#define NWK_SECURITY_BLOCK_SIZE  16

/*****************************************************************************
*****************************************************************************/
enum
{
  NWK_COMMAND_ACK              = 0x00,
  NWK_COMMAND_ROUTE_ERROR      = 0x01,
};

enum
{
  NWK_TX_CONTROL_BROADCAST_PAN_ID = 1 << 0,
  NWK_TX_CONTROL_ROUTING          = 1 << 1,
};

/*****************************************************************************
*****************************************************************************/
typedef struct PACK NwkFrameHeader_t
{
  uint16_t    macFcf;
  uint8_t     macSeq;
  uint16_t    macDstPanId;
  uint16_t    macDstAddr;
  uint16_t    macSrcAddr;

  struct PACK
  {
    uint8_t   ackRequest       : 1;
    uint8_t   securityEnabled  : 1;
    uint8_t   linkLocal        : 1;
    uint8_t   reserved         : 5;
  }           nwkFcf;
  uint8_t     nwkSeq;
  uint16_t    nwkSrcAddr;
  uint16_t    nwkDstAddr;
  struct PACK
  {
    uint8_t   nwkSrcEndpoint   : 4;
    uint8_t   nwkDstEndpoint   : 4;
  };
} NwkFrameHeader_t;

typedef struct NwkFrame_t
{
  uint8_t            state;
  uint8_t            size;
  struct PACK
  {
    NwkFrameHeader_t header;
    uint8_t          payload[NWK_MAX_PAYLOAD_SIZE];
  } data;

  union
  {
    struct
    {
      uint8_t        lqi;
      int8_t         rssi;
    } rx;

    struct
    {
      uint8_t        status;
      uint16_t       timeout;
      uint8_t        control;
      void           (*confirm)(struct NwkFrame_t *frame);
    } tx;
  };
} NwkFrame_t;

typedef struct PACK NwkAckCommand_t
{
  uint8_t    id;
  uint8_t    seq;
  uint8_t    control;
} NwkAckCommand_t;

typedef struct PACK NwkRouteErrorCommand_t
{
  uint8_t    id;
  uint16_t   srcAddr;
  uint16_t   dstAddr;
} NwkRouteErrorCommand_t;

typedef struct NwkIb_t
{
  uint16_t     addr;
  uint16_t     panId;
  uint8_t      nwkSeqNum;
  uint8_t      macSeqNum;
  bool         (*endpoint[NWK_MAX_ENDPOINTS_AMOUNT])(NWK_DataInd_t *ind);
#ifdef NWK_ENABLE_SECURITY
  uint32_t     key[4];
#endif
} NwkIb_t;

/*****************************************************************************
*****************************************************************************/
extern NwkIb_t nwkIb;

/*****************************************************************************
*****************************************************************************/
void nwkFrameInit(void);
NwkFrame_t *nwkFrameAlloc(uint8_t size);
void nwkFrameFree(NwkFrame_t *frame);
NwkFrame_t *nwkFrameByIndex(uint8_t i);
void nwkFrameCommandInit(NwkFrame_t *frame);

void nwkRxInit(void);
bool nwkRxBusy(void);
void nwkRxDecryptConf(NwkFrame_t *frame, bool status);
void nwkRxTaskHandler(void);

void nwkTxInit(void);
void nwkTxFrame(NwkFrame_t *frame);
void nwkTxBroadcastFrame(NwkFrame_t *frame);
void nwkTxAckReceived(NWK_DataInd_t *ind);
bool nwkTxBusy(void);
void nwkTxEncryptConf(NwkFrame_t *frame);
void nwkTxTaskHandler(void);

void nwkDataReqInit(void);
bool nwkDataReqBusy(void);
void nwkDataReqTaskHandler(void);

#ifdef NWK_ENABLE_ROUTING
void nwkRouteInit(void);
void nwkRouteRemove(uint16_t dst);
void nwkRouteFrameReceived(NwkFrame_t *frame);
void nwkRouteFrameSent(NwkFrame_t *frame);
uint16_t nwkRouteNextHop(uint16_t dst);
void nwkRouteFrame(NwkFrame_t *frame);
void nwkRouteErrorReceived(NWK_DataInd_t *ind);
#endif

#ifdef NWK_ENABLE_SECURITY
void nwkSecurityInit(void);
void nwkSecurityProcess(NwkFrame_t *frame, bool encrypt);
void nwkSecurityTaskHandler(void);
#endif

#endif // _NWK_PRIVATE_H_
