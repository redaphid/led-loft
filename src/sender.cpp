#include <WiFi.h>
#include <esp_now.h>

// ESPNow configuration
#define CHANNEL 0
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast address

struct Message
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

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

void sendColor(uint8_t r, uint8_t g, uint8_t b)
{
  Serial.println("Sending color");
  Serial.println(r);
  Serial.println(g);
  Serial.println(b);
  Message message;
  message.r = r;
  message.g = g;
  message.b = b;

  esp_now_send(broadcastAddress, (uint8_t *)&message, sizeof(Message));
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Setting up emitter");
  setupESPNow();
}

void loop()
{
  Serial.println("Looping");
  Serial.println("Sending red");
  sendColor(255, 0, 0); // Send Red
  delay(2050);
  Serial.println("Sending green");
  sendColor(0, 255, 0); // Send Blue
  delay(2050);
}
