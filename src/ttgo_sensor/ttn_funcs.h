#ifndef TTN_FUNCS_H
#define TTN_FUNCS_H
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include "utilities.h"
#include <Arduino.h>
#include "main.h"

extern const lmic_pinmap lmic_pins;

void onEvent (ev_t ev);
void do_send(void * parameters);

#endif
