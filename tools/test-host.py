"""Run current original firmware tests offline; never opens serial or network."""
from pathlib import Path
import argparse,os,subprocess,sys,json
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--compiler',default=os.environ.get('ZIG_EXE') or os.environ.get('CXX','c++'));p.add_argument('--arduinojson',type=Path,required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1];env=os.environ.copy();env['ARDUINOJSON_INCLUDE']=str(a.arduinojson.resolve())
if Path(a.compiler).stem.lower()=='zig':env['ZIG_EXE']=a.compiler
else:env['CXX']=a.compiler
if not (a.arduinojson/'ArduinoJson.h').is_file():p.error('--arduinojson must point to ArduinoJson src')
tests=['history','render','scroll','stream','draft','draft-layout','meter'];results={}
for name in tests:
 subprocess.run([sys.executable,str(r/'tools'/('test-v145-'+name+'.py'))],cwd=r,env=env,check=True);results[name]='PASS'
out=r/'validation/host-tests';out.mkdir(parents=True,exist_ok=True);(out/'results.json').write_text(json.dumps(dict(tests=results,uses_current_firmware_source=True,hardware_tested=False,network_used=False,flashed=False),indent=2),encoding='utf-8');print('PASS: 7 current original-firmware offline test groups')
