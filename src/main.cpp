#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "DHT.h"

// ==== Cấu hình WiFi ====
const char* ssid     = "POCO X4 GT";      // Đổi thành SSID WiFi
const char* password = "hjkhjkhjk";    // Đổi thành mật khẩu WiFi

// ==== Cấu hình ThingSpeak ====
String apiKey = "L8LZ6SICYHTURTBQ";           // 🔑 API Key từ ThingSpeak
const char* server = "api.thingspeak.com";

// ==== Cảm biến DHT11 ====
#define DHTPIN 4       // Chân GPIO nối với DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ==== Hàm gửi dữ liệu lên ThingSpeak với retry ====
bool sendToThingSpeak(float tempC, float hum, float heatIdx, int maxRetries = 3) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ No WiFi. Skipping ThingSpeak send.");
    return false;
  }

  // Kiểm tra DNS trước
  IPAddress ip;
  bool okResolve = WiFi.hostByName(server, ip);
  if (okResolve) {
    Serial.print("Resolved api.thingspeak.com -> ");
    Serial.println(ip);
  } else {
    Serial.println("⚠️ DNS lookup failed, vẫn thử gửi bằng hostname.");
  }

  String urlBase = String("http://") + server + "/update?api_key=" + apiKey;
  String payload = String("&field1=") + String(tempC, 2) +
                   "&field2=" + String(hum, 2) +
                   "&field3=" + String(heatIdx, 2);

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    Serial.printf("➡️ Gửi lên ThingSpeak (attempt %d/%d)\n", attempt, maxRetries);

    HTTPClient http;
    http.begin(urlBase + payload);
    http.setConnectTimeout(5000); // 5 giây timeout
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("ThingSpeak HTTP code: %d\n", httpCode);
      String resp = http.getString();
      Serial.print("Body: ");
      Serial.println(resp);
      http.end();

      if (httpCode == 200) {
        Serial.println("✅ Sent to ThingSpeak successfully.");
        return true;
      }
    } else {
      Serial.printf("❌ HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
      http.end();
    }

    delay(1000 * attempt); // đợi trước khi retry
  }

  Serial.println("❌ All attempts to send to ThingSpeak failed.");
  return false;
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  Serial.printf("=== ESP32 DHT11 -> ThingSpeak ===\nConnecting to WiFi SSID: %s\n", ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ==== Loop ====
void loop() {
  float hum = dht.readHumidity();
  float tempC = dht.readTemperature();
  float heatIdx = dht.computeHeatIndex(tempC, hum, false); // false = Celsius

  if (isnan(hum) || isnan(tempC)) {
    Serial.println("⚠️ Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.printf("Read: Temp = %.2f °C, Humidity = %.2f %%, HeatIndex = %.2f °C\n",
                tempC, hum, heatIdx);

  sendToThingSpeak(tempC, hum, heatIdx);

  delay(20000); // tuân thủ giới hạn 15s của ThingSpeak
}
