// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Copyright (C) May 2022, Belmont Computing, Inc. -- All Rights Reserved
// Licensed under the BCI License. See LICENSE for details.

#include <stdbool.h>
#include <stdint.h>

#include "dif/device.h"
#include "dif/hart.h"
#include "dif/log.h"
#include "dif/print.h"
#include "dif/test_coverage.h"
#include "dif/test_main.h"

static void spin_45(uint8_t state) {
  static const char kSpinnerChars[] = "|/-\\";
  base_printf("%c\r", kSpinnerChars[state]);
}

static void spin_180(void) {
  for (uint8_t state = 0; state < 4; ++state) {
    spin_45(state);
    usleep(100 * 1000);  // 0.1s
  }
}

const test_config_t kTestConfig;

/**
 * A simple program for testing the coverage functionality.
 *
 * Prints a message and displays a spinning bar.
 */
bool test_main(void) {
  // Print an assuring message.
  LOG_INFO("Collecting coverage data.");
  // Display a spinning bar.
  for (uint8_t i = 0; i < 4; ++i) {
    spin_180();
  }
  return true;
}
