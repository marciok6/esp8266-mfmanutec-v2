#include "../include/TemperatureManager.h"

TemperatureManager::TemperatureManager(uint8_t pin) : oneWire_(pin), sensors_(&oneWire_) {}

bool TemperatureManager::begin() {
  sensors_.begin();
  return sensors_.getDeviceCount() > 0;
}

float TemperatureManager::readCelsius() {
  sensors_.requestTemperatures();
  return sensors_.getTempCByIndex(0);
}
