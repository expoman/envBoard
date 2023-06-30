// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// Optionally include custom images
#include "boards.h"

//Lora
#include <LoRa.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_VEML7700.h>
#include "Adafruit_BME680.h"
#include <Adafruit_LSM6DS3TRC.h>

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

//ambient light sensor
Adafruit_VEML7700 veml = Adafruit_VEML7700();
//veml7700 found
bool veml7700fnd = false;

void configureVeml7700(){
  //configure VEML7700
  Serial.println("configure VEML7700");
  if (veml.begin()) {
    veml7700fnd = true;
    Serial.println("Found a VEML7700 sensor");
    veml.setGain(VEML7700_GAIN_1);
    veml.setIntegrationTime(VEML7700_IT_100MS);

  } else {
    Serial.println("No sensor found ... check your wiring?");
  }
}
void printVeml7700Infos(){
    if(veml7700fnd){
      Serial.print("Lux:");
      Serial.println(veml.readLux());
    }
}

//BME680
Adafruit_BME680 bme; // I2C
//bm3680 found
bool bme680fnd = false;

void configureBme680(){
  Serial.println("configure bme680");
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
  }
  else{
    bme680fnd = true;
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  }
}
void printBme680Infos(){
  if(bme680fnd){
    if (! bme.performReading()) {
      Serial.println("Failed to perform reading :(");
    }
    else{
      Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
      Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
      Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
      Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");
    }
  }
}

void setup()
{
  initBoard();

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
  configureVeml7700();
  configureBme680();
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
  Serial.printf("test %d\n", digitalRead(LED0));
  if(digitalRead(LED0) == 0){
    digitalWrite(LED0, 1);
  }
  else{
    digitalWrite(LED0, 0);
  }
  printVeml7700Infos();
  printBme680Infos();

  delay(500);
}
