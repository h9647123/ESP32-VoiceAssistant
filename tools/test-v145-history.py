from host_toolchain import host_compiler, arduinojson_include
from pathlib import Path
import re,sys,subprocess,os
sys.path.insert(0,'tools');from source_blocks import get
r=Path('.').resolve();c=r;out=r/'validation/20261114-v145/history';out.mkdir(parents=True,exist_ok=True)
s=(r/'VoiceAssistant/VoiceAssistant.ino').read_text(encoding='utf-8')
names=['trimChatHistoryToLimit','loadChatHistory','saveChatHistory','addChatMessage']
(out/'web_history_under_test.h').write_text('\n\n'.join(get(s,n) for n in names),encoding='utf-8')
f=(c/'tools/host-tests-v145/test_history_schema.cpp').read_text(encoding='utf-8');f=re.sub(r'\bcam([A-Z]\w*)',lambda m:'voice'+m[1],f);f=re.sub(r'\bCam([A-Z]\w*)',lambda m:'Voice'+m[1],f).replace('cam_strict_json.h','voice_strict_json.h')
a=f.index(' assert(voiceWebMessageContext(');b=f.index(' chatHistory.back().thinkingTimed',a);f=f[:a]+f[b:]
f=f.replace('cam_json_config.h','voice_json_config.h').replace('cam_history_io.h','voice_history_io.h').replace('cam_llm_policy.h','voice_json_config.h')
f=f.replace('#include \"voice_llm_policy.h\"','').replace('static const char* FILE_READ', 'static String stripDisplayMarkers(const String& s){return s;}\nstatic const char* FILE_READ')
f='\n'.join(line for line in f.split('\n') if 'assert(buildLlmRequestBodyBudget' not in line and 'assert(voiceWebMessageContext' not in line)
f=f.replace('research metadata round-trip; prompt inclusion/exclusion','metadata round-trip')
f=f.replace('struct Store {','static bool denyWrites=false;\nstruct Store {').replace('File open(const String& p,const char* mode){','File open(const String& p,const char* mode){if(denyWrites && mode==FILE_WRITE)return {};')
f=f.replace('static bool loadLegacyNvsChatHistory(){return false;}','static bool legacyAvailable=false;static unsigned legacyCleared=0;static bool loadLegacyNvsChatHistory(){if(!legacyAvailable)return false;chatHistory={{"user","legacy-only"}};chatHistoryBytes=30;return true;}')
f=f.replace('static void clearLegacyNvsChatHistory(){}','static void clearLegacyNvsChatHistory(){++legacyCleared;legacyAvailable=false;}')
f=f.replace(' std::cout<<"PASS: actual history', ' files.clear();legacyAvailable=true;denyWrites=true;loadChatHistory();assert(legacyCleared==0&&legacyAvailable);denyWrites=false;loadChatHistory();assert(legacyCleared==1&&!legacyAvailable&&files.count(chatHistoryPath()));\n std::cout<<"PASS: legacy NVS retained on failed migration save, cleared only after commit\\n";\n std::cout<<"PASS: actual history')
(out/'test.cpp').write_text(f,encoding='utf-8');host=r/'.build-host';host.mkdir(exist_ok=True);env=os.environ.copy()
for k,v in [('TEMP','tmp'),('TMP','tmp'),('ZIG_GLOBAL_CACHE_DIR','global-cache'),('ZIG_LOCAL_CACHE_DIR','local-cache')]:(host/v).mkdir(parents=True,exist_ok=True);env[k]=str(host/v)
cmd=[*host_compiler(),'-std=c++17','-I',str(r/'tools/host-tests-v145'),'-I',str(r/'VoiceAssistant'),'-I',str(out),'-I',arduinojson_include(),str(out/'test.cpp'),'-o',str(out/'test.exe')]
p=subprocess.run(cmd,env=env,capture_output=True,text=True);(out/'compile.log').write_text(p.stdout+p.stderr,encoding='utf-8');print(p.stdout+p.stderr);p.check_returncode()
p=subprocess.run([str(out/'test.exe')],capture_output=True,text=True);(out/'tests.log').write_text(p.stdout+p.stderr,encoding='utf-8');print(p.stdout+p.stderr);p.check_returncode()
