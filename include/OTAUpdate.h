#pragma once

#include "AppTypes.h"

class Logger;
class Security;

class OTAUpdate {
 public:
  OTAUpdate(const Security& security, const Logger& logger);

  bool apply(const OtaMetadata& metadata, uint32_t uptime) const;

 private:
  const Security& security_;
  const Logger& logger_;
};
