
#include "Arduino.h"
#include <atomic>
#include <cassert>
#include <cstdio>
struct IPAddress{
 IPAddress(){} IPAddress(int,int,int,int){}
 bool fromString(const String& text){return !text.empty()&&text[0]>='0'&&text[0]<='9';}
 String toString()const{return "203.0.113.10";}
};
struct IatEndpoint{String host="asr.example.com";uint16_t port=16457;bool tls=true;};
static constexpr const char* VOICE_TLS_CA_PEM=nullptr;
struct Log{template<class...T>void printf(const char*,T...) {}}Serial;
static int configured=0,queries=0;
static bool resolveOk=true;
class NetworkClient{
public:virtual ~NetworkClient(){};virtual int connect(const char*,uint16_t,uint32_t)=0;virtual void stop()=0;
};
class WiFiClientSecure:public NetworkClient{
public:bool trustAttached=true,normalSuccess=false;int failures=0,retries=0;
 int connect(const char*,uint16_t,uint32_t)override{if(normalSuccess)return 1;stop();return 0;}
 void stop()override{trustAttached=false;}
 int lastError(char* out,size_t size){snprintf(out,size,"fixture failure");return -1;}
 int connect(IPAddress,uint16_t,const char* host,const char*,const char*,const char*){
  ++retries;assert(trustAttached);assert(String(host)=="asr.example.com");
  if(failures){--failures;stop();return 0;}return 1;
 }
};
static bool voiceConfigureTls(WiFiClientSecure& client){++configured;client.trustAttached=true;return true;}
static bool iatQueryAlternateDns(const String&,const IPAddress&,IPAddress&,std::atomic<bool>*){++queries;return resolveOk;}

#include "tls_retry_under_test.h"

int main(){
 IatEndpoint endpoint;
 {WiFiClientSecure client;client.normalSuccess=true;
  assert(iatConnectWithDnsFallback(client,client,endpoint,8000,true));assert(queries==0&&configured==0);}
 {WiFiClientSecure client;assert(iatConnectWithDnsFallback(client,client,endpoint,8000,true));assert(client.retries==1&&configured==1&&queries==1);}
 configured=queries=0;
 {WiFiClientSecure client;client.failures=1;assert(iatConnectWithDnsFallback(client,client,endpoint,8000,true));assert(client.retries==2&&configured==2&&queries==2);}
 configured=queries=0;
 {WiFiClientSecure client;assert(!iatConnectWithDnsFallback(client,client,endpoint,8000,false));assert(queries==0&&configured==0);}
 {WiFiClientSecure client;std::atomic<bool> cancel{true};assert(!iatConnectWithDnsFallback(client,client,endpoint,8000,true,&cancel));assert(queries==0);}
 for(const char* host:{"192.168.1.10","gateway.local","gateway.lan","gateway.internal"}){
  WiFiClientSecure client;endpoint.host=host;assert(!iatConnectWithDnsFallback(client,client,endpoint,8000,true));assert(queries==0);}
 endpoint.host="asr.example.com";endpoint.tls=false;
 {WiFiClientSecure client;assert(!iatConnectWithDnsFallback(client,client,endpoint,8000,true));assert(queries==0);}
}
