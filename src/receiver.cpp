#include <FastLED.h>
#include <WiFi.h>
#include <esp_now.h>
#include "pb_decode.h"
#include "./generated/LedColorMessage.pb.h"

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

int global_startingIndex = 0;
int global_currentLedIndex = 0;
bool ledColors_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
   Serial.println("Received led color");
    SingleLEDColor ledColor;

    if (!pb_decode(stream, SingleLEDColor_fields, &ledColor)) {
        return false; // Failed to decode SingleLEDColor.
    }

    // Use `ledColor` (and potentially the arg) to store the decoded value.
    // Assuming `leds` is your array of LEDs:
    leds[global_startingIndex + global_currentLedIndex] = CRGB(ledColor.red, ledColor.green, ledColor.blue);
    global_currentLedIndex++;

    return true;
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    Serial.println("Received data");
    pb_istream_t stream = pb_istream_from_buffer(incomingData, len);
    LedColorMessage message = LedColorMessage_init_zero;

    message.ledColors.funcs.decode = &ledColors_callback;
    global_startingIndex = message.index;
    global_currentLedIndex = 0;

    if (pb_decode(&stream, LedColorMessage_fields, &message)) {
        FastLED.show();
    } else {
        Serial.println("Failed to decode message");
    }
}
void setupESPNow() {
  Serial.println("Setting up ESPNow");
  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHANNEL;
  peerInfo.encrypt = false;

  if (uint8_t err = esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    backgroundColor = CRGB::Red;
    return;
  }
  Serial.println("Finished setting up ESPNow");
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
  Serial.begin(115200);
  Serial.println("Setting up");
  delay(1000);
  setupESPNow();
  pinMode(DATA_PIN, OUTPUT);
  xTaskCreatePinnedToCore(
      moveFocusedLed,   /* Task function. */
      "moveFocusedLed", /* name of task. */
      1000,             /* Stack size of task */
      NULL,             /* parameter of the task */
      0,                /* priority of the task */
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
