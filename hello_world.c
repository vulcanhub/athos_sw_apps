// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Copyright (C) May 2022, Belmont Computing, Inc. -- All Rights Reserved
// Licensed under the BCI License. See LICENSE for details.

#include "dif/device.h"
#include "dif/dif_gpio.h"
#include "dif/dif_uart.h"
#include "dif/hart.h"
#include "dif/log.h"
#include "dif/print.h"
#include "dif/check.h"
#include "dif/test_status.h"
#include "dif/test_main.h"
#include "apps/demos.h"
#include "top/sw/autogen/top_athos.h"  // Generated.

const test_config_t kTestConfig;

static dif_gpio_t gpio;
static dif_uart_t uart;

//int main(int argc, char **argv) {
bool test_main(void) {

  // Initialize UART.
  CHECK(
      dif_uart_init(
          (dif_uart_params_t){
              .base_addr = mmio_region_from_addr(TOP_ATHOS_UART0_BASE_ADDR),
          },
          &uart) == kDifUartOk);
  CHECK(dif_uart_configure(&uart, (dif_uart_config_t){
                                      .baudrate = kUartBaudrate,
                                      .clk_freq_hz = kClockFreqPeripheralHz,
                                      .parity_enable = kDifUartToggleDisabled,
                                      .parity = kDifUartParityEven,
                                  }) == kDifUartConfigOk);
  base_uart_stdout(&uart);

  // Initialize GPIO.
  dif_gpio_params_t gpio_params = {
      .base_addr = mmio_region_from_addr(TOP_ATHOS_GPIO_BASE_ADDR),
  };
  CHECK(dif_gpio_init(gpio_params, &gpio) == kDifGpioOk);
  // Enable GPIO: 0-7 and 16 is input; 8-15 is output.
  CHECK(dif_gpio_output_set_enabled_all(&gpio, 0x0ff00) == kDifGpioOk);

  // Add DATE and TIME because I keep fooling myself with old versions
  LOG_INFO("Hello World!");
  LOG_INFO("Built at: " __DATE__ ", " __TIME__);

  demo_gpio_startup(&gpio);

  // Now have UART <-> Buttons/LEDs demo
  // all LEDs off
  CHECK(dif_gpio_write_all(&gpio, 0x0000) == kDifGpioOk);
  LOG_INFO("Try out the switches on the board");
  LOG_INFO("or type anything into the console window.");
  LOG_INFO("The LEDs show the ASCII code of the last character.");

  uint32_t gpio_state = 0;
  uint32_t stop;
  while (true) {
    usleep(10 * 1000);  // 10 ms
    gpio_state = demo_gpio_to_log_echo(&gpio, gpio_state);
    stop = demo_uart_to_uart_and_gpio_echo(&uart, &gpio);
    /* BCI MOD: Stop test when receiving '^C'. */
    if (stop == 2) {
      LOG_INFO("Received ^C. Exiting Hello World test.");
      break;
    }
  }

  return true;
}
