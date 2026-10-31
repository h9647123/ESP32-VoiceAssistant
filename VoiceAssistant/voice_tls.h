#pragma once
#include <WiFiClientSecure.h>
#include <time.h>
#include "config.h"

#ifndef VOICE_TLS_CA_PEM
#define VOICE_TLS_CA_PEM nullptr
#endif
#ifndef VOICE_TLS_DIAGNOSTIC_INSECURE
#define VOICE_TLS_DIAGNOSTIC_INSECURE 0
#endif

// Arduino ESP32 3.3.8 / ESP-IDF ships the Mozilla trust bundle in libmbedtls.
extern const uint8_t voiceCaBundleStart[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t voiceCaBundleEnd[] asm("_binary_x509_crt_bundle_end");

static bool voiceEnsureTlsClock() {
#if VOICE_TLS_DIAGNOSTIC_INSECURE
  return true;
#else
  if (time(nullptr) >= 1700000000) return true;
  configTime(8 * 3600, 0, "ntp.aliyun.com", "ntp1.aliyun.com", "pool.ntp.org");
  const uint32_t started = millis();
  while (time(nullptr) < 1700000000 && millis() - started < 12000) {
    delay(50);
    yield();
  }
  return time(nullptr) >= 1700000000;
#endif
}

static bool voiceConfigureTls(WiFiClientSecure& client) {
#if VOICE_TLS_DIAGNOSTIC_INSECURE
  Serial.println("[TLS] DIAGNOSTIC ONLY: certificate verification disabled");
  client.setInsecure();
#else
  if (VOICE_TLS_CA_PEM) client.setCACert(VOICE_TLS_CA_PEM);
  else client.setCACertBundle(voiceCaBundleStart, voiceCaBundleEnd - voiceCaBundleStart);
#endif
  const bool clockReady = voiceEnsureTlsClock();
  if (!clockReady) Serial.println("[TLS] NTP failed; certificate verification remains required");
  return clockReady;
}
