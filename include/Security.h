#pragma once

#include <Arduino.h>

class Security {
 public:
  String signatureForChipId(const String& chipId) const;
  String sha256(const String& input) const;
};
