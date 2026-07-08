#include "../include/WatchdogManager.h"

#include <Arduino.h>

void WatchdogManager::begin() const { ESP.wdtEnable(8000); }

void WatchdogManager::feed() const { ESP.wdtFeed(); }
