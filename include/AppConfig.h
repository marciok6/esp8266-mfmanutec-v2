#pragma once

#include <Arduino.h>

namespace AppConfig {
constexpr uint8_t PIN_DS18B20 = D1;
constexpr uint8_t PIN_RELAY = D2;
constexpr uint8_t PIN_STATUS_LED = LED_BUILTIN;

constexpr char API_ENDPOINT[] = "http://www.mfmanutec.com.br/api/iot/receive.php";
constexpr char DEVICE_CODE[] = "07";
constexpr char TELEMETRY_CODE[] = "00";
constexpr char SIGNING_SECRET[] = "MFMANUTEC@2026#IoT!Secure$Key";
constexpr char CONFIG_PATH[] = "/config.json";
constexpr char HOSTNAME_PREFIX[] = "mfmanutec";
constexpr char FIRMWARE_VERSION[] = "1.0.0";
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30000;
constexpr uint32_t HTTP_TIMEOUT_MS = 10000;
constexpr uint32_t DEFAULT_INTERVAL_SECONDS = 60;
constexpr float DEFAULT_MIN_TEMPERATURE = 30.0F;
constexpr float DEFAULT_MAX_TEMPERATURE = 80.0F;
constexpr float LOW_VOLTAGE = 2.9F;
constexpr float HIGH_VOLTAGE = 3.6F;
}
