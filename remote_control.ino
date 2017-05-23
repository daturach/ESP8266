/* arduino json library from https://github.com/bblanchon/ArduinoJson
sketch from http://blog.nyl.io/esp8266-led-arduino/

Add your index.php file on a server running PHP. You should see 2 buttons. You can click on any of them.
A file called light.json will be created on the server. This file will be read by this sketch and activate anything you would
like to activate locally. Security is not that high but you don't have to open a port in your router.
You can change the delay (const long interval) between the reads. If you set it to one minute, once you click on a button it will take max. one minute
to react.
raspi_time_off: set the number of seconds the raspi is off (what else!)
The Raspi status is sent to emoncms. If the raspi is OFF, the value is 0. If the raspi is ON, the value is 1. 
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

//emoncms
//const char* host2 = ""; //set in config.h
const char* nodeId   = "1";

//const char* ssid     = ""; //set in config.h
//const char* password = ""; //set in config.h
#import "config.h";
//const char* host     = ""; // Your domain, set in config.h  
//String path          = "/path/to/light.json"; //set in config.h
//const int pin        = 2;
const int relayPin = D1;
const long interval = 600000;  // pause for 600 seconds
const long raspi_time_off = 15000;

unsigned long previousMillis = 0;

void sendData(int imp);

void setup() {  
  pinMode(relayPin, OUTPUT); 
//  pinMode(relayPin, LOW);
  Serial.begin(115200);

  //delay(1000);
/*  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  delay(1500);  
  //Serial.println("IP address: " + WiFi.localIP());  //crash
  IPAddress local_ip = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(local_ip);
  delay(1500);*/
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print("Connecting to ");
    Serial.println(ssid[0]);
    WiFi.begin(ssid[0], password[0]);
    int wifi_ctr = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    delay(1500);  
    //Serial.println("IP address: " + WiFi.localIP());  //crash
    IPAddress local_ip = WiFi.localIP();
    Serial.print("IP address: ");
    Serial.println(local_ip);
    delay(1500);
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");

    delay(500); // wait for server to respond

  // read response
    String section="header";
    while(client.available()){
      String line = client.readStringUntil('\r');
    // Serial.print(line);
    // we’ll parse the HTML body here
      if (section=="header") { // headers..
        Serial.print(".");
        if (line=="\n") { // skips the empty space at the beginning 
          section="json";
        }
      }
      else if (section=="json") {  // print the good stuff
        section="ignore";
        String result = line.substring(1);
      
      // Parse JSON
        int size = result.length() + 1;
        char json[size];
        result.toCharArray(json, size);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& json_parsed = jsonBuffer.parseObject(json);
        if (!json_parsed.success()) {
          Serial.println("parseObject() failed");
          return;
        }

      // Make the decision to turn off or on the LED/Relay
        if (strcmp(json_parsed["light"], "on") == 0) {
          digitalWrite(relayPin, HIGH);
          sendData(0); //Send to host2; raspi is OFF, relais sous tension
          Serial.println("Raspi OFF");
          delay(raspi_time_off);  //During this time, Raspi is OFF
          digitalWrite(relayPin, LOW); //Raspi is ON again, relais non alimenté
          sendData(1); //Send to host2; raspi is ON
          Serial.println("Raspi ON");
          WiFi.disconnect();
        }
        else {
          digitalWrite(relayPin, LOW); //raspi is ON
          sendData(1); //Send to host2; raspi is ON
          Serial.println("Raspi ON");
          WiFi.disconnect();
        }
      }
    }
  Serial.println("closing connection. ");
  }
}

void sendData(int imp) {  //send to host2

  WiFiClient emoClient;

  const int httpPort = 80;
  if (!emoClient.connect(host2, httpPort)) {

    return;
  }

  String json = "{rasp_status:"; //imp=1, raspi is ON; imp=0, raspi is OFF
  json += imp;
//  json += ",humidity:";
//  json += h;  
//  json += ",vcc:";
//  json += vcc;
  json += "}";

  String url = "/input/post.json?node=";
  url += nodeId;
  url += "&apikey=";
  url += privateKey;
  url += "&json=";
  url += json;

  // This will send the request to the server
  emoClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                  "Host: " + host2 + "\r\n" +
                  "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (emoClient.available()) {
    String line = emoClient.readStringUntil('\r');
    Serial.print (line);
 
  }
}
