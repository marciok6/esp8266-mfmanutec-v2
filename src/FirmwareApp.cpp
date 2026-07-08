#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "../include/ApiClient.h"
#include "../include/AppConfig.h"
#include "../include/AppTypes.h"
#include "../include/CaptivePortal.h"
#include "../include/ConfigManager.h"
#include "../include/DeviceInfo.h"
#include "../include/FirmwareApp.h"
#include "../include/LedManager.h"
#include "../include/LittleFSManager.h"
#include "../include/Logger.h"
#include "../include/OTAUpdate.h"
#include "../include/RelayManager.h"
#include "../include/Scheduler.h"
#include "../include/Security.h"
#include "../include/SystemStatus.h"
#include "../include/TemperatureManager.h"
#include "../include/WatchdogManager.h"
#include "../include/WiFiService.h"

namespace {
Logger logger;
SystemStatus systemStatus;
LittleFSManager fsManager;
ConfigManager configManager;
DeviceInfo deviceInfo;
Security security;
LedManager ledManager(AppConfig::PIN_STATUS_LED);
RelayManager relayManager(AppConfig::PIN_RELAY);
TemperatureManager temperatureManager(AppConfig::PIN_DS18B20);
WiFiService wifiService;
CaptivePortal captivePortal;
ApiClient apiClient(deviceInfo, security);
OTAUpdate otaUpdate(security, logger);
Scheduler telemetryScheduler;
WatchdogManager watchdog;

DeviceConfig config;
DeviceConfig pendingConfig;
bool sensorAvailable = false;
bool provisioningReadyToFinalize = false;

RuntimeData snapshotRuntime(int code) {
  RuntimeData runtime;
  runtime.temperature = sensorAvailable ? temperatureManager.readCelsius() : NAN;
  runtime.wifiRssi = wifiService.rssi();
  runtime.voltage = deviceInfo.voltage();
  runtime.relayState = relayManager.getState() ? 1 : 0;
  runtime.uptime = millis() / 1000;
  runtime.code = code;
  return runtime;
}

DiagnosticSummary buildDiagnosticSummary(const String& reason, int code) {
  RuntimeData runtime = snapshotRuntime(code);
  DiagnosticSummary summary;
  summary.ready = true;
  summary.code = code;
  summary.reason = reason;
  summary.chipId = deviceInfo.chipId();
  summary.mac = deviceInfo.macAddress();
  summary.apSsid = deviceInfo.apSsid();
  summary.stationSsid = config.ssid;
  summary.stationIp = wifiService.ip();
  summary.apiEndpoint = AppConfig::API_ENDPOINT;
  summary.deviceStatus = config.status;
  summary.cliente = config.cliente;
  summary.descricao = config.descricao;
  summary.localInstalacao = config.localInstalacao;
  summary.temperature = runtime.temperature;
  summary.wifiRssi = runtime.wifiRssi;
  summary.voltage = runtime.voltage;
  summary.relayState = runtime.relayState;
  summary.uptime = runtime.uptime;
  return summary;
}

int validateRuntime(const RuntimeData& runtime) {
  if (!sensorAvailable) {
    return CODE_SENSOR_NOT_FOUND;
  }
  if (runtime.temperature == DEVICE_DISCONNECTED_C || runtime.temperature == -127.0F) {
    return CODE_INVALID_TEMPERATURE;
  }
  if (isnan(runtime.temperature)) {
    return CODE_SENSOR_READ_ERROR;
  }
  if (runtime.voltage < AppConfig::LOW_VOLTAGE) {
    return CODE_LOW_VOLTAGE;
  }
  if (runtime.voltage > AppConfig::HIGH_VOLTAGE) {
    return CODE_HIGH_VOLTAGE;
  }
  if (runtime.temperature < config.minTemperature || runtime.temperature > config.maxTemperature) {
    return CODE_TEMPERATURE_OUT_OF_RANGE;
  }
  return CODE_NORMAL;
}

void enterAccessPointMode(const String& reason, int code) {
  systemStatus.setState(DeviceState::AccessPoint);
  systemStatus.setCode(code);
  ledManager.setState(DeviceState::AccessPoint);
  wifiService.beginAccessPoint(deviceInfo.apSsid());
  captivePortal.begin(deviceInfo.apSsid());
  captivePortal.setPortalMode(code == CODE_WIFI_DISCONNECTED || code == CODE_SYNC_FAILURE || code == CODE_JSON_SEND_ERROR
                                  ? PortalMode::Diagnostics
                                  : PortalMode::Provisioning);
  if (code == CODE_WIFI_DISCONNECTED || code == CODE_SYNC_FAILURE || code == CODE_JSON_SEND_ERROR) {
    captivePortal.setDiagnosticSummary(buildDiagnosticSummary(reason, code));
  } else {
    captivePortal.clearDiagnosticSummary();
  }
  captivePortal.setStatusMessage(reason);
  logger.log(code, reason, millis() / 1000, WiFi.softAPIP().toString());
}

bool applyRelayState(int relayState, int successCode, int failureCode) {
  bool applied = relayManager.setState(relayState == 1);
  logger.log(applied ? successCode : failureCode, applied ? "Relay atualizado" : "Falha ao acionar rele",
             millis() / 1000, String(relayState));
  return applied;
}

bool performProvisioning(const ProvisioningRequest& request) {
  captivePortal.setStatusMessage("Conectando ao Wi-Fi...");
  systemStatus.setState(DeviceState::WifiConnecting);
  ledManager.setState(DeviceState::WifiConnecting);

  wifiService.disconnect();
  if (!wifiService.connect(request.ssid, request.password, AppConfig::WIFI_CONNECT_TIMEOUT_MS)) {
    systemStatus.setCode(CODE_WIFI_AUTH_FAILURE);
    captivePortal.setStatusMessage("Falha ao conectar. Tente novamente.");
    ledManager.setState(DeviceState::AccessPoint);
    systemStatus.setState(DeviceState::AccessPoint);
    logger.log(CODE_WIFI_AUTH_FAILURE, "Falha na autenticacao Wi-Fi", millis() / 1000, request.ssid);
    return false;
  }

  captivePortal.setStatusMessage("Conectado. Sincronizando com servidor...");
  DeviceConfig draft;
  draft.ssid = request.ssid;
  draft.password = request.password;
  draft.chipId = deviceInfo.chipId();
  draft.firmware = AppConfig::FIRMWARE_VERSION;

  DynamicJsonDocument response(2048);
  RuntimeData runtime = snapshotRuntime(CODE_NORMAL);
  if (!apiClient.provision(draft, runtime, response)) {
    systemStatus.setCode(CODE_SYNC_FAILURE);
    logger.log(CODE_SYNC_FAILURE, "Falha na sincronizacao inicial", runtime.uptime);
    enterAccessPointMode("Falha de comunicacao com o servidor durante a sincronizacao.", CODE_SYNC_FAILURE);
    return false;
  }

  apiClient.applyServerConfig(draft, response["config"]);
  applyRelayState(draft.relayState, CODE_COMMAND_EXECUTED, CODE_COMMAND_FAILED);
  pendingConfig = draft;
  provisioningReadyToFinalize = true;

  ProvisioningSummary summary;
  summary.ready = true;
  summary.chipId = draft.chipId;
  summary.mac = deviceInfo.macAddress();
  summary.ip = wifiService.ip();
  summary.cliente = draft.cliente;
  summary.descricao = draft.descricao;
  summary.localInstalacao = draft.localInstalacao;
  summary.status = draft.status;
  summary.expectedInterval = draft.expectedInterval;
  summary.minTemperature = draft.minTemperature;
  summary.maxTemperature = draft.maxTemperature;
  summary.relayState = draft.relayState;
  captivePortal.setProvisioningSummary(summary);
  captivePortal.setStatusMessage("Configuracao recebida do servidor. Revise os dados e conclua.");
  return true;
}

void finalizeProvisioning() {
  if (!provisioningReadyToFinalize) {
    return;
  }

  if (!configManager.save(pendingConfig)) {
    systemStatus.setCode(CODE_CONFIG_SAVE_ERROR);
    captivePortal.setStatusMessage("Erro ao salvar configuracao.");
    logger.log(CODE_CONFIG_SAVE_ERROR, "Erro ao persistir configuracao", millis() / 1000);
    return;
  }

  config = pendingConfig;
  provisioningReadyToFinalize = false;
  captivePortal.setStatusMessage("Configuracao concluida. Reiniciando...");
  delay(1500);
  ESP.restart();
}

bool connectWithSavedConfig() {
  systemStatus.setState(DeviceState::WifiConnecting);
  systemStatus.setCode(CODE_NORMAL);
  ledManager.setState(DeviceState::WifiConnecting);
  if (!wifiService.connect(config.ssid, config.password, AppConfig::WIFI_CONNECT_TIMEOUT_MS)) {
    logger.log(CODE_WIFI_DISCONNECTED, "Falha ao conectar com configuracao salva", millis() / 1000, config.ssid);
    return false;
  }
  logger.log(CODE_CONFIG_SYNCED, "Wi-Fi conectado", millis() / 1000, wifiService.ip());
  return true;
}

void processApiResponse(const JsonDocument& response) {
  if (response["codigo"] != 100) {
    logger.log(CODE_API_RESPONSE_INVALID, "Resposta da API fora do contrato", millis() / 1000);
    return;
  }

  bool configChanged = apiClient.applyServerConfig(config, response["config"]);
  JsonVariantConst command = response["comando"];
  if (!command.isNull() && command.containsKey("relay_state")) {
    const int relayState = command["relay_state"] | config.relayState;
    if (applyRelayState(relayState, CODE_COMMAND_EXECUTED, CODE_COMMAND_FAILED)) {
      config.relayState = relayState;
      configChanged = true;
    }
  }

  OtaMetadata metadata;
  if (apiClient.extractOtaMetadata(response, metadata)) {
    otaUpdate.apply(metadata, millis() / 1000);
  }

  if (configChanged) {
    config.firmware = AppConfig::FIRMWARE_VERSION;
    if (configManager.save(config)) {
      logger.log(CODE_CONFIG_SYNCED, "Configuracao sincronizada", millis() / 1000);
    } else {
      logger.log(CODE_CONFIG_SYNC_FAILURE, "Falha ao salvar configuracao sincronizada", millis() / 1000);
    }
  }
}

void runNormalOperation() {
  ledManager.setState(DeviceState::Normal);
  systemStatus.setState(DeviceState::Normal);

  if (!wifiService.isConnected()) {
    if (!connectWithSavedConfig()) {
      enterAccessPointMode("Falha ao reconectar. Reconfigure o dispositivo.", CODE_WIFI_DISCONNECTED);
    }
    return;
  }

  if (!telemetryScheduler.ready(config.expectedInterval * 1000UL)) {
    return;
  }

  RuntimeData runtime = snapshotRuntime(CODE_NORMAL);
  runtime.code = validateRuntime(runtime);
  systemStatus.setCode(runtime.code);
  if (runtime.code != CODE_NORMAL) {
    logger.log(runtime.code, "Leitura fora do esperado", runtime.uptime);
  }

  DynamicJsonDocument response(2048);
  if (!apiClient.sendTelemetry(config, runtime, response)) {
    logger.log(CODE_JSON_SEND_ERROR, "Falha ao enviar telemetria", runtime.uptime);
    enterAccessPointMode("Falha de comunicacao com o servidor. Dashboard de diagnostico ativo.", CODE_JSON_SEND_ERROR);
    return;
  }
  processApiResponse(response);
}
}

void appSetup() {
  logger.begin();
  ledManager.begin();
  relayManager.begin();
  watchdog.begin();
  wifiService.beginStation();

  if (!fsManager.begin()) {
    systemStatus.setState(DeviceState::CriticalError);
    systemStatus.setCode(CODE_FILESYSTEM_ERROR);
    ledManager.setState(DeviceState::CriticalError);
    logger.log(CODE_FILESYSTEM_ERROR, "Falha ao montar LittleFS", millis() / 1000);
    enterAccessPointMode("Erro de leitura do armazenamento. Reconfigure o dispositivo.", CODE_FILESYSTEM_ERROR);
    return;
  }

  sensorAvailable = temperatureManager.begin();
  if (!sensorAvailable) {
    logger.log(CODE_SENSOR_NOT_FOUND, "Sensor DS18B20 nao detectado", millis() / 1000);
  }

  if (!configManager.load(config)) {
    config = DeviceConfig{};
    config.chipId = deviceInfo.chipId();
    config.firmware = AppConfig::FIRMWARE_VERSION;
    ledManager.setState(DeviceState::Unconfigured);
    enterAccessPointMode("Sem configuracao. Acesse o portal para continuar.", CODE_CONFIG_NOT_FOUND);
    return;
  }

  config.chipId = deviceInfo.chipId();
  config.firmware = AppConfig::FIRMWARE_VERSION;
  applyRelayState(config.relayState, CODE_COMMAND_EXECUTED, CODE_COMMAND_FAILED);

  if (!connectWithSavedConfig()) {
    enterAccessPointMode("Nao foi possivel conectar ao Wi-Fi salvo.", CODE_WIFI_DISCONNECTED);
    return;
  }

  RuntimeData runtime = snapshotRuntime(CODE_NORMAL);
  DynamicJsonDocument response(2048);
  if (apiClient.sendTelemetry(config, runtime, response)) {
    processApiResponse(response);
  }
}

void appLoop() {
  watchdog.feed();
  ledManager.update();

  if (captivePortal.isRunning()) {
    captivePortal.loop();
    ProvisioningRequest request;
    if (captivePortal.consumeProvisioningRequest(request)) {
      performProvisioning(request);
    }
    if (captivePortal.consumeFinalizeRequest()) {
      finalizeProvisioning();
    }
    return;
  }

  runNormalOperation();
}
