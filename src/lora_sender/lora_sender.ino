#include <LoRa.h>
#include "boards.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_VEML7700.h>
#include "Adafruit_BME680.h"
#include <Adafruit_LSM6DS3TRC.h>

RTC_DATA_ATTR int counter = 0;

//ambient light sensor
Adafruit_VEML7700 veml = Adafruit_VEML7700();

Adafruit_LSM6DS3TRC lsm6ds3trc;
Adafruit_BME680 bme; // I2C

// GPIO where the DS18B20 is connected to
const int oneWireBus_0_pin = 2; 

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWireBus_0(oneWireBus_0_pin);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors_0(&oneWireBus_0);

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120        /* Time ESP32 will go to sleep (in seconds) */
//#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

//define LEDS
#define LED0 12
#define LED1 14

//i2c pins
#define I2C_SDA 13
#define I2C_SCL 15

//log mode over lora
#define LORALOG true

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
    // When the power is turned on, a delay is required.
    //delay(1500);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("trying to disable internal pullups");
  digitalWrite(I2C_SDA, LOW);
  digitalWrite(I2C_SCL, LOW);
    //Serial.println("Small LoRa Sender");
    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DI0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        //while (1);
    }

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  float vbat = analogRead(ADC_PIN)*3.3/4096*2;

  //leds
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);

  Serial.println("VEML7700 demo");

  if (veml.begin()) {
    Serial.println("Found a VEML7700 sensor");
  } else {
    Serial.println("No sensor found ... check your wiring?");
    //while (1);
  }
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);
  Serial.print("Lux:");
  Serial.println(veml.readLux());

    sensors_0.requestTemperatures(); 
    float temperatureC = sensors_0.getTempCByIndex(0);


    Serial.print(temperatureC);
    Serial.println("ºC");
    Serial.print("VBAT: ");
    Serial.println(vbat);


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

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    //return;
  }

  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Temperature = "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Humidity = "); Serial.print(bme.humidity); Serial.println(" %");
  Serial.print("Gas = "); Serial.print(bme.gas_resistance / 1000.0); Serial.println(" KOhms");

 Serial.println("Adafruit LSM6DS33 test!");
  if (!lsm6ds3trc.begin_I2C(0x6a)) {
    Serial.println("Failed to find LSM6DS3TR-C chip");
    while (1) {
      delay(10);
    }
  }

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
  Serial.println(" m/s^2 ");

    if(LORALOG){
      Serial.print("Sending packet: ");
      Serial.println(counter);
      // send packet
      LoRa.beginPacket();
      LoRa.print("{");
      LoRa.print("\"room\": \"test_device\", ");
      LoRa.print("\"Lux\": ");
      LoRa.print(veml.readLux());
      LoRa.print(", \"OneWireTemp_0\": ");
      LoRa.print(sensors_0.getTempCByIndex(0));
      LoRa.print(", \"VBAT\": ");
      LoRa.print(vbat);
      LoRa.print(", \"Hum\": ");
      LoRa.print(bme.humidity);
      LoRa.print(", \"BmeTemp\": ");
      LoRa.print(bme.temperature);
      LoRa.print(", \"Pressure\": ");
      LoRa.print(bme.pressure / 100.0);
      LoRa.print(", \"Gas\": ");
      LoRa.print(bme.gas_resistance/1000.0);
      LoRa.print(", \"Accel_X\": ");
      LoRa.print(accel.acceleration.x);
      LoRa.print(", \"Accel_Y\": ");
      LoRa.print(accel.acceleration.y);
      LoRa.print(", \"Accel_Z\": ");
      LoRa.print(accel.acceleration.z);
      LoRa.print(", \"Accel_Temp\": ");
      LoRa.print(temp.temperature);
      LoRa.print("}");
      LoRa.endPacket();
    }

#ifdef HAS_DISPLAY
    if (u8g2) {
        char buf[256];
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Transmitting: OK!");
        snprintf(buf, sizeof(buf), "Sending: %d", counter);
        u8g2->drawStr(0, 30, buf);
        snprintf(buf, sizeof(buf), "Temp: %.2fC", temperatureC);
        u8g2->drawStr(0, 48, buf);
        u8g2->sendBuffer();
    }
#endif
    //counter++;
    esp_deep_sleep_start();
}

void loop()
{
}
