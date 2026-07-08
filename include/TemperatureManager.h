#pragma once

#include <DallasTemperature.h>
#include <OneWire.h>

class TemperatureManager {
 public:
  explicit TemperatureManager(uint8_t pin);

  bool begin();
  float readCelsius();

 private:
  OneWire oneWire_;
  DallasTemperature sensors_;
};
