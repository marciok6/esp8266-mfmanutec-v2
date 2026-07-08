#pragma once

#include <Arduino.h>

class Scheduler {
 public:
  bool ready(uint32_t intervalMs);
  void reset();

 private:
  uint32_t lastRunMs_ = 0;
  bool firstRun_ = true;
};
