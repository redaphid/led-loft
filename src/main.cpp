#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 1000
#define DATA_PIN 26
CRGB backgroundColor = CRGB::DarkBlue;
// 26 == 7 on breadboard
//  Define the array of leds
CRGB leds[NUM_LEDS];
int focusedLED = 0;
TaskHandle_t RefreshLedsTask;

CRGB getPixelFromColorPalette(int i)
{
  auto index = i % NUM_LEDS;
  // return a pixel from a soft lavender to a deep purple depending on the index
  CRGB baseColor = CHSV(200, 255, 255);
  // move the baseColor ahead based on the tick
  baseColor = blend(baseColor, CHSV(200, 255, 255), 255 * index / NUM_LEDS);
  return baseColor;
}

void moveFocusedLed(void *parameter)
{
  for (;;)
  {
    focusedLED = (focusedLED + 1) % NUM_LEDS;
    delay(100);
  }
}

void setup()
{

  pinMode(DATA_PIN, OUTPUT);
  xTaskCreatePinnedToCore(
      moveFocusedLed,   /* Task function. */
      "moveFocusedLed", /* name of task. */
      1000,             /* Stack size of task */
      NULL,             /* parameter of the task */
      1,                /* priority of the task */
      &RefreshLedsTask, /* Task handle to keep track of created task */
      1);               /* Core */

  FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUM_LEDS); // GRB ordering is assume
  // set all of the leds a gradient from blue to red
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = backgroundColor;
  }
  FastLED.show();
}
void loop()
{
  // for each led, fade it to the color of the led before it
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i == focusedLED)
    {
      continue;
    }
    leds[i] = getPixelFromColorPalette(i);
    delay(10);
  }
}
