#include "../include/OTAUpdate.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Updater.h>
#include <WiFiClientSecureBearSSL.h>

#include <bearssl/bearssl_hash.h>

#include "../include/Logger.h"
#include "../include/Security.h"

namespace {
template <typename TClient>
bool writeFirmwareStream(TClient& client, const OtaMetadata& metadata, const Logger& logger, uint32_t uptime) {
  HTTPClient http;
  http.setTimeout(15000);
  if (!http.begin(client, metadata.url)) {
    logger.log(CODE_OTA_ERROR, "Falha ao iniciar download OTA", uptime, metadata.url);
    return false;
  }

  const int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    logger.log(CODE_OTA_ERROR, "Resposta HTTP invalida no OTA", uptime, String(httpCode));
    http.end();
    return false;
  }

  const int contentLength = http.getSize();
  if (contentLength <= 0 || !Update.begin(contentLength)) {
    logger.log(CODE_OTA_ERROR, "Nao foi possivel iniciar gravacao OTA", uptime, String(contentLength));
    http.end();
    return false;
  }

  br_sha256_context shaContext;
  br_sha256_init(&shaContext);
  uint8_t hash[32];
  uint8_t buffer[512];
  WiFiClient* stream = http.getStreamPtr();
  int remaining = contentLength;

  while (http.connected() && (remaining > 0 || remaining == -1)) {
    size_t available = stream->available();
    if (available == 0) {
      delay(1);
      yield();
      continue;
    }

    const size_t chunk = stream->readBytes(buffer, std::min(available, sizeof(buffer)));
    if (chunk == 0) {
      continue;
    }

    br_sha256_update(&shaContext, buffer, chunk);
    if (Update.write(buffer, chunk) != chunk) {
      logger.log(CODE_OTA_ERROR, "Falha ao escrever firmware OTA", uptime, String(Update.getError()));
      http.end();
      return false;
    }

    if (remaining > 0) {
      remaining -= static_cast<int>(chunk);
    }
    yield();
  }

  br_sha256_out(&shaContext, hash);
  char calculated[65];
  for (size_t index = 0; index < sizeof(hash); ++index) {
    snprintf(calculated + (index * 2), 3, "%02x", hash[index]);
  }
  calculated[64] = '\0';

  String expected = metadata.sha256;
  expected.toLowerCase();
  if (String(calculated) != expected) {
    logger.log(CODE_OTA_ERROR, "Hash SHA-256 do OTA invalido", uptime, calculated);
    http.end();
    return false;
  }

  if (!Update.end()) {
    logger.log(CODE_OTA_ERROR, "Falha ao finalizar OTA", uptime, String(Update.getError()));
    http.end();
    return false;
  }

  http.end();
  logger.log(CODE_NORMAL, "OTA concluido com sucesso", uptime, metadata.version);
  ESP.restart();
  return true;
}
}

OTAUpdate::OTAUpdate(const Security& security, const Logger& logger) : security_(security), logger_(logger) {}

bool OTAUpdate::apply(const OtaMetadata& metadata, uint32_t uptime) const {
  if (!metadata.available || metadata.url.isEmpty() || metadata.sha256.isEmpty()) {
    return false;
  }

  if (metadata.url.startsWith("https://")) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
    return writeFirmwareStream(client, metadata, logger_, uptime);
  }

  WiFiClient client;
  return writeFirmwareStream(client, metadata, logger_, uptime);
}
