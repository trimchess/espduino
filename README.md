# espduino ESP8266-ESP13 ESP Board
Project with some simple code to measure temperature and humidity with an DHT11 sensor
and to measure the light intensity with an LDR using the A/D converter on the ESP Board.
The results are pushed eyvery 10 Mminutes to grovestreams. Therefore you need a (free) grovestreams account (grovestreams.com)
The WiFi network data are in the WiFiParam.h file (ssid, password). The WiFiParam File is not included in the Project and
all other .h Files ar also not ncluded.
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiParam.h>
#include <dht11.h>			http://playground.arduino.cc/Main/DHT11Lib
#include <TimedAction.h>		Not used in this version
#include "grovestreams.h"   Includes the grovestream API Key. You should place your
                            grovestreams.h file in a local lib directory and place there
                            your grovestreams API-key
                            (String apiKey = "your-personal-grovestreams-api-key")

Developped with the Atomic/PlatformIO IDE.
