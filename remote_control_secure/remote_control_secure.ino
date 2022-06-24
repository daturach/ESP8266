// Mar 2018 by Earle F. Philhower, III
// Released to the public domain
// Updated daturach june 2022, built with IDE 1.8.19, Arduino Jason version 5.6.5 (does NOT work with 6 and above)

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "certs.h"
#include <ArduinoJson.h>
#include "config.h"

//emoncms
//const char* host2 = ""; //set in config.h
const char* nodeId   = "1";

//const char* ssid     = ""; //set in config.h
//const char* password = ""; //set in config.h

//const char* host     = ""; // Your domain, set in config.h  
//String path          = "/path/to/light.json"; //set in config.h
//const int pin        = 2;
const int relayPin = D1;
const long interval = 600000;  // pause for 600 seconds
//const long interval = 60000;  // pause for 60 seconds, for testing
const long raspi_time_off = 15000;

unsigned long previousMillis = 0;

void sendData(int imp);


// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "ch.pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// Try and connect using a WiFiClientBearSSL to specified host:port and dump HTTP response
void fetchURL(BearSSL::WiFiClientSecure *client, const char *host, const uint16_t port, const char *path) {
  /*if (!path) {
    path = "/";
  }*/

  Serial.printf("Trying: %s:443...", host);
  client->connect(host, port);
  if (!client->connected()) {
    Serial.printf("*** Can't connect. ***\n-------\n");
    return;
  }
  Serial.printf("Connected!\n-------\n");
  client->write("GET ");
  client->write(path);
  client->write(" HTTP/1.0\r\nHost: ");
  client->write(host);
  client->write("\r\nUser-Agent: ESP8266\r\n");
  client->write("\r\n");
}


void fetchCertAuthority() {

  BearSSL::WiFiClientSecure client;
  BearSSL::X509List cert(cert_USERTrust_RSA_Certification_Authority);
  client.setTrustAnchors(&cert);

  setClock();
  fetchURL(&client, host, port, path);

    delay(2000); // wait for server to respond
    //Serial.print(path);

  // read response
    String section="header";
    while(client.available()){
      String line = client.readStringUntil('\r');
     //Serial.print(line);
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
        //Serial.println(size);
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


void setup() {
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();
}


void loop() {
    unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print("Connecting to ");
    Serial.println(ssid[0]);
    WiFi.begin(ssid[0], pass[0]);
    //int wifi_ctr = 0;
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
    
    fetchCertAuthority();
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
