#include <WiFi.h>
#include <esp_now.h>

struct Payload {
  float temp;
  float humi;
};

void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(Payload)) {
    Payload p;
    memcpy(&p, data, sizeof(p));
    Serial.printf("From %02X:%02X:%02X:%02X:%02X:%02X | Temp=%.1f °C, Humi=%.1f %%\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                  p.temp, p.humi);
  } else {
    Serial.printf("Unknown packet len=%d\n", len);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {}