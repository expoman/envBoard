#ifndef LORAFUNCS_H
#define LORAFUNCS_H
#include <Arduino.h>
#include "utilities.h"
#include <LoRa.h>
#include "sensorFuncs.h"
#include "main.h"
#include "dataFormat.h"

void do_lorasend(void * parameters);

void configLora();
void loraLog();

#endif
