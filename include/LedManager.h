#pragma once

#include <Arduino.h>

#include "AppTypes.h"

class LedManager {
 public:
  explicit LedManager(uint8_t pin);

  void begin();
  void setState(DeviceState state);
  void update();

 private:
  void writeLed(bool on);

  uint8_t pin_;
  DeviceState state_ = DeviceState::Unconfigured;
  uint32_t lastToggleMs_ = 0;
  bool ledOn_ = false;
};
