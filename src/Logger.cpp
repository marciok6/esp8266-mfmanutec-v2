#include "../include/Logger.h"

#include <ArduinoJson.h>

void Logger::begin() {
  Serial.begin(115200);
  Serial.println();
}

void Logger::log(int code, const String& reason, uint32_t uptime, const String& extra) const {
  StaticJsonDocument<256> entry;
  entry["codigo"] = code;
  entry["motivo"] = reason;
  entry["timestamp"] = millis();
  entry["uptime"] = uptime;
  if (!extra.isEmpty()) {
    entry["extra"] = extra;
  }
  serializeJson(entry, Serial);
  Serial.println();
}
