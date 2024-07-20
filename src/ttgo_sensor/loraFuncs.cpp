#include "loraFuncs.h"

void do_lorasend(void * parameters){
for(;;){
  if(logMethod == 1){
     if(sensorData != 0){
        Serial.println("start loraLog");
        loraLog();
        Serial.println("end loraLog");
     }
  }
  vTaskDelay(logLoraInterval * 1000/ portTICK_PERIOD_MS);
}
}

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
    LoRa.print(DATAROOM);
    LoRa.print(":");
    LoRa.print(deviceName.c_str());
    LoRa.print(",");
    if(veml7700fnd){
    LoRa.print(DATALUX);
    LoRa.print(":");
    LoRa.print(veml.readLux());
    }
    float owTemp0 = sensors_0.getTempCByIndex(0); 
    if(owTemp0 != 127){
       LoRa.print(",");
       LoRa.print(DATAONEWIRE0);
       LoRa.print(":");
       LoRa.print(owTemp0);
    }
    #ifdef MONVUSB
    LoRa.print(",");
    LoRa.print(DATAVBAT);
    LoRa.print(":");
    LoRa.print(getVusb());
    #endif
    if(bme680fnd){
    LoRa.print(",");
    LoRa.print(DATABMEHUM);
    LoRa.print(":");
    LoRa.print(bme.humidity);
    LoRa.print(",");
    LoRa.print(DATABMETEMP);
    LoRa.print(":");
    LoRa.print(bme.temperature);
    LoRa.print(",");
    LoRa.print(DATABMEPRESSURE);
    LoRa.print(":");
    LoRa.print(bme.pressure / 100.0);
    LoRa.print(",");
    LoRa.print(DATABMEGAS);
    LoRa.print(":");
    LoRa.print(bme.gas_resistance/1000.0);
    }

    if(veml7700fnd){
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
    LoRa.print(",");
    LoRa.print(DATAACCELX);
    LoRa.print(":");
    LoRa.print(accel.acceleration.x);
    LoRa.print(",");
    LoRa.print(DATAACCELY);
    LoRa.print(":");
    LoRa.print(accel.acceleration.y);
    LoRa.print(",");
    LoRa.print(DATAACCELZ);
    LoRa.print(":");
    LoRa.print(accel.acceleration.z);
    LoRa.print(",");
    LoRa.print(DATAACCELTEMP);
    LoRa.print(":");
    LoRa.print(temp.temperature);
    }
    LoRa.print("}");
    uint8_t resLora = LoRa.endPacket();
    Serial.print("result lora");
    Serial.println(resLora);
    Serial.println("end");
    delay(500);
}
