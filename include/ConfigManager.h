#pragma once

#include "AppTypes.h"

class ConfigManager {
 public:
  bool load(DeviceConfig& config) const;
  bool save(const DeviceConfig& config) const;
  bool clear() const;
};
