#pragma once
#include <stdint.h>

#ifndef CUSTOM_IAT_MAX_RECORD_SECONDS
#define CUSTOM_IAT_MAX_RECORD_SECONDS 60
#endif
static_assert(CUSTOM_IAT_MAX_RECORD_SECONDS >= 1 && CUSTOM_IAT_MAX_RECORD_SECONDS <= 120,
    "CUSTOM IAT recording limit must be 1..120 seconds");

static constexpr uint32_t voiceAsrRecordLimitMs(bool customIat,uint32_t legacySeconds) {
  return (customIat ? uint32_t(CUSTOM_IAT_MAX_RECORD_SECONDS) : legacySeconds)*1000U;
}
static constexpr uint32_t voiceMicCapacitySamples(uint32_t rate,uint32_t recordingMs,uint32_t tailMs) {
  // Allocate the entire recording plus tail and 2s scheduler headroom. Use
  // milliseconds so a fractional-second tail is never rounded down.
  return uint32_t((uint64_t(rate)*(recordingMs+tailMs+2000U)+999U)/1000U);
}
static constexpr uint32_t voiceAsrUploadLimitBytes(uint32_t rate,bool customIat,uint32_t legacySeconds,uint32_t tailMs) {
  return customIat ? voiceMicCapacitySamples(rate,voiceAsrRecordLimitMs(true,legacySeconds),tailMs)*2U : rate*2U*60U;
}
