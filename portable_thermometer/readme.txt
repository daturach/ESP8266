Adafruit libraries have been modified by mcauser to work on 64x48 OLED display
See https://github.com/mcauser/Adafruit_SSD1306/tree/esp8266-64x48/examples

Compiled with Arduino IDE 1.6.10

The thermometer runs at 3 VDC using two 1.5 V batteries. It's not optimal because the temp sensor
runs at 3VDC; the OLED display runs at 3.3VDC! and I don't know what will happen when batteries will discharge.

When programming the Wemos, switch the thermometer OFF so that the 3.3 VDC out from the regulator 
is not applied at the batteries output. 
The Wemos should connect to the best AP according to the list provided in the config.h file. Outdoors, it's
necessary to use a mobile phone in tethering mode.

Right after the temperature is sent to the server, the thermometer goes into deep sleep during the time specified.
