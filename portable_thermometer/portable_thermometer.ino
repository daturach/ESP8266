/*********************************************************************
Measure temperatures and send to emoncms.org on node 4

Adafruit libraries have been modified by mcauser to work on 64x48 OLED display
https://github.com/mcauser/Adafruit_SSD1306/tree/esp8266-64x48/examples

+++++++++++++++++
From Adafruit:
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 64x48 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

The ESP8266EX(Which IC D1 board used)  integrates a generic purpose 10-bit analog ADC. The ADC range is from 0V to 1.0V
And on the  board, we use following circuit:
++++++++++++++++++

*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>


// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{
  0b00000001, 0b11000000, //        ###      
  0b00000011, 0b11100000, //       #####     
  0b00000111, 0b00100000, //      ###  #     
  0b00000111, 0b11100000, //      ######     
  0b00000111, 0b00100000, //      ###  #     
  0b00000111, 0b11100000, //      ######     
  0b00000111, 0b00100000, //      ###  #     
  0b00000111, 0b11100000, //      ######     
  0b00000111, 0b00100000, //      ###  #     
  0b00001111, 0b11110000, //     ########    
  0b00011111, 0b11111000, //    ##########   
  0b00011111, 0b11111000, //    ##########   
  0b00011111, 0b11111000, //    ##########   
  0b00011111, 0b11111000, //    ##########   
  0b00001111, 0b11110000, //     ########    
  0b00000111, 0b11100000, //      ######     
};

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Data wire is plugged into GPIO2 (D4) on the Wemos mini
#define ONE_WIRE_BUS D4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

String clientName;
WiFiClient wifiClient;
ESP8266WiFiMulti wifiMulti;

//emoncms
const char* host = "emoncms.org";
const char* nodeId   = "4";
#import "config.h"
float temp_sensor = 0;
/*float temp_pipe_in = 0;
float temp_pipe_out = 0;
int sensorValue = 0;
float voltage = 0;*/
int interval = 1;  //interval in minutes for the updates on the server

void sendData();


void setup()   {
  
  Serial.begin(9600);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  /*display.display(); //this line causes problems when resetting the Wemos board. Connection is not always a success
  delay(2000); //this line causes problems when resetting the Wemos board. Connection is not always a success */
  
  Serial.println();
  Serial.println();
  
  /*WiFi.begin(ssid[0], password[0]);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());*/

  Serial.print("Connecting to Wifi");

  wifiMulti.addAP(ssid[0], password[0]);
  wifiMulti.addAP(ssid[1], password[1]);
  wifiMulti.addAP(ssid[2], password[2]);
  
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  //Serial.println("");
  //Serial.println("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Clear the buffer.
  /*display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connected!");
  display.display();
  delay(2000); */
  
  Serial.println("Collecting temperatures...");
  sensors.begin();


  // Clear the buffer.
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.println("Connected!");
  display.display();
  delay(2000); 

}


void loop() {
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  //delay (10000);
  Serial.println(sensors.getTempCByIndex(0));
  
  /*sensorValue = analogRead(A0);
  Serial.print (sensorValue);
  voltage = sensorValue * (8.55 / 1023.0);  //factor depends on the different resistors acting as a divider
  Serial.print("Battery voltage: ");
  Serial.println(voltage);*/

  display.drawBitmap(0, 16,  logo16_glcd_bmp, 16, 16, 1);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(16,16);
  //display.println("Hello, world!");
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println(sensors.getTempCByIndex(0));
  temp_sensor = sensors.getTempCByIndex(0);
  /*temp_pipe_in = sensors.getTempCByIndex(1);
  temp_pipe_out = sensors.getTempCByIndex(2);*/
  //display.setTextSize(2);
  //display.setTextColor(WHITE);
  //display.print("0x"); display.println(0xDEADBEEF, HEX);
 
  sendData();

  delay(2000);
 // Clear the buffer.
  display.clearDisplay();
  display.display(); 
  
  ESP.deepSleep(interval * 60 * 1000000); //interval in minute, max: 73 minutes ?
  delay(100);
}

void sendData() {

  WiFiClient emoClient;

  const int httpPort = 80;
  if (!emoClient.connect(host, httpPort)) {
    return;
  }

  String json = "{temp_sensor:";
  json += temp_sensor;
  /*json += ",water_pipe_out:";
  json += temp_pipe_out;  
  json += ",water_pipe_in:";
  json += temp_pipe_in;
  json += ",bat_voltage:";
  json += voltage;*/
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
  delay(100);

  // Read all the lines of the reply from server and print them to Serial
  while (emoClient.available()) {
    String line = emoClient.readStringUntil('\r');
    Serial.print (line);
  }
  display.println ("\n\nServer Ok");
  display.display();
  
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

