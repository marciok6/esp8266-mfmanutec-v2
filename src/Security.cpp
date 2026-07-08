#include "../include/Security.h"

#include <bearssl/bearssl_hash.h>

#include "../include/AppConfig.h"

String Security::signatureForChipId(const String& chipId) const {
  return sha256(chipId + AppConfig::SIGNING_SECRET);
}

String Security::sha256(const String& input) const {
  uint8_t output[32];
  br_sha256_context context;
  br_sha256_init(&context);
  br_sha256_update(&context, input.c_str(), input.length());
  br_sha256_out(&context, output);

  char encoded[65];
  for (size_t index = 0; index < sizeof(output); ++index) {
    snprintf(encoded + (index * 2), 3, "%02x", output[index]);
  }
  encoded[64] = '\0';
  return String(encoded);
}
