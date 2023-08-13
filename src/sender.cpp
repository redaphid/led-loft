#include <WiFi.h>
#include <esp_now.h>
#include <FastLED.h>
#include "./generated/LedColorMessage.pb.h"
#include "pb_encode.h"

#define NUM_LEDS 20
#define SIZE 255
CRGB leds[NUM_LEDS]; // Define the array of leds

// ESPNow configuration
#define CHANNEL 0
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast address

void setupESPNow()
{
  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHANNEL;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Setting up emitter");
  setupESPNow();
}

bool encode_led_colors(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    for (int currentLED = 0; currentLED < NUM_LEDS; currentLED++) {
        SingleLEDColor ledColor = {
            .red = leds[currentLED].r,
            .green = leds[currentLED].g,
            .blue = leds[currentLED].b
        };

        if (!pb_encode_tag_for_field(stream, field) ||
            !pb_encode_submessage(stream, SingleLEDColor_fields, &ledColor)) {
            return false;
        }
    }
    return true;
}

void loop() {
    // Create a rainbow pattern
    Serial.println("Sending rainbow pattern");
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(i * (255 / NUM_LEDS), 255, 255);
    }

    LedColorMessage message = {};
    message.index = 0; // Starting index
    message.ledColors.funcs.encode = &encode_led_colors; // Set the callback for encoding

    uint8_t buffer[SIZE]; // SIZE should be big enough to hold the encoded message
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, SIZE);

    if (pb_encode(&stream, LedColorMessage_fields, &message)) {
        // Now send the buffer over ESP-Now
        Serial.println("Sending message");
        Serial.println(stream.bytes_written);
        esp_now_send(broadcastAddress, buffer, stream.bytes_written);
    } else {
        // Handle encoding error
        Serial.println("Failed to encode message");
    }
}
