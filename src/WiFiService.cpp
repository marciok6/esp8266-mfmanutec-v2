#include "../include/WiFiService.h"

#include <ESP8266WiFi.h>

void WiFiService::beginStation() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
}

bool WiFiService::connect(const String& ssid, const String& password, uint32_t timeoutMs) const {
  const WiFiMode_t currentMode = WiFi.getMode();
  WiFi.mode((currentMode == WIFI_AP || currentMode == WIFI_AP_STA) ? WIFI_AP_STA : WIFI_STA);
  WiFi.hostname("mfmanutec-device");
  WiFi.begin(ssid.c_str(), password.c_str());
  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(250);
    yield();
  }
  return WiFi.status() == WL_CONNECTED;
}

void WiFiService::disconnect() const { WiFi.disconnect(false); }

bool WiFiService::isConnected() const { return WiFi.status() == WL_CONNECTED; }

int WiFiService::rssi() const { return isConnected() ? WiFi.RSSI() : 0; }

String WiFiService::ip() const { return WiFi.localIP().toString(); }

void WiFiService::beginAccessPoint(const String& ssid) const {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid.c_str());
}
