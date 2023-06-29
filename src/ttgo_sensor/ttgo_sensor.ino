// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// Optionally include custom images
#include "boards.h"


SSD1306Wire display(0x3c, I2C_SDA, I2C_SCL);

void setup()
{
    initBoard();

    // Initialising the UI will init the display too.
    display.init();

    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
}

void loop()
{
    // clear the display
    display.clear();
    display.drawString(0, 10, "Hallo Welt!!!");
    display.display();
    delay(10);
}
