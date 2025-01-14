#include <WiFi.h>
#include <esp_now.h>

void onDataRecv(const esp_now_recv_info_t *mac, const uint8_t *incomingData, int len) {

  char receivedMsg[len + 1];
  memcpy(receivedMsg, incomingData, len);
  receivedMsg[len] = '\0';
  Serial.print("Received Message: ");
  Serial.println(receivedMsg);

  // Reply back
  const char *reply = "Hello from Slave";
  esp_now_send(mac->src_addr, (uint8_t *)reply, strlen(reply));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  delay(1000);
}
