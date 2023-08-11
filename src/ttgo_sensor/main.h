#ifndef MAIN_H
#define MAIN_H
#include <Preferences.h>
#include "dataFormat.h"

#define LORALOG false

extern Preferences prefs;

extern String deviceName;
extern uint8_t logMethod;
extern uint8_t logLoraInterval;
extern uint8_t sensorData;

#endif /* -- MAIN_H -- */
