//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

//Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_BME680.h"
#include <Adafruit_VEML7700.h>
#include <Adafruit_LSM6DS33.h>

#include <OneWire.h>
#include <DallasTemperature.h>

Adafruit_VEML7700 veml = Adafruit_VEML7700();

Adafruit_BME680 bme; // I2C

Adafruit_LSM6DS33 lsm6ds33;

//log mode over lora
#define LORALOG 0
//define the pins used by the LoRa transceiver module
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

//define LEDS
#define LED0 12
#define LED1 14

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//OLED pins
#define OLED_SDA 21
#define OLED_SCL 22 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//i2c pins
#define I2C_SDA 13
#define I2C_SCL 15
//packet counter
int counter = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

OneWire oneWireBus(2);
// Pass OneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWireBus);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

  pinMode(19, OUTPUT);
  digitalWrite(19, HIGH);
  //reset OLED display via software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //leds
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("trying to disable internal pullups");
  digitalWrite(I2C_SDA, LOW);
  digitalWrite(I2C_SCL, LOW);

  Serial.println("VEML7700 demo");

  if (veml.begin()) {
    Serial.println("Found a VEML7700 sensor");
  } else {
    Serial.println("VEML770 No sensor found ... check your wiring?");
    //while (1);
  }
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    //while (1);
  }
    // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms


  Serial.println("Adafruit LSM6DS33 test!");

  if (!lsm6ds33.begin_I2C(0x6a)) {
    // if (!lsm6ds33.begin_SPI(LSM_CS)) {
    // if (!lsm6ds33.begin_SPI(LSM_CS, LSM_SCK, LSM_MISO, LSM_MOSI)) {
    Serial.println("Failed to find LSM6DS33 chip");
  }

  Serial.println("LSM6DS33 Found!");

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA SENDER ");
  display.display();
  
  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);


}

void loop() {

  bme.begin(0x76);
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LORA SENDER");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("LoRa packet sent.");
  display.setCursor(0,30);
  display.print("Counter:");
  display.setCursor(50,30);
  display.print(counter);      
  display.display();

  counter++;

  display.setCursor(0,40);
  if(LORALOG==1){
  if(digitalRead(LED0) == 0){
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, LOW);
  display.print("GPIO12/14: HIGH");  
  }
  else{
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, HIGH);
  display.print("GPIO12/14: LOW");
  }}
  else{
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
  }
  display.setCursor(0,50);
  display.print("Lux "); display.println(veml.readLux());
  Serial.print("Lux:");
  Serial.println(veml.readLux());
  display.display();

  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
  Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");
  sensors.requestTemperatures();
  Serial.println("Temperature is: " + String(sensors.getTempCByIndex(0)) + "°C");

  //Send LoRa packet to receiver
  if(LORALOG == 1){
  LoRa.beginPacket();
  LoRa.print("TEST_device;");
  LoRa.print("Temp:");
  LoRa.print(String(sensors.getTempCByIndex(0)));
  LoRa.print(";Lux:");
  LoRa.print(String(veml.readLux()));
  LoRa.endPacket();
  Serial.println("send LORA packet");
  }

  delay(1000);
}