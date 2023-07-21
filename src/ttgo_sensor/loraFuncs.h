#ifndef LORAFUNCS_H
#define LORAFUNCS_H
#include <Arduino.h>
#include "utilities.h"
#include <LoRa.h>
#include "sensorFuncs.h"

extern String deviceName;

void configLora();
void loraLog();

#endif
