#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// ===== Настройки Wi-Fi =====
const char* ssid = "HUAWEI_E5783_4CE3";
const char* password = "AYMiag7n";

// ===== GitHub ссылки =====
const char* firmwareURL = "https://raw.githubusercontent.com/walladiy/Wi-Fi_temp-controller/main/firmware.bin";
const char* versionURL  = "https://raw.githubusercontent.com/walladiy/Wi-Fi_temp-controller/main/version.txt";

// ===== Текущая версия прошивки =====
const char* currentVersion = "1.0.0"; // изменяйте при каждом релизе

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  checkForUpdate();
}

void loop() {
  // ничего не делаем
}

void checkForUpdate() {
  Serial.println("Checking for firmware update...");

  HTTPClient http;
  http.begin(versionURL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String latestVersion = http.getString();
    latestVersion.trim();
    Serial.println("Latest version: " + latestVersion);

    if (latestVersion != currentVersion) {
      Serial.println("New version found! Updating...");
      performOTA();
    } else {
      Serial.println("Firmware is up-to-date.");
    }
  } else {
    Serial.println("Failed to fetch version file. HTTP code: " + String(httpCode));
  }

  http.end();
}

void performOTA() {
  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);
    if (canBegin) {
      WiFiClient* client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);

      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " bytes successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength));
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("Update finished. Rebooting...");
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Update error #: " + String(Update.getError()));
      }
    } else {
      Serial.println("Not enough space to begin OTA");
    }
  } else {
    Serial.println("HTTP GET failed, error: " + String(httpCode));
  }

  http.end();
}
