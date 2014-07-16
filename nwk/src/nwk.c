/**
 * \file nwk.c
 *
 * \brief Network layer management functions implementation
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
 * $Id: nwk.c 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nwkPrivate.h"
#include "phy.h"

/*****************************************************************************
*****************************************************************************/
NwkIb_t nwkIb;

/*****************************************************************************
*****************************************************************************/
void NWK_Init(void)
{
  nwkIb.nwkSeqNum = 0;
  nwkIb.macSeqNum = 0;
  nwkIb.addr = 0;

  for (uint8_t i = 0; i < NWK_MAX_ENDPOINTS_AMOUNT; i++)
    nwkIb.endpoint[i] = NULL;

  nwkTxInit();
  nwkRxInit();
  nwkFrameInit();
  nwkDataReqInit();

#ifdef NWK_ENABLE_ROUTING
  nwkRouteInit();
#endif

#ifdef NWK_ENABLE_SECURITY
  nwkSecurityInit();
#endif
}

/*****************************************************************************
*****************************************************************************/
void NWK_SetAddr(uint16_t addr)
{
  nwkIb.addr = addr;
  PHY_SetShortAddr(addr);
}

/*****************************************************************************
*****************************************************************************/
void NWK_SetPanId(uint16_t panId)
{
  nwkIb.panId = panId;
  PHY_SetPanId(panId);
}

/*****************************************************************************
*****************************************************************************/
void NWK_OpenEndpoint(uint8_t id, bool (*handler)(NWK_DataInd_t *ind))
{
  nwkIb.endpoint[id] = handler;
}

#ifdef NWK_ENABLE_SECURITY
/*****************************************************************************
*****************************************************************************/
void NWK_SetSecurityKey(uint8_t *key)
{
  memcpy((uint8_t *)nwkIb.key, key, NWK_SECURITY_KEY_SIZE);
}
#endif

/*****************************************************************************
*****************************************************************************/
bool NWK_Busy(void)
{
  return nwkRxBusy() || nwkTxBusy() || nwkDataReqBusy() || PHY_Busy();
}

/*****************************************************************************
*****************************************************************************/
void NWK_SleepReq(void)
{
  PHY_Sleep();
}

/*****************************************************************************
*****************************************************************************/
void NWK_WakeupReq(void)
{
  PHY_Wakeup();
}

/*****************************************************************************
*****************************************************************************/
void NWK_TaskHandler(void)
{
  nwkRxTaskHandler();
  nwkTxTaskHandler();
  nwkDataReqTaskHandler();
#ifdef NWK_ENABLE_SECURITY
  nwkSecurityTaskHandler();
#endif
}
