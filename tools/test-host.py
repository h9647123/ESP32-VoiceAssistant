"""Compile tests against the current firmware source, without hardware or network."""
from pathlib import Path
import argparse,json,os,shutil,subprocess
from source_blocks import endblock,get,put

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler',default=os.environ.get('CXX','c++'),help='C++17 compiler executable; zig is supported')
parser.add_argument('--arduinojson',type=Path,required=True,help='ArduinoJson src directory')
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
out=root/'validation/host-tests';out.mkdir(parents=True,exist_ok=True)
if not (args.arduinojson/'ArduinoJson.h').is_file():parser.error('--arduinojson must contain ArduinoJson.h')
compiler=shutil.which(args.compiler) or (args.compiler if Path(args.compiler).is_file() else None)
if not compiler:parser.error('C++ compiler not found')
command=[str(compiler)]
env=os.environ.copy()
if Path(compiler).stem.lower()=='zig':
    command+=['c++']
    for key,folder in [('TEMP','tmp'),('TMP','tmp'),('ZIG_GLOBAL_CACHE_DIR','global-cache'),('ZIG_LOCAL_CACHE_DIR','local-cache')]:
        path=out/folder;path.mkdir(exist_ok=True);env[key]=str(path)
source=(root/'VoiceAssistant/VoiceAssistant.ino').read_text(encoding='utf-8')
think='static size_t thinkTagSuffixLength('+source.split('static size_t thinkTagSuffixLength(',1)[1].split('static bool streamChunkSuggestsBreak(',1)[0]
(out/'thinking_under_test.h').write_text(think,encoding='utf-8')
start=source.index('class LlmSseSink : public Stream {')
stop=endblock(source,source.index('{',start))+1
(out/'sse_under_test.h').write_text(source[start:stop],encoding='utf-8')
tls=(root/'VoiceAssistant/iat_dns_fallback.h').read_text(encoding='utf-8')
(out/'tls_retry_under_test.h').write_text(get(tls,'iatConnectWithDnsFallback'),encoding='utf-8')
menu=(root/'tools/host-tests/test_menu_draft.cpp').read_text(encoding='utf-8')
for name in ['menuGroupOf','menuGroupName','menuItemVisible','menuItemCount','menuItemAt','menuSelectItem','menuReturnRoot','commitAsrDraftSegment']:
    menu=put(menu,name,get(source,name))
(out/'menu_draft_current.cpp').write_text(menu,encoding='utf-8')
results={}
for name,cpp in [('reasoning',root/'tools/host-tests/test_reasoning.cpp'),('tls_retry',root/'tools/host-tests/test_tls_retry.cpp'),('menu_draft',out/'menu_draft_current.cpp')]:
    exe=out/(name+('.exe' if os.name=='nt' else ''))
    includes=[root/'tools/host-tests',out,root/'VoiceAssistant',args.arduinojson]
    build=command+['-std=c++17']+sum((['-I',str(p)] for p in includes),[])+[str(cpp),'-o',str(exe)]
    subprocess.run(build,env=env,check=True)
    subprocess.run([str(exe)],env=env,check=True)
    results[name]='PASS'
report={'tests':results,'uses_current_firmware_source':True,'hardware_tested':False,'network_used':False,'flashed':False}
(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
