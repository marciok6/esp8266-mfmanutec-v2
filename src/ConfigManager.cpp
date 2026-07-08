#include "ConfigManager.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

#include "AppConfig.h"

bool ConfigManager::load(DeviceConfig& config) const {
  if (!LittleFS.exists(AppConfig::CONFIG_PATH)) {
    return false;
  }

  File file = LittleFS.open(AppConfig::CONFIG_PATH, "r");
  if (!file) {
    return false;
  }

  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, file);
  file.close();
  if (error) {
    return false;
  }

  config.ssid = json["ssid"] | "";
  config.password = json["password"] | "";
  config.chipId = json["chip_id"] | "";
  config.cliente = json["cliente"] | "";
  config.descricao = json["descricao"] | "";
  config.localInstalacao = json["local_instalacao"] | "";
  config.expectedInterval = json["expected_interval"] | AppConfig::DEFAULT_INTERVAL_SECONDS;
  config.minTemperature = json["min_temperature"] | AppConfig::DEFAULT_MIN_TEMPERATURE;
  config.maxTemperature = json["max_temperature"] | AppConfig::DEFAULT_MAX_TEMPERATURE;
  config.status = json["status"] | "Ativo";
  config.relayState = json["relay_state"] | 0;
  config.firmware = json["firmware"] | AppConfig::FIRMWARE_VERSION;
  return config.isValid();
}

bool ConfigManager::save(const DeviceConfig& config) const {
  File file = LittleFS.open(AppConfig::CONFIG_PATH, "w");
  if (!file) {
    return false;
  }

  StaticJsonDocument<512> json;
  json["ssid"] = config.ssid;
  json["password"] = config.password;
  json["chip_id"] = config.chipId;
  json["cliente"] = config.cliente;
  json["descricao"] = config.descricao;
  json["local_instalacao"] = config.localInstalacao;
  json["expected_interval"] = config.expectedInterval;
  json["min_temperature"] = config.minTemperature;
  json["max_temperature"] = config.maxTemperature;
  json["status"] = config.status;
  json["relay_state"] = config.relayState;
  json["firmware"] = config.firmware;

  bool ok = serializeJson(json, file) > 0;
  file.close();
  return ok;
}

bool ConfigManager::clear() const {
  if (!LittleFS.exists(AppConfig::CONFIG_PATH)) {
    return true;
  }
  return LittleFS.remove(AppConfig::CONFIG_PATH);
}
