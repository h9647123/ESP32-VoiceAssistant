#include "Arduino.h"
#include <vector>
#include <cassert>
static constexpr uint8_t MENU_ITEM_MAX_COUNT=20,MENU_GROUP_COUNT=5;
enum class UiLanguage{Chinese,English};
struct Config{bool enableThinking=true;UiLanguage uiLanguage=UiLanguage::Chinese;}cfg;
enum MenuItem : uint8_t {
  MenuBrightness,
  MenuSleep,
  MenuSession,
  MenuProvider,
  MenuWifi,
  MenuModel,
  MenuPrompt,
  MenuEnableThinking,
  MenuThinkingDisplay,
  MenuAsrProvider,
  MenuAsr,
  MenuClock,
  MenuDht,
  MenuSearch,
  MenuStyle,
  MenuLanguage,
  MenuClearChat,
  MenuServiceCheck,
  MenuWebUi,
  MenuInfo
};
static const MenuItem MENU_ORDER[MENU_ITEM_MAX_COUNT] = {
    MenuBrightness, MenuSleep,
    MenuClock, MenuDht, MenuStyle, MenuLanguage,
    MenuSession, MenuClearChat,
    MenuProvider, MenuModel, MenuPrompt, MenuEnableThinking,
    MenuThinkingDisplay, MenuSearch,
    MenuWifi,
    MenuAsrProvider, MenuAsr,
    MenuServiceCheck, MenuWebUi, MenuInfo};
static int8_t menuGroup=-1,menuRootIndex=0;
static uint8_t menuIndex=0,menuScroll=0;static bool menuEditing=false,clearChatConfirm=false;
static int8_t menuGroupOf(MenuItem item) {
  if(item==MenuWifi || item==MenuAsrProvider || item==MenuAsr)return 0;
  if(item==MenuBrightness || item==MenuSleep || item==MenuClock || item==MenuDht || item==MenuStyle || item==MenuLanguage)return 1;
  if(item==MenuSession || item==MenuPrompt || item==MenuClearChat)return 2;
  if(item==MenuProvider || item==MenuModel || item==MenuEnableThinking || item==MenuThinkingDisplay || item==MenuSearch)return 3;
  return 4;
}
static const char* menuGroupName(int8_t group) {
  static const char* zh[]={"设置","UI","聊天","AI","系统"};
  static const char* en[]={"Settings","UI","Chat","AI","System"};
  return group>=0 && group<MENU_GROUP_COUNT ? (cfg.uiLanguage==UiLanguage::Chinese?zh[group]:en[group]):"MENU";
}
static bool menuItemVisible(MenuItem item) {
  return (cfg.enableThinking || item != MenuThinkingDisplay) &&
      (menuGroup < 0 || menuGroupOf(item) == menuGroup);
}
static uint8_t menuItemCount() {
  if (menuGroup == -1) return MENU_GROUP_COUNT;
  uint8_t count = 0;
  for (const MenuItem item : MENU_ORDER) if (menuItemVisible(item)) ++count;
  return count;
}
static MenuItem menuItemAt(uint8_t visibleIndex) {
  uint8_t visible = 0;
  for (const MenuItem item : MENU_ORDER) {
    if (menuGroup == -1) {
      if (menuGroupOf(item) == visibleIndex) return item;
    } else if (menuItemVisible(item) && visible++ == visibleIndex) return item;
  }
  return MENU_ORDER[0];
}
static void menuSelectItem(MenuItem item) {
  menuGroup = menuGroupOf(item); menuRootIndex = menuGroup; menuIndex = 0;
  while (menuIndex+1 < menuItemCount() && menuItemAt(menuIndex) != item) ++menuIndex;
  menuScroll = 0; menuEditing = false; clearChatConfirm = false;
}
static void menuReturnRoot() {
  if (menuGroup >= 0) menuRootIndex = menuGroup;
  menuGroup = -1; menuIndex = menuRootIndex; menuScroll = 0;
  menuEditing = false; clearChatConfirm = false;
}
static bool asrAppendMode=false,composingQuestion=false;
static std::vector<String> pendingSegments;static String pendingQuestion,asrBestTranscript,asrStableTranscript,asrPartialTranscript;
static void rebuildPendingQuestionFromSegments(){pendingQuestion="";for(const auto& value:pendingSegments){if(pendingQuestion.length())pendingQuestion+=" ";pendingQuestion+=value;}}
static void commitAsrDraftSegment(const String& segment) {
  if (!asrAppendMode && !composingQuestion) pendingSegments.clear();
  if (segment.length()) pendingSegments.push_back(segment);
  rebuildPendingQuestionFromSegments();
  composingQuestion = true;
  asrBestTranscript = asrStableTranscript = asrPartialTranscript = "";
}
int main(){
 assert(menuItemCount()==5);
 for(unsigned i=0;i<5;++i)assert(menuGroupOf(menuItemAt(i))==int(i));
 for(MenuItem item:MENU_ORDER){menuSelectItem(item);assert(menuItemAt(menuIndex)==item);assert(menuGroup==menuGroupOf(item));menuReturnRoot();assert(menuIndex==menuGroupOf(item));}
 menuSelectItem(MenuDht);assert(menuGroup==1);menuReturnRoot();
 cfg.enableThinking=false;menuGroup=3;for(unsigned i=0;i<menuItemCount();++i)assert(menuItemAt(i)!=MenuThinkingDisplay);
 menuReturnRoot();assert(menuItemCount()==5);
 pendingSegments={"old draft"};asrBestTranscript="first";asrStableTranscript="first";asrPartialTranscript="first";
 commitAsrDraftSegment("first");assert(pendingSegments.size()==1&&pendingQuestion=="first");assert(asrBestTranscript.empty()&&asrStableTranscript.empty()&&asrPartialTranscript.empty());
 asrAppendMode=true;asrBestTranscript="second";commitAsrDraftSegment("second");assert(pendingSegments.size()==2&&pendingQuestion=="first second"&&asrBestTranscript.empty());
}
