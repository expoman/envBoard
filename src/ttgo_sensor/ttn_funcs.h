#ifndef TTN_FUNCS_H
#define TTN_FUNCS_H
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include "utilities.h"
#include <Arduino.h>

extern const lmic_pinmap lmic_pins;

extern osjob_t sendjob;

void onEvent (ev_t ev);
void do_send(osjob_t* j);

#endif
