#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 500
#define DATA_PIN 26
//26 == 7 on breadboard
// Define the array of leds
CRGB leds[NUM_LEDS];

// print to serial from the other core
void printToSerial(String message) {
  Serial.println(message);
}

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  FastLED.addLeds<WS2811, DATA_PIN,RGB>(leds, NUM_LEDS); // GRB ordering is assume
  // set all of the leds to an initial purple/blue fade, with leds closer to 0 being more purple
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(255 - (i * 255 / NUM_LEDS), 255, 255);
  }
  FastLED.show();
}
void loop() {
  // for each led, fade it to the color of the led before it
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = blend(leds[i], leds[(i + 1) % NUM_LEDS], 128);
    //if the led is too dim, set it to a random color
    if (leds[i].r < 10 && leds[i].g < 10 && leds[i].b < 10) {
      leds[i] = CHSV(random(255), 255, 255);
    }
  }
  FastLED.show();
}
