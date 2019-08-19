#include"FS.h"
#include<ArduinoJson.h>
#include<OneButton.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>

//const char* mqttServer = "io.adafruit.com";
const int mqttPort = 1883;
//const char* mqttUser = "mahadevan1998";
//const char* mqttPassword = "f77f4c5a4daa44d3a1b58ea797c7fba0";
 

const int reset_pin = D5;
float press_time = 0.0 ;
const int relay1 = D1;
const int switch_pin = D7; 
int switch_state = LOW;


//Webserver declaration
ESP8266WebServer server(80);
OneButton button(reset_pin,true);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  
  if ((char)payload[0] == '1') {
    digitalWrite(relay1, !digitalRead(relay1));   
      } else if((char)payload[0]=='0') {
    digitalWrite(relay1, !digitalRead(relay1)); // Turn the LED off by making the voltage HIGH
  }

}

WiFiClient espClient;
PubSubClient client(espClient);

//WiFi Setup variables
char ssidc[30];//Stores the router name 
char passwordc[30];//Stores the password
const char *ssid = "";
const char *pass = ""; 

const char* username = "";
const char* key = "";
const char* cloud = "";
 


IPAddress wap_ip(192,168,1,1);
IPAddress wap_gateway(192,168,1,254);
IPAddress subnet(255, 255, 255, 0);
//IPAddress ip(192,168,0,105); 

const char *ssid2 = "Homatico Solutions";
//const char *password2 = "12345678";

const char HTTP_200[]   = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char HTTP_HEAD[]  = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[]  = "<style>div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align:center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}</style>";
const char HTTP_SCRIPT[]  = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[]  = "</head><body><div style='text-align: left; display: inline-block;'>";
const char HTTP_WIFI_OPTIONS[]  = "<form action=\"/wifi\" method=\"post\"><button>Configure WiFi</button></form>";
const char HTTP_WIFI_SAVE[]  = "<form action=\"/wifisave\" method=\"post\"><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
//const char HTTP_WIFI_SAVE[]  = "<form method='post' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/>";
const char HTTP_PORTAL_OPTIONS[] = "<form action=\"/relay\" method=\"post\"><button>Relay</button></form><br/><form action=\"/resetwifi\" method=\"get\"><button>Reset Wifi</button></form><br/><form action=\"/mqtt\" method=\"get\"><button>Mqtt Configure</button></form>";
const char HTTP_MQTT_SAVE[]  = "<form action=\"/mqttsave\" method=\"post\"><input id='u' name='u' length=32 placeholder='SSID'><br/><input id='k' name='k' length=64 type='password' placeholder='password'><br/><input id='c' name='c' length=64 placeholder='Server'>";
const char HTTP_CALL_ACTION[] = "<form action=\"/callAction\" method=\"get\"><input id='d' name='deviceID' length=32 placeholder='DEVICE ID'><br/><input id='a' name='name' length=64 placeholder='NAME'><br/>";
const char HTTP_FORM_END[]  = "<br/><button type='submit'>save</button></form>";
const char HTTP_END[]  = "</div></body></html>";


void WAP(){
   
  WiFi.softAPConfig(wap_ip, wap_gateway, subnet);
  Serial.print("Setting soft-AP ... ");
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  WiFi.softAP(ssid2);
 
  
}   


void handleRoot() {
  String page;

 if(WiFi.status()!=WL_CONNECTED){
  page = HTTP_HEAD;
  page += HTTP_HEAD;
  page += HTTP_STYLE;
  page += HTTP_SCRIPT;
  page += HTTP_HEAD_END;
  page += HTTP_WIFI_OPTIONS;
  page += HTTP_END;
  server.send(200,"text/html",page);
  
 }
 else{
  page = HTTP_HEAD;
  page += HTTP_HEAD;
  page += HTTP_STYLE;
  page += HTTP_SCRIPT;
  page += HTTP_HEAD_END;
  page += HTTP_PORTAL_OPTIONS;
  page += HTTP_END;
  
 server.send(200, "text/html",page);
  
}
}
void handleWifi()
{
  String page = HTTP_HEAD;
  page += HTTP_HEAD;
  page += HTTP_STYLE;
  page += HTTP_SCRIPT;
  page += HTTP_HEAD_END;
  page += HTTP_WIFI_SAVE;
  page += HTTP_FORM_END;
  page += HTTP_END;

  server.send(200,"text/html",page);
}




void handleWifiSave()
{  
   String s , p;
   s = server.arg("s");
   p = server.arg("p");
   s.toCharArray(ssidc,30);
   p.toCharArray(passwordc,30);


  DynamicJsonBuffer jsonBuffer;
  JsonObject& wifi = jsonBuffer.createObject();
  wifi["ssid"] = server.arg("s");
  wifi["password"] = server.arg("p");
 
 
 File wifiFile = SPIFFS.open("/wifiConfig.json", "w");
 wifi.printTo(wifiFile);
 wifiFile.close();
 
 Serial.println("Saved");
 delay(2000);
 
   WiFi.begin(ssidc, passwordc);
    
    while (WiFi.status() != WL_CONNECTED) 
    {
    delay(500);
    Serial.print("Wifi Status:");
    Serial.println(WiFi.status());
    }
    
    
   IPAddress IP = WiFi.localIP();
   //server.send(200,"text/html","<p>Connected to network</p>");
   String addr= String(IP[0])+String(".")+String(IP[1])+String(".")+String(IP[2])+String(".")+String(IP[3]);
   server.send(200,"text/html","<p><a href=http://"+addr+">"+addr+"</a></p>");
   Serial.println(WiFi.localIP());

  
   
   delay(5000);
   
   WiFi.mode(WIFI_STA);
   WiFi.softAPdisconnect(true);
   
   }


   bool loadWifi() {

  if(SPIFFS.exists("/wifiConfig.json"))
  {
  
  File wifiFile = SPIFFS.open("/wifiConfig.json", "r");

    size_t size = wifiFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  
 wifiFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& wifi = jsonBuffer.parseObject(buf.get());

  if (!wifi.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

ssid = wifi["ssid"];
pass = wifi["password"];
 
 Serial.println("Loaded ssid: ");
  Serial.println(ssid);
  Serial.print("Loaded password: ");
  Serial.println(pass);

  wifiFile.close();
  
  return true;

  }

  else
  {
    return false;
  }
}


void resetWifi()
{ server.send(200,"text/html","<h4> Wifi settings reset<br/><p>Device is restarting in few seconds, then configure wifi</p></h4>");  
  delay(3000);
  if(SPIFFS.exists("/wifiConfig.json"))
  {
  SPIFFS.remove("/wifiConfig.json");
  Serial.println("deleted wifi config");
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  WAP();
  }  



void toggle()
{
  digitalWrite(relay1,!digitalRead(relay1));
  relay();    
}

void callAction(){
  if((server.arg("deviceID") == "relay1")&& (server.arg("name")== "turnOn")){
    On();
    String ON =
      "{\"device\" : \" relay 1\" , \" status\" : \"on\"}";
       server.send(200,"text/html",ON);
    }

    else if((server.arg("deviceID") == "relay1")&& (server.arg("name")== "turnOff"))
    {
      Off();
      String OFF =
      "{\"device\" : \"relay 1\" , \"status\" : \"off\"}";
       server.send(200,"text/html",OFF);
    }
}

void Status()
{
  
              
             if(digitalRead(relay1)== HIGH){
              server.send(200,"text/html","<p> LIGHT IS ON </p>");
              }

              else if(digitalRead(relay1)== LOW){
              server.send(200,"text/html","<p> LIGHT IS OFF </p>");
              }
}

void relay()
{ 
  String page;
  page = HTTP_HEAD;
  page += HTTP_HEAD;
  page += HTTP_STYLE;
  page += HTTP_SCRIPT;
  page += HTTP_HEAD_END;
  page += "<p><a href=\"/toggle\"><button class=\"button\">ON/OFF</button></a><br><a href=\"/status\">Check status</a></p>";
  page += HTTP_END;
  server.send(200,"text/html",page);

}  

void On(){
  digitalWrite(relay1,HIGH);
}

void Off()
{
  digitalWrite(relay1,LOW);
}

void mqtt()
{
  
  String page = HTTP_HEAD;
  page += HTTP_HEAD;
  page += HTTP_STYLE;
  page += HTTP_SCRIPT;
  page += HTTP_HEAD_END;
  page += HTTP_MQTT_SAVE;
  page += HTTP_FORM_END;
  page += HTTP_END;

  server.send(200,"text/html",page);
}

void mqttSave()
{
 DynamicJsonBuffer jsonBuffer;
 JsonObject& json = jsonBuffer.createObject();
 json["username"] = server.arg("u");
 json["key"] = server.arg("k");
 json["server"] = server.arg("c");
 
 File configFile = SPIFFS.open("/config.json", "w");
 json.printTo(configFile);
 configFile.close();
 
 server.send(200,"text/html","Restarting");
 delay(2000);
 ESP.restart();
 
 }

bool loadConfig() {

  if(SPIFFS.exists("/config.json"))
  {
  
  File configFile = SPIFFS.open("/config.json", "r");

    size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  
  configFile.readBytes(buf.get(), size);

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  configFile.close();

  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

 username = json["username"];
 key = json["key"];
 cloud = json["server"];

 
  Serial.println("Loaded username: ");
  Serial.println(username);
  Serial.print("Loaded password: ");
  Serial.println(key);
  Serial.println("Loaded servername:  ");
  Serial.println(cloud);
  
       Serial.println(username);
       Serial.println(key);
       Serial.println(cloud);
     
       client.setServer(cloud, mqttPort);
       client.setCallback(callback);
       if( client.connect("ESP8266Client", username, key)){ 
      Serial.println(("MQTT Connected"));
      client.publish("mahadevan1998/feeds/sonoff","HELLO");
      client.subscribe("mahadevan1998/feeds/onoff");
      delay(1000);
     }
  else
  {
    Serial.println("could not connect");
    }
    
 client.subscribe("mahadevan1998/feeds/onoff");
  
 configFile.close();
 return true;

  }

  else
  {
    return false;
  }
}




void setup()
    {
      Serial.begin(115200);
      SPIFFS.begin();
      button.attachDoubleClick(resetWifi);

     pinMode (reset_pin, INPUT_PULLUP);
     pinMode(relay1, OUTPUT);
     pinMode(switch_pin, INPUT);
     switch_state=digitalRead(switch_pin);
     digitalWrite(relay1, LOW);
     WiFi.disconnect(true);

    

     if(!loadWifi())
     {
      Serial.println("WiFi not loaded");
      WAP();
     }

     else
     {      // WiFi.config(ip);
             WiFi.begin(ssid, pass);
    
            while (WiFi.status() != WL_CONNECTED) 
    {
            delay(500);
            Serial.print("Wifi Status:");
            Serial.println(WiFi.status());
            }
       
           Serial.println(WiFi.localIP());

              }

     /*  if(!loadConfig())
       {
        Serial.println("no mqtt present");
       }
       else
       {
       Serial.println("conected");
       }*/

       Serial.println(client.state());
    

      
    Serial.print("username: ");
    Serial.println(username);
    Serial.print("key: ");
    Serial.println(key);
    Serial.print("server: ");
    Serial.println(cloud);

    
     server.on("/", handleRoot);
     server.on("/wifi",handleWifi);
     server.on("/wifisave",handleWifiSave);
     server.on("/relay",relay);
     server.on("/resetwifi",resetWifi);
     server.on("/toggle", toggle);
     server.on("/callAction", callAction);
     server.on("/on",On);
     server.on("/off",Off);
     server.on("/status", Status);
     server.on("/mqtt", mqtt);
     server.on("/mqttsave", mqttSave);
  
     server.begin();



        }  
    
    void loop()
        {
  if(digitalRead(switch_pin)!= switch_state)
            {
            digitalWrite(relay1,!digitalRead(relay1));
            switch_state = digitalRead(switch_pin);
            }

button.tick();
server.handleClient();
if(!client.connected())
{
 if(!loadConfig())
       {
        Serial.println("no mqtt present");
       }
       else
       {
       Serial.println("conected");
       client.subscribe("mahadevan1998/feeds/onoff");
       }

}

else
{
  client.subscribe("mahadevan1998/feeds/onoff");
}



//Serial.println(client.state());
/*if(client.connected()){
  
Serial.println("yaaas");
  client.subscribe("mahadevan1998/feeds/onoff"); 
}*/
client.loop();

 }
