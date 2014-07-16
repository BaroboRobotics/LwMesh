/**
 * \file WSNDemo.c
 *
 * \brief WSNDemo application implementation
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
 * $Id: WSNDemo.c 5245 2012-09-10 20:07:02Z ataradov $
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "config.h"
#include "hal.h"
#include "phy.h"
#include "sys.h"
#include "nwk.h"
#include "halUart.h"
#include "halSleep.h"
#include "sysTimer.h"
#include "leds.h"

#ifdef APP_ENABLE_OTA
  #include "otaClient.h"
#endif

/*****************************************************************************
*****************************************************************************/
#if APP_ADDR == 0
  #define APP_CAPTION     "Coordinator"
  #define APP_NODE_TYPE   0
  #define APP_COORDINATOR 1
#elif APP_ADDR < 0x8000
  #define APP_CAPTION     "Router"
  #define APP_NODE_TYPE   1
  #define APP_ROUTER      1
#else
  #define APP_CAPTION     "End Device"
  #define APP_NODE_TYPE   2
  #define APP_ENDDEVICE   1
#endif

#define APP_CAPTION_SIZE  (sizeof(APP_CAPTION) - 1)

#define LED_NETWORK       0
#define LED_DATA          1

/*****************************************************************************
*****************************************************************************/
typedef struct AppMessage_t
{
  uint8_t     messageType;
  uint8_t     nodeType;
  uint64_t    extAddr;
  uint16_t    shortAddr;
  uint32_t    softVersion;
  uint32_t    channelMask;
  uint16_t    panId;
  uint8_t     workingChannel;
  uint16_t    parentShortAddr;
  uint8_t     lqi;
  int8_t      rssi;

  struct
  {
    uint8_t   type;
    uint8_t   size;
    int32_t   battery;
    int32_t   temperature;
    int32_t   light;
  } sensors;

  struct
  {
    uint8_t   type;
    uint8_t   size;
    char      text[APP_CAPTION_SIZE];
  } caption;
} AppMessage_t;

typedef enum AppState_t
{
  APP_STATE_INITIAL,
  APP_STATE_SEND,
  APP_STATE_WAIT_CONF,
  APP_STATE_SENDING_DONE,
  APP_STATE_WAIT_SEND_TIMER,
  APP_STATE_PREPARE_TO_SLEEP,
  APP_STATE_SLEEP,
  APP_STATE_WAKEUP,
} AppState_t;

/*****************************************************************************
*****************************************************************************/
static AppState_t appState = APP_STATE_INITIAL;

#if APP_ROUTER || APP_ENDDEVICE
static NWK_DataReq_t nwkDataReq;
static SYS_Timer_t appNetworkStatusTimer;
static bool appNetworkStatus;
#endif

static AppMessage_t msg;
static SYS_Timer_t appDataSendingTimer;

/*****************************************************************************
*****************************************************************************/
void HAL_UartBytesReceived(uint16_t bytes)
{
  for (uint16_t i = 0; i < bytes; i++)
    HAL_UartReadByte();
  ledToggle(2);
}

/*****************************************************************************
*****************************************************************************/
static void appSendMessage(uint8_t *data, uint8_t size)
{
  uint8_t cs = 0;

  HAL_UartWriteByte(0x10);
  HAL_UartWriteByte(0x02);

  for (uint8_t i = 0; i < size; i++)
  {
    if (data[i] == 0x10)
    {
      HAL_UartWriteByte(0x10);
      cs += 0x10;
    }
    HAL_UartWriteByte(data[i]);
    cs += data[i];
  }

  HAL_UartWriteByte(0x10);
  HAL_UartWriteByte(0x03);
  cs += 0x10 + 0x02 + 0x10 + 0x03;

  HAL_UartWriteByte(cs);
}

/*****************************************************************************
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind)
{
  AppMessage_t *msg = (AppMessage_t *)ind->data;

  ledToggle(LED_DATA);

  msg->lqi = ind->lqi;
  msg->rssi = ind->rssi;

  appSendMessage(ind->data, ind->size);
  return true;
}

/*****************************************************************************
*****************************************************************************/
static void appDataSendingTimerHandler(SYS_Timer_t *timer)
{
  if (APP_STATE_WAIT_SEND_TIMER == appState)
    appState = APP_STATE_SEND;
  else
    SYS_TimerStart(&appDataSendingTimer);

  (void)timer;
}

#if APP_ROUTER || APP_ENDDEVICE
/*****************************************************************************
*****************************************************************************/
static void appNetworkStatusTimerHandler(SYS_Timer_t *timer)
{
  ledToggle(LED_NETWORK);
  (void)timer;
}
#endif

/*****************************************************************************
*****************************************************************************/
#if APP_ROUTER || APP_ENDDEVICE
static void appDataConf(NWK_DataReq_t *req)
{
  ledOff(LED_DATA);

  if (NWK_SUCCESS_STATUS == req->status)
  {
    if (!appNetworkStatus)
    {
      ledOn(LED_NETWORK);
      SYS_TimerStop(&appNetworkStatusTimer);
      appNetworkStatus = true;
    }
  }
  else
  {
    msg.sensors.light++;

    if (appNetworkStatus)
    {
      ledOff(LED_NETWORK);
      SYS_TimerStart(&appNetworkStatusTimer);
      appNetworkStatus = false;
    }
  }

  appState = APP_STATE_SENDING_DONE;
}
#endif

/*****************************************************************************
*****************************************************************************/
static void appSendData(void)
{
#ifdef NWK_ENABLE_ROUTING
  msg.parentShortAddr = NWK_RouteNextHop(0);
#else
  msg.parentShortAddr = 0;
#endif

  msg.sensors.battery     = rand();
  msg.sensors.temperature = rand() & 0x7f;
//  msg.sensors.light       = rand() & 0xff;

#if APP_COORDINATOR
  appSendMessage((uint8_t *)&msg, sizeof(msg));
  SYS_TimerStart(&appDataSendingTimer);
  appState = APP_STATE_WAIT_SEND_TIMER;
#else
  nwkDataReq.dstAddr = 0;
  nwkDataReq.dstEndpoint = APP_ENDPOINT;
  nwkDataReq.srcEndpoint = APP_ENDPOINT;
  nwkDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
  nwkDataReq.data = (uint8_t *)&msg;
  nwkDataReq.size = sizeof(msg);
  nwkDataReq.confirm = appDataConf;

  ledOn(LED_DATA);
  NWK_DataReq(&nwkDataReq);

  appState = APP_STATE_WAIT_CONF;
#endif
}

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
/*****************************************************************************
*****************************************************************************/
void PHY_RandomConf(uint16_t rnd)
{
  srand(rnd);
}
#endif

/*****************************************************************************
*****************************************************************************/
static void appInit(void)
{
  msg.messageType          = 1;
  msg.nodeType             = APP_NODE_TYPE;
  msg.extAddr              = APP_ADDR;
  msg.shortAddr            = APP_ADDR;
  msg.softVersion          = 0x01010100;
  msg.channelMask          = (1L << APP_CHANNEL);
  msg.panId                = APP_PANID;
  msg.workingChannel       = APP_CHANNEL;
  msg.parentShortAddr      = 0;
  msg.lqi                  = 0;
  msg.rssi                 = 0;

  msg.sensors.type        = 1;
  msg.sensors.size        = sizeof(int32_t) * 3;
  msg.sensors.battery     = 0;
  msg.sensors.temperature = 0;
  msg.sensors.light       = 0;

  msg.caption.type         = 32;
  msg.caption.size         = APP_CAPTION_SIZE;
  memcpy(msg.caption.text, APP_CAPTION, APP_CAPTION_SIZE);

#if APP_COORDINATOR
  // Enable RCB_BB RS232 level converter
  #if defined(PLATFORM_RCB128RFA1)
    DDRD = (1 << 4) | (1 << 6) | (1 << 7);
    PORTD = (0 << 4) | (1 << 6) | (1 << 7);
  #endif

  #if defined(PLATFORM_RCB231)
    DDRC = (1 << 4) | (1 << 6) | (1 << 7);
    PORTC = (0 << 4) | (1 << 6) | (1 << 7);
  #endif
#endif

  ledsInit();

  NWK_SetAddr(APP_ADDR);
  NWK_SetPanId(APP_PANID);
  PHY_SetChannel(APP_CHANNEL);
  PHY_SetRxState(true);

#ifdef NWK_ENABLE_SECURITY
  NWK_SetSecurityKey((uint8_t *)APP_SECURITY_KEY);
#endif

  NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

  appDataSendingTimer.interval = APP_SENDING_INTERVAL;
  appDataSendingTimer.mode = SYS_TIMER_INTERVAL_MODE;
  appDataSendingTimer.handler = appDataSendingTimerHandler;

#if APP_ROUTER || APP_ENDDEVICE
  appNetworkStatus = false;
  appNetworkStatusTimer.interval = 500;
  appNetworkStatusTimer.mode = SYS_TIMER_PERIODIC_MODE;
  appNetworkStatusTimer.handler = appNetworkStatusTimerHandler;
  SYS_TimerStart(&appNetworkStatusTimer);
#else
  ledOn(LED_NETWORK);
#endif

#ifdef PHY_ENABLE_RANDOM_NUMBER_GENERATOR
  PHY_RandomReq();
#endif

  appState = APP_STATE_SEND;
  SCCR0 &= ~(1<<SCCKSEL);
}

/*****************************************************************************
*****************************************************************************/
static void APP_TaskHandler(void)
{
  switch (appState)
  {
    case APP_STATE_INITIAL:
    {
      appInit();
    } break;

    case APP_STATE_SEND:
    {
#if APP_ENDDEVICE
      ledToggle(0);
#endif
      appSendData();
    } break;

    case APP_STATE_SENDING_DONE:
    {
#if APP_ENDDEVICE
      appState = APP_STATE_PREPARE_TO_SLEEP;
#else
      SYS_TimerStart(&appDataSendingTimer);
      appState = APP_STATE_WAIT_SEND_TIMER;
#endif
    } break;

    case APP_STATE_PREPARE_TO_SLEEP:
    {
      if (!NWK_Busy())
      {
        NWK_SleepReq();
        appState = APP_STATE_SLEEP;
      }
    } break;

    case APP_STATE_SLEEP:
    {
      ledsClose();

      PHY_SetRxState(false);

      //HAL_Sleep(APP_SENDING_INTERVAL);
      _delay_ms(500);
      appState = APP_STATE_WAKEUP;
    } break;

    case APP_STATE_WAKEUP:
    {
      NWK_WakeupReq();

      ledsInit();
      ledOn(LED_NETWORK);

      PHY_SetRxState(true);

      appState = APP_STATE_SEND;
    } break;

    default:
      break;
  }
}

/*****************************************************************************
*****************************************************************************/
int main(void)
{
  CLKPR = (1<<CLKPCE);
  CLKPR = (1<<CLKPS0);
  SYS_Init();
  HAL_UartInit(38400);
#ifdef APP_ENABLE_OTA
  OTA_ClientInit();
#endif

  while (1)
  {
    SYS_TaskHandler();
    HAL_UartTaskHandler();
#ifdef APP_ENABLE_OTA
    OTA_ClientTaskHandler();
#endif
    APP_TaskHandler();
  }
}
