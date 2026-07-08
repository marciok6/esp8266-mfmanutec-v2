#pragma once

#include <Arduino.h>

class Logger {
 public:
  void begin();
  void log(int code, const String& reason, uint32_t uptime, const String& extra = "") const;
};
