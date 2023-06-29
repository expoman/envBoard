// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// Optionally include custom images
#include "boards.h"

//Lora
#include <LoRa.h>

#include <OneWire.h>
#include <DallasTemperature.h>

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
DallasTemperature sensor_0(&oneWireBus_0);

#endif

//user leds
#define LED0 12
#define LED1 14

//define i2c pins
#define I2CSDA_1 13
#define I2CSCL_1 15



void setup()
{
  initBoard();

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();
  display.drawString(0, 10, "TTGO Lora32 v2 setup");
  display.drawString(0, 20, "setup i2c");
  display.display();
  //setup i2c
  Wire.begin(I2CSDA_1, I2CSCL_1);
  Serial.println("trying to disable internal pullups");
  digitalWrite(I2CSDA_1, LOW);
  digitalWrite(I2CSCL_1, LOW);
  display.drawString(0, 20, "setup i2c done");
  display.drawString(0, 30, "setup LoRa");
  display.display();

  //Serial.println("Small LoRa Sender");
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(LoRa_frequency)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }
  display.drawString(0, 30, "setup LoRa done");
  display.display();

}

void loop()
{
    // clear the display
    display.clear();
    display.drawString(0, 10, "TTGO Lora32 v2");
    #ifdef MONVUSB
    char buf0[32];
    sprintf(buf0, "vbat: %.2f V",getVusb());
    display.drawString(0,20, buf0);
    #endif
    display.display();
    delay(500);
}
