// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Copyright (C) May 2022, Belmont Computing, Inc. -- All Rights Reserved
// Licensed under the BCI License. See LICENSE for details.

#include "apps/demos.h"

#include <stdbool.h>

#include "dif/device.h"
#include "dif/dif_gpio.h"
#include "dif/dif_uart.h"
#include "dif/hart.h"
#include "dif/log.h"
#include "dif/check.h"

void demo_gpio_startup(dif_gpio_t *gpio) {
  LOG_INFO("Watch the LEDs!");

  // Give a LED pattern as startup indicator for 5 seconds.
  CHECK(dif_gpio_write_all(gpio, 0xff00) == kDifGpioOk);
  for (int i = 0; i < 32; ++i) {
    usleep(5 * 1000);  // 5 ms
    CHECK(dif_gpio_write(gpio, 8 + (i % 8), (i / 8) % 2) == kDifGpioOk);
  }
  CHECK(dif_gpio_write_all(gpio, 0x0000) == kDifGpioOk);  // All LEDs off.
}

/**
 * Mask for "valid" GPIO bits. The first byte represents switch inputs,
 * while byte 16 represents the FTDI bit.
 */
static const uint32_t kGpioMask = 0x100ff;

/**
 * Mask for the FTDI bit among the GPIO bits.
 */
static const uint32_t kFtdiMask = 0x10000;

uint32_t demo_gpio_to_log_echo(dif_gpio_t *gpio, uint32_t prev_gpio_state) {
  uint32_t gpio_state;
  CHECK(dif_gpio_read_all(gpio, &gpio_state) == kDifGpioOk);
  gpio_state &= kGpioMask;

  uint32_t state_delta = prev_gpio_state ^ gpio_state;
  for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
    bool changed = ((state_delta >> bit_idx) & 0x1) != 0;
    bool is_currently_set = ((gpio_state >> bit_idx) & 0x1) != 0;
    if (changed) {
      LOG_INFO("GPIO switch #%d changed to %d", bit_idx, is_currently_set);
    }
  }

  if ((state_delta & kFtdiMask) != 0) {
    if ((gpio_state & kFtdiMask) != 0) {
      LOG_INFO("FTDI control changed. Enable JTAG.");
    } else {
      LOG_INFO("FTDI control changed. Enable JTAG.");
    }
  }

  return gpio_state;
}

uint32_t demo_uart_to_uart_and_gpio_echo(dif_uart_t *uart, dif_gpio_t *gpio) {
  /* BCI MOD: Return 2 if receiving '^C'. */
  uint32_t rv;
  rv = 0;

  while (true) {
    size_t chars_available;
    if (dif_uart_rx_bytes_available(uart, &chars_available) != kDifUartOk ||
        chars_available == 0) {
      break;
    }

    uint8_t rcv_char;
    CHECK(dif_uart_bytes_receive(uart, 1, &rcv_char, NULL) == kDifUartOk);
    CHECK(dif_uart_byte_send_polled(uart, rcv_char) == kDifUartOk);
    CHECK(dif_gpio_write_all(gpio, rcv_char << 8) == kDifGpioOk);

    /* BCI MOD: Look for '^C'. */
    if (rv != 2) {
      if (rcv_char == ((uint8_t) '^')) {
        rv = 1;
      } else {
        if (rcv_char == ((uint8_t) 'C')) {
          rv = 2;
        } else {
          rv = 0;
        }
      }
    }
  }

  return rv;
}
