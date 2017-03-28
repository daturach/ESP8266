// idea from https://tzapu.com/minimalist-battery-powered-esp8266-wifi-temperature-logger/
//sensitive data defined in config.h file
// When an impulse is triggered by the schmitt trigger, the Wemos board is resetted.
// This will initialize the Wifi connection and one impulse will be sent to emoncms.org

#include <ESP8266WiFi.h>

//const char* ssid     = "my_id"; //defined in config.h
//const char* password = "my_pass"; //defined in config.h

//#include <WiFiClient.h>

//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
//#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager


//#include <SimpleTimer.h>          //http://playground.arduino.cc/Code/SimpleTimer  || https://github.com/jfturcot/SimpleTimer

String clientName;
WiFiClient wifiClient;
const byte TIME_BETWEEN_READINGS = 15;  //In secs. Time before Wemos goes into sleep. Just before going to sleep, a 0 value is sent to the server.
//This time should not be too high so that the next pulse coming from the gaz detector is not missed!

//void callback(const MQTT::Publish& pub) {
// handle message arrived
//}

//PubSubClient client(wifiClient, "iot.eclipse.org");

//emoncms
const char* host = "emoncms.org";
const char* nodeId   = "2";

//either define key here of create a config.h key and add it there
//const char* privateKey = "My_Key";
#import "config.h"

ADC_MODE(ADC_VCC);


//float imp = 1;
//float t = 4;
int vcc;


void sendData(int imp);

void setup(void)
{
   Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop(void)
{

  vcc = ESP.getVcc();//readvdd33();
  // original loop
  delay(2000);
sendData(1); //send the pulse (one)

  for (int i = TIME_BETWEEN_READINGS; i > 0; i--) //wait some time before sending the 0 value pulse
  {
    delay(1000);
  }

sendData(0); //Send 0. This is to have a zero value after the pulse has been detected and sent to the server
   
  ESP.deepSleep(0);  //Sleep for ever, will be waked up by external schmitt trigger
  //delay(1000);
}


void sendData(int imp) {

  WiFiClient emoClient;

  const int httpPort = 80;
  if (!emoClient.connect(host, httpPort)) {

    return;
  }

  String json = "{gaz:";
  json += imp;
//  json += ",humidity:";
//  json += h;  
  json += ",vcc:";
  json += vcc;
  json += "}";

  String url = "/input/post.json?node=";
  url += nodeId;
  url += "&apikey=";
  url += privateKey;
  url += "&json=";
  url += json;

  // This will send the request to the server
  emoClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" +
                  "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (emoClient.available()) {
    String line = emoClient.readStringUntil('\r');
    Serial.print (line);
 
  }

/*
  if (client.connected()) {
    DEBUG_PRINT("Sending mqtt: ");
    //DEBUG_PRINT(json);

    if (client.publish(topic, (char*) json.c_str())) {
      DEBUG_PRINT("Publish ok");
    }
    else {
      DEBUG_PRINT("Publish failed");
    }
  }*/


}




