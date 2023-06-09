#include "loraFuncs.h"

void configLora(){
  Serial.println("configure LoRa");
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(LoRa_frequency)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }
}

void loraLog(){
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