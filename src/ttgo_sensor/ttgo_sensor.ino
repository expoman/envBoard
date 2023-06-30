// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// Optionally include custom images
#include "boards.h"

//Lora
#include <LoRa.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "sensorFuncs.h"

SSD1306Wire display(0x3c, I2C_SDA, I2C_SCL);

#define USE1WIRE

#define MONVUSB

#define LORALOG false

#ifdef MONVUSB
#define ADC_PIN 35

float getVusb(){
  return analogRead(ADC_PIN)*3.3/4096*2;
}
#endif

#ifdef USE1WIRE
//GPIO where the DS18b20 sensor is connected to
const int oneWireBus_0_pin = 2;

//Setup oneWire instance
OneWire oneWireBus_0(oneWireBus_0_pin);
//Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors_0(&oneWireBus_0);

#endif

//user leds
#define LED0 14
#define LED1 12

//define i2c pins
#define I2CSDA_1 13
#define I2CSCL_1 15

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

void setup()
{
  initBoard();

  if(useDeepSleep){
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  }

  #ifdef HAS_DISPLAY
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  #endif

  //setup i2c
  Serial.println("configure i2c_1");
  Wire.begin(I2CSDA_1, I2CSCL_1);
  Serial.println("trying to disable internal pullups");
  digitalWrite(I2CSDA_1, LOW);
  digitalWrite(I2CSCL_1, LOW);

  Serial.println("configure LoRa");
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(LoRa_frequency)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }

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
}

void loop()
{
    Serial.println("Start loop...");
    #ifdef HAS_DISPLAY
    // clear the display
    display.clear();
    display.drawString(0, 10, "TTGO Lora32 v2");
    #ifdef MONVUSB
    char buf0[32];
    sprintf(buf0, "vbat: %.2f V",getVusb());
    display.drawString(0,20, buf0);
    Serial.println(buf0);
    #endif
    if(bme680fnd){
      //bme.performReading();
      sprintf(buf0, "Hum:%d, T:%d, P:%d, G:%d", bme.humidity, bme.temperature, bme.pressure, bme.gas_resistance);
      display.drawString(0, 30, buf0);
    }
    if(veml7700fnd){
      sprintf(buf0, "Lux: %d", veml.readLux());
      display.drawString(0,40, buf0);
    }
    display.display();
    #endif

    #ifdef USE1WIRE
    sensors_0.requestTemperatures(); 
    float temperatureC = sensors_0.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    #endif


  //LED0 has problem -> resolder to other pin
  //toggle LED1
  Serial.printf("test %d\n", digitalRead(LED1));
  if(digitalRead(LED1) == 0){
    digitalWrite(LED1, 1);
  }
  else{
    digitalWrite(LED1, 0);
  }
  printVeml7700Infos();
  printBme680Infos();
  printLsm6dsInfos();

  if(LORALOG){
    Serial.print("LoRa sending packet");
    //send packet
    LoRa.beginPacket();
    LoRa.print("{");
    LoRa.print("\"room\": \"test_device\", ");
    if(veml7700fnd){
    LoRa.print("\"Lux\": ");
    LoRa.print(veml.readLux());
    }
    LoRa.print(", \"OneWireTemp_0\": ");
    LoRa.print(sensors_0.getTempCByIndex(0));
    #ifdef MONVUSB
    LoRa.print(", \"VBAT\": ");
    LoRa.print(getVusb());
    #endif
    if(bme680fnd){
    LoRa.print(", \"Hum\": ");
    LoRa.print(bme.humidity);
    LoRa.print(", \"BmeTemp\": ");
    LoRa.print(bme.temperature);
    LoRa.print(", \"Pressure\": ");
    LoRa.print(bme.pressure / 100.0);
    LoRa.print(", \"Gas\": ");
    LoRa.print(bme.gas_resistance/1000.0);
    }

    if(veml7700fnd){
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
    LoRa.print(", \"Accel_X\": ");
    LoRa.print(accel.acceleration.x);
    LoRa.print(", \"Accel_Y\": ");
    LoRa.print(accel.acceleration.y);
    LoRa.print(", \"Accel_Z\": ");
    LoRa.print(accel.acceleration.z);
    LoRa.print(", \"Accel_Temp\": ");
    LoRa.print(temp.temperature);
    LoRa.print("}");
    }
    LoRa.endPacket();
    }
  delay(500);
}