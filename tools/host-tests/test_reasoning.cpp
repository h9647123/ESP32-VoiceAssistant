#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
#define ARDUINOJSON_ENABLE_PROGMEM 0
#include "Arduino.h"
#include <ArduinoJson.h>
#include <cassert>
#include <iostream>
#include <vector>
#include "asr_capture_limits.h"
#include "service_probe_policy.h"
#include "latex_display.h"
using PsramString=String;
struct ChatMsg{String role,content;};
static std::vector<ChatMsg> chatHistory;
struct Config{bool enableThinking=true,showThinking=true,clockMode=false;}cfg;
static String lastError;
static uint32_t millis(){static uint32_t now=0;return ++now;}
static String uiText(const char* cn,const char*,const char*){return cn;}
struct SerialMock {template<class... Args>void printf(const char*,Args...){} void println(const char*){}} Serial;
struct EspMock {int getFreeHeap(){return 100000;}} ESP;
static constexpr size_t LLM_FALLBACK_CAPTURE_BYTES=32768,LLM_SSE_LINE_MAX_BYTES=32768,LLM_VISIBLE_REPLY_MAX_BYTES=49152;
static constexpr char RICH_THINKING_TOGGLE=3;
enum class UiPage {Page1};static UiPage uiPage=UiPage::Page1;
static bool screenSleeping=false,chatLinesDirty=true,chatAutoScrollBottom=false;
static std::vector<String> chatLines;static std::vector<uint8_t> chatLineRoles;
static size_t maxTailBytes=0;
static void rebuildChatLines(){chatLines={"user","","assistant",""};chatLineRoles={1,0,2,0};}
static void appendChatMessageLines(const ChatMsg& msg,std::vector<String>& lines,std::vector<uint8_t>& roles){maxTailBytes=max(maxTailBytes,msg.content.length());lines.push_back(msg.content);roles.push_back(2);lines.push_back("");roles.push_back(0);}
static void tickLlmWaitingAnimation(bool){}
static void drawPage1TopLine(){}
static void drawChatWindow(bool){}
static void drawPage1Footer(const String&){}
static void updateChatMessageContent(size_t i,const String& content,bool=true){if(i<chatHistory.size())chatHistory[i].content=content;}
static String truncateUtf8ToBytes(const String& text,size_t count){count=min(count,text.length());while(count<text.length()&&count&&(uint8_t(text[count])&0xc0)==0x80)--count;return text.substring(0,count);}
#include "thinking_under_test.h"
#include "sse_under_test.h"
static void feed(LlmSseSink& sink,const char* key,const String& text){JsonDocument d;d["choices"][0]["delta"][key]=text;String line;serializeJson(d,line);line="data: "+line+"\n\n";sink.write((const uint8_t*)line.data(),line.size());}
int main(){
 static_assert(voiceAsrRecordLimitMs(true,12)==60000,"CUSTOM 60s");
 static_assert(voiceAsrRecordLimitMs(false,12)==12000,"legacy unchanged");
 static_assert(voiceMicCapacitySamples(16000,60000,1800)==1020800,"capture plus tail");
 static_assert(voiceAsrUploadLimitBytes(16000,true,12,1800)==2041600,"complete upload");
 assert(serviceNeedsFullProbe(false,true,false));assert(!serviceNeedsFullProbe(false,true,true));
 auto latex=[](const std::string& input){std::string result;for(size_t i=0;i<input.size();)if(!latexDisplayToken(input.data(),input.size(),i,result))result+=input[i++];return result;};
 assert(latex(R"(\[\boxed{a\in(-\infty,-1]}\])")=="a∈(-∞,-1]");
 assert(latex(R"(\boxed{\sqrt{x^{2}+1}})")=="√(x^{2}+1)");

 chatHistory={{"user","long reasoning"},{"assistant",""}};chatLinesDirty=true;
 {LlmSseSink sink(1);for(int i=0;i<1500;++i){feed(sink,"reasoning_content",String(std::string(1024,'r')));assert(sink.answer().size()<=8192);assert(!sink.failed());}
  feed(sink,"content","最终答案：a∈(-∞,-1]。\n");const char* done="data: {\"choices\":[{\"finish_reason\":\"stop\"}]}\n\n";sink.write((const uint8_t*)done,strlen(done));sink.finish();assert(sink.done());assert(!sink.truncated());assert(!sink.failed());assert(sink.answer().find("最终答案")!=String::npos);assert(maxTailBytes<=771);}
 {cfg.showThinking=false;LlmSseSink sink(1);for(int i=0;i<100;++i)feed(sink,"reasoning_content",String(std::string(1024,'r')));assert(sink.answer().empty());feed(sink,"content","Final");sink.finish();assert(sink.answer()=="Final");}
 {cfg.showThinking=true;LlmSseSink sink(1);for(int i=0;i<800&&!sink.done();++i)feed(sink,"content",String(std::string(128,'x')));assert(sink.done());assert(sink.outputLimited());assert(sink.answer().length()<=49152);}
 {LlmSseSink sink(1);feed(sink,"content","<think>");for(int i=0;i<1000;++i){feed(sink,"content",String(std::string(1024,'t')));assert(sink.answer().length()<=8192);assert(!sink.truncated());}
  feed(sink,"content","</think>Final answer after inline reasoning.");sink.finish();assert(!sink.truncated());assert(sink.answer().find("Final answer after inline reasoning.")!=String::npos);}
 std::cout<<"original_long_reasoning_parser_passed\n";
}
