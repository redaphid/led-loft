#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 1000
#define DATA_PIN 26
#define BLINK_PIN 27
#define BACKGROUND_COLOR CRGB::DarkBlue
// 26 == 7 on breadboard
//  Define the array of leds
CRGB leds[NUM_LEDS];
int focusedLED = 0;
int focusedDirection = 1;
TaskHandle_t RefreshLedsTask;

void refreshLEDsTask(void *parameter)
{
  for (;;)
  {
    focusedLED+=focusedDirection;
    // if we hit the end, reverse direction
    if (focusedLED == NUM_LEDS)
    {
      focusedDirection = -1;
      focusedLED = NUM_LEDS - 1;
    }
    // if we hit the beginning, reverse direction
    if (focusedLED == -1)
    {
      focusedDirection = 1;
      focusedLED = 0;
    }

    auto realFocusedColor = leds[focusedLED];
    leds[focusedLED] = CRGB::White;
    FastLED.show();
    leds[focusedLED] = realFocusedColor;
  }
}

void setup()
{

  pinMode(DATA_PIN, OUTPUT);
  pinMode(BLINK_PIN, OUTPUT);
  xTaskCreatePinnedToCore(
      refreshLEDsTask,   /* Task function. */
      "refreshLEDsTask", /* name of task. */
      1000,              /* Stack size of task */
      NULL,              /* parameter of the task */
      1,                 /* priority of the task */
      &RefreshLedsTask,  /* Task handle to keep track of created task */
      1);                /* Core */

  FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUM_LEDS); // GRB ordering is assume
  // set all of the leds a gradient from blue to red
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(map(i, 0, NUM_LEDS, 0, 255), 255, 255);
  }
  // delay(1000);
  // FastLED.show();
}
void loop()
{
  // for each led, fade it to the color of the led before it
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if(i == focusedLED){
      continue;
    }
    // blend 10% with the background color.
    leds[i] = blend(leds[i], BACKGROUND_COLOR, 1);
    // 1% of the time, pick a random color to fade to
    if (random8() == 1)
    {
      leds[i] = CRGB::Red;
    }
    // 5% of the time, tween to the next color
    if (random8() < 15)
    {
      leds[i] = blend(leds[i], leds[i - 1], 10);
    }
    if (random8() < 5)
    {
      leds[i] = blend(leds[i], leds[i + 1], 10);
    }
  }
  // fade the first led to black
  // leds[0].fadeToBlackBy(64);
}
