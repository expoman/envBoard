// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// Optionally include custom images
#include "boards.h"

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//Lora
#include <LoRa.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "sensorFuncs.h"
#include "loraFuncs.h"

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

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss =  RADIO_CS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst =  RADIO_RST_PIN,
    .dio = {RADIO_DIO0_PIN, RADIO_DIO1_PIN, RADIO_BUSY_PIN}
};

//set the delay at the end of the loop
#define UPLOOPTIME 60000

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = "Hello, Maurice!";
static osjob_t sendjob;
static osjob_t readSensor;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;
const unsigned SENS_INTERVAL = 20;

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        case EV_TXCANCELED:
            Serial.println(F("EV_TXCANCELED"));
            break;
        case EV_RXSTART:
            /* do not print anything -- it wrecks timing */
            break;
        case EV_JOIN_TXCOMPLETE:
            Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
            break;

        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void do_read(osjob_t* j){
    Serial.println("Start loop...");
    #ifdef HAS_DISPLAY
    // clear the display
    display.clear();
    display.drawString(0, 10, "TTGO Lora32 v2");
    #ifdef MONVUSB
    char buf0[32];
    char buf1[32];
    sprintf(buf0, "vbat: %.2f V",getVusb());
    display.drawString(0,20, buf0);
    Serial.println(buf0);
    #endif
    if(bme680fnd){
      //bme.performReading();
      sprintf(buf0, "Hum:%.1f, T:%.1f",bme.humidity, bme.temperature);
      sprintf(buf1, "P:%.1f, G:%.1f", bme.pressure/100.0, bme.gas_resistance/1000.0);
      display.drawString(0, 30, buf0);
      display.drawString(0, 40, buf1);
    }
    if(veml7700fnd){
      sprintf(buf0, "Lux: %.2f", veml.readLux());
      display.drawString(0,50, buf0);
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
  sprintf(buf0, "State GPIO pin(%d): %d", LED1, digitalRead(LED1));
  Serial.println(buf1);
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
    loraLog();
  }
  // Schedule sensors reading cycle
  os_setTimedCallback(&readSensor, os_getTime()+sec2osticks(SENS_INTERVAL), do_read);
}

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
  //setupLMIC();
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
  do_read(&readSensor);
}

void loop()
{
  os_runloop_once();
}
