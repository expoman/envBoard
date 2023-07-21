#ifndef LORAFUNCS_H
#define LORAFUNCS_H
#include <Arduino.h>
#include "utilities.h"
#include <LoRa.h>
#include "sensorFuncs.h"

extern String deviceName;

extern osjob_t loraSendJob;
void do_lorasend(osjob_t* j);

void configLora();
void loraLog();

#endif
