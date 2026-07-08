#pragma once

#include <Arduino.h>

enum class DeviceState : uint8_t {
  Unconfigured,
  WifiConnecting,
  Normal,
  CriticalError,
  AccessPoint
};

enum class PortalMode : uint8_t {
  Provisioning,
  Diagnostics
};

enum DeviceCode : int {
  CODE_NORMAL = 0,
  CODE_SENSOR_NOT_FOUND = 1,
  CODE_INVALID_TEMPERATURE = 2,
  CODE_SENSOR_READ_ERROR = 3,
  CODE_TEMPERATURE_OUT_OF_RANGE = 4,
  CODE_RELAY_FAILURE = 5,
  CODE_CONFIG_NOT_FOUND = 7,
  CODE_CONFIG_SAVE_ERROR = 8,
  CODE_CONFIG_READ_ERROR = 9,
  CODE_WIFI_DISCONNECTED = 10,
  CODE_WIFI_AUTH_FAILURE = 11,
  CODE_SERVER_UNAVAILABLE = 12,
  CODE_HTTP_TIMEOUT = 13,
  CODE_INVALID_SIGNATURE = 14,
  CODE_DEVICE_NOT_REGISTERED = 15,
  CODE_FILESYSTEM_ERROR = 16,
  CODE_LOW_HEAP = 17,
  CODE_WATCHDOG = 18,
  CODE_UNEXPECTED_RESTART = 19,
  CODE_OTA_ERROR = 20,
  CODE_LOW_VOLTAGE = 22,
  CODE_HIGH_VOLTAGE = 23,
  CODE_INTERNAL_ERROR = 24,
  CODE_JSON_INVALID = 25,
  CODE_JSON_SEND_ERROR = 26,
  CODE_API_RESPONSE_INVALID = 27,
  CODE_SYNC_FAILURE = 30,
  CODE_AP_ACTIVE = 33,
  CODE_FACTORY_RESET = 34,
  CODE_SENSOR_DISCONNECTED = 35,
  CODE_WIFI_RETRIES_EXCEEDED = 37,
  CODE_CRITICAL_TEMPERATURE = 39,
  CODE_CONFIG_SYNCED = 41,
  CODE_CONFIG_SYNC_FAILURE = 42,
  CODE_COMMAND_EXECUTED = 43,
  CODE_COMMAND_FAILED = 44
};

struct DeviceConfig {
  String ssid;
  String password;
  String chipId;
  String cliente;
  String descricao;
  String localInstalacao;
  uint32_t expectedInterval = 60;
  float minTemperature = 30.0F;
  float maxTemperature = 80.0F;
  String status = "Ativo";
  int relayState = 0;
  String firmware = "1.0.0";

  bool isValid() const { return !ssid.isEmpty() && !chipId.isEmpty(); }
};

struct RuntimeData {
  float temperature = NAN;
  int wifiRssi = 0;
  float voltage = 0.0F;
  int relayState = 0;
  uint32_t uptime = 0;
  int code = CODE_NORMAL;
};

struct ProvisioningRequest {
  bool submitted = false;
  String ssid;
  String password;
};

struct ProvisioningSummary {
  bool ready = false;
  String chipId;
  String mac;
  String cliente;
  String descricao;
  String localInstalacao;
  String status;
  String ip;
  uint32_t expectedInterval = 0;
  float minTemperature = 0.0F;
  float maxTemperature = 0.0F;
  int relayState = 0;
};

struct DiagnosticSummary {
  bool ready = false;
  int code = CODE_NORMAL;
  String reason;
  String chipId;
  String mac;
  String apSsid;
  String stationSsid;
  String stationIp;
  String apiEndpoint;
  String deviceStatus;
  String cliente;
  String descricao;
  String localInstalacao;
  float temperature = NAN;
  int wifiRssi = 0;
  float voltage = 0.0F;
  int relayState = 0;
  uint32_t uptime = 0;
};

struct OtaMetadata {
  bool available = false;
  String version;
  String url;
  String sha256;
};
