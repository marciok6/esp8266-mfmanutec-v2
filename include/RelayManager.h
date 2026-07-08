#pragma once

#include <Arduino.h>

class RelayManager {
 public:
  explicit RelayManager(uint8_t pin);

  void begin();
  bool setState(bool enabled);
  bool getState() const;

 private:
  uint8_t pin_;
  bool state_ = false;
};
