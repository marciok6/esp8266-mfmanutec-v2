#pragma once

#include <Arduino.h>

class DeviceInfo {
 public:
  String chipId() const;
  String macAddress() const;
  String apSsid() const;
  float voltage() const;
};
