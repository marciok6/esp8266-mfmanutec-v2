#include "ApiClient.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "AppConfig.h"
#include "DeviceInfo.h"
#include "Security.h"

ApiClient::ApiClient(const DeviceInfo& deviceInfo, const Security& security)
    : deviceInfo_(deviceInfo), security_(security) {}

bool ApiClient::provision(const DeviceConfig& baseConfig, const RuntimeData& runtime, JsonDocument& response) const {
  DynamicJsonDocument payload(768);
  fillBasePayload(payload, AppConfig::DEVICE_CODE, runtime, baseConfig.chipId);
  return postPayload(payload, response);
}

bool ApiClient::sendTelemetry(const DeviceConfig& config, const RuntimeData& runtime, JsonDocument& response) const {
  DynamicJsonDocument payload(768);
  fillBasePayload(payload, AppConfig::TELEMETRY_CODE, runtime, config.chipId);
  return postPayload(payload, response);
}

bool ApiClient::applyServerConfig(DeviceConfig& config, const JsonVariantConst& configJson) const {
  if (configJson.isNull()) {
    return false;
  }

  config.status = configJson["status"] | config.status;
  config.cliente = configJson["cliente"] | config.cliente;
  config.descricao = configJson["descricao"] | config.descricao;
  config.localInstalacao = configJson["local_instalacao"] | config.localInstalacao;
  config.expectedInterval = configJson["expected_interval"] | config.expectedInterval;
  config.minTemperature = configJson["min_temperature"] | config.minTemperature;
  config.maxTemperature = configJson["max_temperature"] | config.maxTemperature;
  config.relayState = configJson["relay_state"] | config.relayState;
  return true;
}

bool ApiClient::extractOtaMetadata(const JsonDocument& response, OtaMetadata& metadata) const {
  JsonVariantConst update = response["atualizacao"];
  if (update.isNull()) {
    metadata = OtaMetadata{};
    return false;
  }

  metadata.available = (update["disponivel"] | 0) == 1;
  metadata.version = String(update["versao"] | "");
  metadata.url = String(update["url"] | "");
  metadata.sha256 = String(update["hash"] | "");
  return metadata.available;
}

bool ApiClient::postPayload(const JsonDocument& payload, JsonDocument& response) const {
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(AppConfig::HTTP_TIMEOUT_MS);
  if (!http.begin(client, AppConfig::API_ENDPOINT)) {
    return false;
  }

  http.addHeader("Content-Type", "application/json");
  String body;
  serializeJson(payload, body);
  const int httpCode = http.POST(body);
  if (httpCode <= 0) {
    http.end();
    return false;
  }

  const String responseBody = http.getString();
  http.end();
  return deserializeJson(response, responseBody) == DeserializationError::Ok;
}

void ApiClient::fillBasePayload(JsonDocument& payload, const String& code, const RuntimeData& runtime,
                                const String& chipId) const {
  payload["codigo"] = code;
  payload["chip_id"] = chipId;
  payload["assinatura"] = security_.signatureForChipId(chipId);

  JsonObject data = payload.createNestedObject("data");
  data["mac"] = deviceInfo_.macAddress();
  data["firmware"] = AppConfig::FIRMWARE_VERSION;
  data["temperatura"] = runtime.temperature;
  data["wifi"] = runtime.wifiRssi;
  data["tensao"] = runtime.voltage;
  data["relay_state"] = runtime.relayState;
}
