#ifndef SENSORFUNCS_H
#define SENSORFUNCS_H
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include "utilities.h"
#include "SSD1306Wire.h"
#include "boards.h"

#include <OneWire.h>
#include <DallasTemperature.h>

extern DallasTemperature sensors_0;
extern DallasTemperature sensors_1;

extern String temperature;
extern String humidity;
extern String pressure;

#include <Adafruit_VEML7700.h>
#include "Adafruit_BME680.h"
#include <Adafruit_LSM6DS3TRC.h>

#define MONVUSB
#ifdef MONVUSB
#define ADC_PIN 35
float getVusb();
#endif

#define USE1WIRE

extern Adafruit_VEML7700 veml;
extern bool veml7700fnd;

void configVeml7700();
void printVeml7700Infos();

extern Adafruit_BME680 bme;
extern bool bme680fnd;

void configBme680();
void printBme680Infos();

extern Adafruit_LSM6DS3TRC lsm6ds3trc;
extern bool lsm6dsfnd;

void configLsm6ds();
void printLsm6dsInfos();

void do_readSensors(void * parameters);

void displayConfig();
#endif
