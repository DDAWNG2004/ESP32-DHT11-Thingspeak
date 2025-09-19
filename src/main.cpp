#include <WiFi.h>
#include <esp_now.h>

typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message myData;

// Callback khi nhận dữ liệu
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Received: ");
  Serial.println(myData.msg);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}
