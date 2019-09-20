/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>
#include "Display.h"

Display tft = Display();

uint8_t y = 0;

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);

  tft.fillScreen(ILI9341_BLACK);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  tft.hLine(y, ILI9341_CYAN);

  delay(20);

  tft.hLine(y, ILI9341_BLACK);
  y = (y + 1) % 144;
}
