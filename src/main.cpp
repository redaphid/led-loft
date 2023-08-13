#include <FastLED.h>
#include <WiFi.h>
#include <esp_now.h>
// How many leds in your strip?
#define NUM_LEDS 1000
CRGB leds[NUM_LEDS]; //  Define the array of leds
int focusedLED = 0;
#define DATA_PIN 26 // 26 == 7 on breadboard
CRGB backgroundColor = CRGB::DarkBlue;
TaskHandle_t RefreshLedsTask;

// ESPNow configuration
#define CHANNEL 0
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast address

struct Message {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  // Handle incoming LED color data
  Message *incomingData = (Message *)data;
  backgroundColor = CRGB(incomingData->r, incomingData->g, incomingData->b);
}

void setupESPNow() {
  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

CRGB getPixelFromColorPalette(int i)
{
  auto index = i % NUM_LEDS;
  // return a pixel from a soft lavender to a deep purple depending on the index
  CRGB baseColor = backgroundColor;
  // move the baseColor ahead based on the tick
  baseColor = blend(baseColor, CHSV(200, 255, 255), 255 * index / NUM_LEDS);
  return baseColor;
}

void moveFocusedLed(void *parameter)
{
  for (;;)
  {
    focusedLED = (focusedLED + 1) % NUM_LEDS;
    delay(10);
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
    if (i > focusedLED - 5 && i < focusedLED + 5)
    {
      leds[i] = CRGB::White;
      continue;
    }
    leds[i] = getPixelFromColorPalette(i);
  }
  FastLED.show();
}
