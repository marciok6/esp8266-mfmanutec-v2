#include "LedManager.h"

LedManager::LedManager(uint8_t pin) : pin_(pin) {}

void LedManager::begin() {
  pinMode(pin_, OUTPUT);
  writeLed(false);
}

void LedManager::setState(DeviceState state) { state_ = state; }

void LedManager::update() {
  const uint32_t now = millis();
  switch (state_) {
    case DeviceState::Normal:
      writeLed(true);
      return;
    case DeviceState::Unconfigured:
      if (now - lastToggleMs_ >= 200) {
        lastToggleMs_ = now;
        writeLed(!ledOn_);
      }
      return;
    case DeviceState::WifiConnecting:
      if (now - lastToggleMs_ >= 800) {
        lastToggleMs_ = now;
        writeLed(!ledOn_);
      }
      return;
    case DeviceState::AccessPoint:
      if (now - lastToggleMs_ >= 500) {
        lastToggleMs_ = now;
        writeLed(!ledOn_);
      }
      return;
    case DeviceState::CriticalError: {
      const uint32_t phase = now % 2000;
      const bool on = (phase < 120) || (phase >= 240 && phase < 360);
      writeLed(on);
      return;
    }
  }
}

void LedManager::writeLed(bool on) {
  ledOn_ = on;
  digitalWrite(pin_, on ? LOW : HIGH);
}
