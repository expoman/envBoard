// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier

#include <lmic.h>
#include "Arduino.h"
#include <SPI.h>

//Lora
#include <LoRa.h>

#include "main.h"

#include "sensorFuncs.h"
#include "loraFuncs.h"

#include "ttn_funcs.h"

#include <Preferences.h>

#include "webserver.h"

//define i2c pins
#define I2CSDA_1 13
#define I2CSCL_1 15

Preferences prefs;

//deep sleep state
bool useDeepSleep = false;
RTC_DATA_ATTR int counter = 0;
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
//#define TIME_TO_SLEEP  120        /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

String getDevName(){
   return prefs.getString("devName", "devBoard");
}

String deviceName = "";
uint8_t logMethod = 0;
uint8_t logLoraInterval = 0;

void setup()
{
  initBoard();

  prefs.begin("envSensor", false);

  Serial.println(getDevName().c_str());
  deviceName = getDevName();
  logMethod = prefs.getUInt("logMethod", 0);
  logLoraInterval = prefs.getUInt("logLoraInterval", 120);
  Serial.print("logInterval: ");
  Serial.println(logLoraInterval);

  if(useDeepSleep){
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  }

  #ifdef HAS_DISPLAY
  // Initialising the UI will init the display too.
  displayConfig();
  #endif

  //setup i2c
  Serial.println("configure i2c_1");
  Wire.begin(I2CSDA_1, I2CSCL_1);
  Serial.println("trying to disable internal pullups");
  digitalWrite(I2CSDA_1, LOW);
  digitalWrite(I2CSCL_1, LOW);

  configLora();

  //Configure user leds
  Serial.println("configure use leds");
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);


  //configure sensors
  configVeml7700();
  configBme680();
  configLsm6ds();

  //enter deep sleep state
  if(useDeepSleep){
    esp_deep_sleep_start();
  }
  Serial.println("LoRa Receiver");
  // Reset the MAC state. Session and pending data transfers will be discarded.
  //LMIC_reset();

  //configure webserver
  configWebserver();

  // Start job (sending automatically starts OTAA too)
  //xTaskCreate(do_send, "LoraTTNTask", 1000, NULL, 1, NULL);
  // Start Lora job (sending automatically starts OTAA too)
  xTaskCreate(do_lorasend, "LoraTask", 2000, NULL, 1, NULL);
  // Start job reading sensor data
  xTaskCreate(do_readSensors, "sensTask", 2000, NULL, 1, NULL);
}

void loop()
{
vTaskDelay(10 / portTICK_PERIOD_MS);
  //Serial.println("start");
  //LoRa.beginPacket();
  //LoRa.print("\"room\": \"");
  //LoRa.print(deviceName.c_str());
  //LoRa.endPacket();
  //Serial.println("end");
  //delay(5000);
}
