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

void configVeml7700(){
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

void configBme680(){
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

//LSM6DS3 sensor
Adafruit_LSM6DS3TRC lsm6ds3trc;
bool lsm6dsfnd = false;

void configLsm6ds(){
  Serial.println("configure LSM6DS33");
  if (!lsm6ds3trc.begin_I2C(0x6a)) {
    Serial.println("Failed to find LSM6DS3TR-C chip");
  }
  else{
    lsm6dsfnd = true;
    Serial.println("LSM6DS33 Found!");

    // lsm6ds3trc.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    Serial.print("Gyro range set to: ");
    switch (lsm6ds3trc.getGyroRange()) {
    case LSM6DS_GYRO_RANGE_125_DPS:
      Serial.println("125 degrees/s");
      break;
    case LSM6DS_GYRO_RANGE_250_DPS:
      Serial.println("250 degrees/s");
      break;
    case LSM6DS_GYRO_RANGE_500_DPS:
      Serial.println("500 degrees/s");
      break;
    case LSM6DS_GYRO_RANGE_1000_DPS:
      Serial.println("1000 degrees/s");
      break;
    case LSM6DS_GYRO_RANGE_2000_DPS:
      Serial.println("2000 degrees/s");
      break;
    case ISM330DHCX_GYRO_RANGE_4000_DPS:
      break; // unsupported range for the DS33
    }
    // lsm6ds3trc.setGyroDataRate(LSM6DS_RATE_12_5_HZ);
    Serial.print("Gyro data rate set to: ");
    switch (lsm6ds3trc.getGyroDataRate()) {
    case LSM6DS_RATE_SHUTDOWN:
      Serial.println("0 Hz");
      break;
    case LSM6DS_RATE_12_5_HZ:
      Serial.println("12.5 Hz");
      break;
    case LSM6DS_RATE_26_HZ:
      Serial.println("26 Hz");
      break;
    case LSM6DS_RATE_52_HZ:
      Serial.println("52 Hz");
      break;
    case LSM6DS_RATE_104_HZ:
      Serial.println("104 Hz");
      break;
    case LSM6DS_RATE_208_HZ:
      Serial.println("208 Hz");
      break;
    case LSM6DS_RATE_416_HZ:
      Serial.println("416 Hz");
      break;
    case LSM6DS_RATE_833_HZ:
      Serial.println("833 Hz");
      break;
    case LSM6DS_RATE_1_66K_HZ:
      Serial.println("1.66 KHz");
      break;
    case LSM6DS_RATE_3_33K_HZ:
      Serial.println("3.33 KHz");
      break;
    case LSM6DS_RATE_6_66K_HZ:
      Serial.println("6.66 KHz");
      break;
    }

    // lsm6ds3trc.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
    Serial.print("Accelerometer data rate set to: ");
    switch (lsm6ds3trc.getAccelDataRate()) {
    case LSM6DS_RATE_SHUTDOWN:
      Serial.println("0 Hz");
      break;
    case LSM6DS_RATE_12_5_HZ:
      Serial.println("12.5 Hz");
      break;
    case LSM6DS_RATE_26_HZ:
      Serial.println("26 Hz");
      break;
    case LSM6DS_RATE_52_HZ:
      Serial.println("52 Hz");
      break;
    case LSM6DS_RATE_104_HZ:
      Serial.println("104 Hz");
      break;
    case LSM6DS_RATE_208_HZ:
      Serial.println("208 Hz");
      break;
    case LSM6DS_RATE_416_HZ:
      Serial.println("416 Hz");
      break;
    case LSM6DS_RATE_833_HZ:
      Serial.println("833 Hz");
      break;
    case LSM6DS_RATE_1_66K_HZ:
      Serial.println("1.66 KHz");
      break;
    case LSM6DS_RATE_3_33K_HZ:
      Serial.println("3.33 KHz");
      break;
    case LSM6DS_RATE_6_66K_HZ:
      Serial.println("6.66 KHz");
      break;
    }

    // lsm6ds3trc.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    Serial.print("Accelerometer range set to: ");
    switch (lsm6ds3trc.getAccelRange()) {
    case LSM6DS_ACCEL_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case LSM6DS_ACCEL_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case LSM6DS_ACCEL_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case LSM6DS_ACCEL_RANGE_16_G:
      Serial.println("+-16G");
      break;
    }
  }
}

void printLsm6dsInfos(){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("\t\tAccel X: ");
  Serial.print(accel.acceleration.x);
  Serial.print(" \tY: ");
  Serial.print(accel.acceleration.y);
  Serial.print(" \tZ: ");
  Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2");
  /* Display the results (gyroscop is measured in rad/s) */
  Serial.print("\t\Gyro X: ");
  Serial.print(gyro.gyro.x);
  Serial.print(" \tY: ");
  Serial.print(gyro.gyro.y);
  Serial.print(" \tZ: ");
  Serial.print(gyro.gyro.z);
  Serial.println(" rad/s");
    /* Display the results (temp is measured in deg C) */
  Serial.print("\t\Temp: ");
  Serial.print(temp.temperature);
  Serial.println(" deg C");
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
  configVeml7700();
  configBme680();
  configLsm6ds();
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