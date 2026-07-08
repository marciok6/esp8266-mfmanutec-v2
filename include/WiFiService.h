#pragma once

#include <Arduino.h>

class WiFiService {
 public:
  void beginStation();
  bool connect(const String& ssid, const String& password, uint32_t timeoutMs) const;
  void disconnect() const;
  bool isConnected() const;
  int rssi() const;
  String ip() const;
  void beginAccessPoint(const String& ssid) const;
};
