#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H
#include <Arduino.h>
#include <WiFi.h>
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

void configWebserver();

extern IPAddress IP;
extern AsyncWebServer server;

extern const char index_html[] PROGMEM;
#endif /* -- MYWEBSERVER_H -- */
