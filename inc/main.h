#pragma once

#include <stdint.h>

void setup_hardware();
void pThreadExampleSetup();
void start_blink_task();
uint32_t get_clock_speed();
void start_debug_print_task();
void setup_wired_ethernet();

void led_on();
void led_off();

