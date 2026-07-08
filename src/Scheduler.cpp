#include "../include/Scheduler.h"

bool Scheduler::ready(uint32_t intervalMs) {
  const uint32_t now = millis();
  if (firstRun_) {
    firstRun_ = false;
    lastRunMs_ = now;
    return true;
  }
  if (now - lastRunMs_ >= intervalMs) {
    lastRunMs_ = now;
    return true;
  }
  return false;
}

void Scheduler::reset() {
  firstRun_ = true;
  lastRunMs_ = 0;
}
