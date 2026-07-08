#pragma once

#include <ArduinoJson.h>

#include "AppTypes.h"

class DeviceInfo;
class Security;

class ApiClient {
 public:
  ApiClient(const DeviceInfo& deviceInfo, const Security& security);

  bool provision(const DeviceConfig& baseConfig, const RuntimeData& runtime, JsonDocument& response) const;
  bool sendTelemetry(const DeviceConfig& config, const RuntimeData& runtime, JsonDocument& response) const;
  bool applyServerConfig(DeviceConfig& config, const JsonVariantConst& configJson) const;
  bool extractOtaMetadata(const JsonDocument& response, OtaMetadata& metadata) const;

 private:
  bool postPayload(const JsonDocument& payload, JsonDocument& response) const;
  void fillBasePayload(JsonDocument& payload, const String& code, const RuntimeData& runtime, const String& chipId) const;

  const DeviceInfo& deviceInfo_;
  const Security& security_;
};
