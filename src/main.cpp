#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiParam.h>
#include <dht11.h>
#include <TimedAction.h>



dht11 DHT11;
#define DHT11PIN 2 //D2 on board ESP8266-13
#define UPLOAD_TIMEOUT 600000 // 600ms > 10min

#define LUM_PIN A0

void updateGroveStreams();

String apiKey = "6e3b31c6-1f5a-39d4-b3d6-ca2ee27dd4a8";
const char* server = "www.grovestreams.com";

WiFiClient client;

TimedAction checkForUpdateGroveStreams = TimedAction(UPLOAD_TIMEOUT, updateGroveStreams);


void setup()
{
  Serial.begin(115200);
  delay(10);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop()
{
  //checkForUpdateGroveStreams.check();
  updateGroveStreams();
  delay(UPLOAD_TIMEOUT);
}

void updateGroveStreams()
{
  Serial.println("Enter updateGroveStreams");
  char url[250];
  int chk = DHT11.read(DHT11PIN);
  float lum = analogRead(LUM_PIN);
  Serial.println(lum);
  if (client.connect(server,80) && (chk ==  DHTLIB_OK))
  {
    float t = DHT11.temperature;
    float h = DHT11.humidity;
    Serial.println(t);
    Serial.println(h);
    Serial.println("Connected to Grovestreams, try to upload");
    //Uploaded grovestreams stream:
    //PUT /api/feed?compId=Bedroom_1&temp=24.000000&humid=33.000000&lum=461.000000&api_key=6e3b31c6-1f5a-39d4-b3d6-ca2ee27dd4a8& HTTP/1.1
    sprintf(url, "PUT /api/feed?compId=Lounge_1&temp=%f&humid=%f&lum=%f&api_key=6e3b31c6-1f5a-39d4-b3d6-ca2ee27dd4a8& HTTP/1.1",t,h,lum);
    Serial.println("Upload to Grovestreams");
    Serial.println(url);
    Serial.print(F("urlBuf length = "));
    Serial.println(strlen(url));
    client.println(url);
    client.print(F("Host: "));
    client.println();
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/json"));
    client.println();
  }
  else
  {
    Serial.println("Error while processing... DHT11_RESPONSE:");
    Serial.println(chk);
  }
  client.stop();
}
