/**
 * \file leds.h
 *
 * \brief LEDs interface
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
 * $Id: leds.h 5223 2012-09-10 16:47:17Z ataradov $
 *
 */

#ifndef _LEDS_H_
#define _LEDS_H_

#include "hal.h"
#include "halGpio.h"

#if defined(PLATFORM_RCB128RFA1) || defined(PLATFORM_RCB231)
  HAL_GPIO_PIN(LED0, E, 2);
  HAL_GPIO_PIN(LED1, E, 3);
  HAL_GPIO_PIN(LED2, E, 4);

#elif defined(PLATFORM_ZIGBIT)
  HAL_GPIO_PIN(LED0, B, 5);
  HAL_GPIO_PIN(LED1, B, 6);
  HAL_GPIO_PIN(LED2, B, 7);

#elif defined(PLATFORM_XPLAINED)
  HAL_GPIO_PIN(LED0, B, 4);
  HAL_GPIO_PIN(LED1, B, 5);
  HAL_GPIO_PIN(LED2, B, 6);

#else
  #error Unknown platform
#endif

INLINE void ledsInit(void)
{
  HAL_GPIO_LED0_out();
  HAL_GPIO_LED0_set();

  HAL_GPIO_LED1_out();
  HAL_GPIO_LED1_set();

  HAL_GPIO_LED2_out();
  HAL_GPIO_LED2_set();
}

INLINE void ledsClose(void)
{
  HAL_GPIO_LED0_in();
  HAL_GPIO_LED1_in();
  HAL_GPIO_LED2_in();
}

INLINE void ledOn(uint8_t i)
{
#ifdef __INVERT_ON_OFF
  if (0 == i)
    HAL_GPIO_LED0_set();
  else if (1 == i)
    HAL_GPIO_LED1_set();
  else if (2 == i)
    HAL_GPIO_LED2_set();
#else
  if (0 == i)
    HAL_GPIO_LED0_clr();
  else if (1 == i)
    HAL_GPIO_LED1_clr();
  else if (2 == i)
    HAL_GPIO_LED2_clr();
#endif
}

INLINE void ledOff(uint8_t i)
{
#ifdef __INVERT_ON_OFF
  if (0 == i)
    HAL_GPIO_LED0_clr();
  else if (1 == i)
    HAL_GPIO_LED1_clr();
  else if (2 == i)
    HAL_GPIO_LED2_clr();
#else
  if (0 == i)
    HAL_GPIO_LED0_set();
  else if (1 == i)
    HAL_GPIO_LED1_set();
  else if (2 == i)
    HAL_GPIO_LED2_set();
#endif
}

INLINE void ledToggle(uint8_t i)
{
  if (0 == i)
    HAL_GPIO_LED0_toggle();
  else if (1 == i)
    HAL_GPIO_LED1_toggle();
  else if (2 == i)
    HAL_GPIO_LED2_toggle();
}

#endif // _LEDS_H_
