#include "../include/RelayManager.h"

RelayManager::RelayManager(uint8_t pin) : pin_(pin) {}

void RelayManager::begin() {
  pinMode(pin_, OUTPUT);
  setState(false);
}

bool RelayManager::setState(bool enabled) {
  state_ = enabled;
  digitalWrite(pin_, enabled ? HIGH : LOW);
  return digitalRead(pin_) == (enabled ? HIGH : LOW);
}

bool RelayManager::getState() const { return state_; }
