#include "DeviceInfo.h"

#include <ESP8266WiFi.h>

#include "AppConfig.h"

String DeviceInfo::chipId() const { return String(ESP.getChipId(), HEX); }

String DeviceInfo::macAddress() const { return WiFi.macAddress(); }

String DeviceInfo::apSsid() const {
  String suffix = chipId();
  suffix.toUpperCase();
  return String("MFMANUTEC-") + suffix;
}

float DeviceInfo::voltage() const { return static_cast<float>(ESP.getVcc()) / 1000.0F; }
