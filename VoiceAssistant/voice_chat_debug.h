#pragma once
#include "voice_math_samples.h"
static void voiceExportSprite(LGFX_Sprite& sprite) {
  Serial.println("[VOICE-VIEW-BEGIN] width=240 height=240 format=RGB888");
  uint8_t row[720], encoded[961];
  for (int y = 0; y < 240; ++y) {
    sprite.readRectRGB(0, y, 240, 1, row);
    size_t length = 0;
    if (mbedtls_base64_encode(encoded, sizeof(encoded), &length, row, sizeof(row))) {
      Serial.println("[VOICE-VIEW-ERROR] Encode failed"); return;
    }
    Serial.write(encoded, length); Serial.println(); yield();
  }
  Serial.println("[VOICE-VIEW-END]");
}
static bool voiceChatScreenshotReady() {
  return uiPage==UiPage::Page1 && appState!=AppState::Thinking && !recording && !asrSessionActive;
}
static void voiceExportChatScreen() {
  if(!voiceChatScreenshotReady()){Serial.println("[UI-SCREENSHOT] unavailable; return to idle home");return;}
  noteInteraction();drawPage1();
  if(!headerSpriteReady || !chatSpriteReady || !footerSpriteReady){Serial.println("[UI-SCREENSHOT] render buffers unavailable");return;}
  LGFX_Sprite frame(&tft);frame.setPsram(true);frame.setColorDepth(16);
  if(!frame.createSprite(240,240)){Serial.println("[UI-SCREENSHOT] PSRAM unavailable");return;}
  frame.fillSprite(TFT_BLACK);
  if(headerSpriteReady)headerSprite.pushSprite(&frame,0,0);
  if(chatSpriteReady)chatSprite.pushSprite(&frame,0,CHAT_AREA_TOP);
  if(footerSpriteReady)footerSprite.pushSprite(&frame,0,202);
  if(chatLines.size()>CHAT_VISIBLE_LINES) {
    const int trackY=CHAT_AREA_TOP+12,trackH=CHAT_AREA_HEIGHT-24;
    frame.fillRoundRect(CHAT_SCROLL_TRACK_X,trackY,CHAT_SCROLL_TRACK_W,trackH,3,frame.color565(180,180,180));
    const uint16_t color=frame.color565(150,210,255);
    frame.fillTriangle(CHAT_SCROLL_CENTER_X,CHAT_AREA_TOP+4,CHAT_SCROLL_CENTER_X-CHAT_SCROLL_ARROW_HALF,CHAT_AREA_TOP+10,CHAT_SCROLL_CENTER_X+CHAT_SCROLL_ARROW_HALF,CHAT_AREA_TOP+10,color);
    frame.fillTriangle(CHAT_SCROLL_CENTER_X,CHAT_AREA_BOTTOM-4,CHAT_SCROLL_CENTER_X-CHAT_SCROLL_ARROW_HALF,CHAT_AREA_BOTTOM-10,CHAT_SCROLL_CENTER_X+CHAT_SCROLL_ARROW_HALF,CHAT_AREA_BOTTOM-10,color);
    if(chatScrollbarLastThumbY>=0)frame.fillRoundRect(CHAT_SCROLL_TRACK_X,chatScrollbarLastThumbY,CHAT_SCROLL_TRACK_W,chatScrollbarLastThumbH,3,TFT_WHITE);
  }
  Serial.printf("[UI-SCREENSHOT] source=render_buffers width=240 height=240 chat_top=%d scroll_px=%ld\n",CHAT_AREA_TOP,long(chatScrollPixels));
  voiceExportSprite(frame);frame.deleteSprite();
}
static void voiceMathScreenshot(unsigned page,unsigned row=0) {
  if(!voiceChatScreenshotReady()){Serial.println("[MATH-TEST] idle home without draft photos required");return;}
  const char* sample=voiceMathSample(page);if(!sample){Serial.println("[MATH-TEST] pages: 1-9; optional scroll row");return;}
  auto saved=std::move(chatHistory);const size_t bytes=chatHistoryBytes;
  const auto pixels=chatScrollPixels,target=chatScrollTargetPixels;const auto scroll=chatScroll;const bool autoScroll=chatAutoScrollBottom;
  chatHistory.clear();ChatMsg message;message.role="assistant";message.content=sample;if(page==8){message.thinkingTimed=message.thinkingComplete=true;message.thinkingMs=12000;}chatHistory.push_back(std::move(message));
  markChatLinesDirty();chatAutoScrollBottom=false;setChatScrollRow(row);
  Serial.printf("[MATH-TEST] page=%u row=%u source=render_buffers no_network=1 no_history_write=1\n",page,row);
  voiceExportChatScreen();
  chatHistory=std::move(saved);chatHistoryBytes=bytes;chatScroll=scroll;chatScrollPixels=pixels;chatScrollTargetPixels=target;chatAutoScrollBottom=autoScroll;
  markChatLinesDirty();drawPage1();
}
static void voiceTestChat(const String& question,bool screenshot=false) {
  if(!voiceChatScreenshotReady() || composingQuestion || pendingQuestion.length()){Serial.println("[TESTCHAT] idle home without draft required");return;}
  auto saved=std::move(chatHistory);const size_t bytes=chatHistoryBytes,prefix=chatHistoryPrefixBytes;
  const bool incomplete=chatHistoryReadIncomplete;const AppState state=appState;
  const String error=lastError,failed=lastFailedQuestion,transient=transientSearchResult,sources=voiceLastSearchSources,summary=chatRollingSummary;
  const bool failedSearch=lastFailedUseSearch;const size_t summaryMessages=chatRollingSummaryMsgs;
  auto evidence=std::move(chatRollingWebEvidence),trace=std::move(chatRollingWebTrace);
  const auto pixels=chatScrollPixels,target=chatScrollTargetPixels;const auto scroll=chatScroll;const bool autoScroll=chatAutoScrollBottom;
  chatHistory.clear();chatHistoryBytes=chatHistoryPrefixBytes=0;chatHistoryReadIncomplete=false;
  chatRollingSummary="";chatRollingSummaryMsgs=0;voiceValidationMode=true;markChatLinesDirty();
  const auto ticks=voiceBusyHeaderTicks;const uint32_t started=millis();
  const bool ok=askAndStoreChat(question,false);
  if(ok && screenshot){chatAutoScrollBottom=false;setChatScrollRow(0);voiceExportChatScreen();}
  Serial.printf("[TESTCHAT] ok=%d no_history_write=1 elapsed_ms=%lu header_ticks=%lu internal=%u largest=%u\n",ok,(unsigned long)(millis()-started),(unsigned long)(voiceBusyHeaderTicks-ticks),unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)),unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)));
  voiceValidationMode=false;chatHistory=std::move(saved);chatHistoryBytes=bytes;chatHistoryPrefixBytes=prefix;chatHistoryReadIncomplete=incomplete;
  chatRollingSummary=summary;chatRollingSummaryMsgs=summaryMessages;chatRollingWebEvidence=std::move(evidence);chatRollingWebTrace=std::move(trace);
  appState=state;lastError=error;lastFailedQuestion=failed;lastFailedUseSearch=failedSearch;transientSearchResult=transient;voiceLastSearchSources=sources;
  chatScroll=scroll;chatScrollPixels=pixels;chatScrollTargetPixels=target;chatAutoScrollBottom=autoScroll;
  markChatLinesDirty();drawPage1();
}
