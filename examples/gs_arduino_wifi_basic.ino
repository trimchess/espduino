#include <SPI.h>
#include <Ethernet.h>

 
 
// Local Network Settings
byte mac[] = {
  0x90, 0xA2, 0xDA, 0x0E, 0x60, 0xA3 };  // Change this!!! Must be unique on local network.
                                         // Look for a sticker on the back of your Ethernet shield.
 
// GroveStreams Settings
char gsApiKey[] = "YOUR_SECRET_API_KEY_HERE";   //Change This!!!
char gsComponentName[] = "Temperature";        //Optionally change. Set this to give your component a name when it initially registers.
 
char gsDomain[] = "grovestreams.com";   //Don't change. The GroveStreams domain.
char gsComponentTemplateId[] = "temp";  //Don't change. Tells GS what template to use when the feed initially arrives and a new component needs to be created.
                                        // The blueprint is expecting "temp". 
 
//GroveStreams Stream IDs. Stream IDs tell GroveStreams which component streams the values will be assigned to.
//Don't change these unless you edit your GroveStreams component definition and change the stream ID to match these.
char gsStreamId1[] = "s1";   //Don't change. Temp C.
char gsStreamId2[] = "s2";   //Don't change. Temp F.
 
// Other Settings
const unsigned long updateFrequency = 20000UL;    // Update frequency in milliseconds (20000 = 20 seconds). Change this to change your sample frequency.
 
const int temperaturePin = 0;          // You might need to change depending on the Pin you are using. The Temperature pin number.    
char samples[35];                      // Change this buffer size only if you increase or decrease the size of samples being uploaded.
 
char myIPAddress[20];  //Don't Change. Set below from DHCP. Needed by GroveStreams to verify that a device is not uploading more than once every 10s.
char myMac[20];        //Don't Change. Set below from the above mac variable. The readable Mac is used by GS to determine which component the
                       // feeds are uploading into. It must match an existing GroveStreams component's ID
 
unsigned long lastSuccessfulUploadTime = 0; //Don't change. Used to determine if samples need to be uploaded.
int failedCounter = 0;                      //Don't change. Used for Internet Connection Reset logic
 
// Initialize Arduino Ethernet Client
EthernetClient client;
 
 
void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
 
  // Start Ethernet on Arduino
  startEthernet();
}
 
void loop()
{
 
  // Update sensor data to GroveStreams
  if(millis() - lastSuccessfulUploadTime > updateFrequency)
  {
    updateGroveStreams();
  }
 
}
 
void updateGroveStreams()
{
  unsigned long connectAttemptTime = millis();
 
  if (client.connect(gsDomain, 80))
  {        
 
    //Assemble the url used to pass the temperature readings to GroveStreams.
 
    //You may have to increase the size of the urlBuf if you've increased any other char array sizes
    char urlBuf[175]; //149
 
 
    sprintf(urlBuf, "PUT /api/feed?compTmplId=%s&compId=%s&compName=%s&api_key=%s%s HTTP/1.1", 
           gsComponentTemplateId, myMac, gsComponentName, gsApiKey, getSamples());
 
    Serial.println(urlBuf);    
    Serial.print(F("urlBuf length = "));
    Serial.println(strlen(urlBuf));
 
    client.println(urlBuf);  //Send the url with temp readings in one println(..) to decrease the chance of dropped packets
    client.print(F("Host: "));
    client.println();
    client.println(F("Connection: close"));
    client.print(F("X-Forwarded-For: "));     //Include this line and the next line if you have more than one device uploading behind
    client.println(myIPAddress);              // your outward facing router (avoids the GS 10 second upload rule)
    client.println(F("Content-Type: application/json"));
 
    client.println(); 
 
 
    if (client.connected())
    {
 
      //Report Response
      while(!client.available())
      {
        delay(1);
      }
 
      while(client.available()) 
      {
        char c = client.read();
        Serial.print(c);
      }
 
 
      //Client is disconnected now; stop it.
      client.stop();
 
      lastSuccessfulUploadTime = connectAttemptTime;
      failedCounter = 0;
    }
    else
    {
 
      handleConnectionFailure();
    }
 
  }
  else
  {
     handleConnectionFailure();
  }
 
}
 
void handleConnectionFailure() {
  //Connection failed. Increase failed counter
  failedCounter++;
 
  Serial.print(F("Connection to GroveStreams Failed "));
  Serial.print(failedCounter);  
  Serial.println(F(" times"));
  delay(1000);
 
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) 
  {
 
    //Too many failures. Restart Ethernet.
    startEthernet();
  }
 }
 
void startEthernet()
{
  //Start or restart the Ethernet connection.
  client.stop();
 
  Serial.println(F("Connecting Arduino to network..."));
  Serial.println();  
 
  //Wait for the connection to finish stopping
  delay(2000);
 
  //Connect to the network and obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println(F("DHCP Failed, reset your Arduino and try again"));
    Serial.println();
  }
  else
  {
    Serial.println(F("Arduino connected to network using DHCP"));
 
    //Set the mac and ip variables so that they can be used during sensor uploads later
    Serial.print(F(" MAC: "));
    Serial.println(getMacReadable());
    Serial.print(F(" IP address: "));
    Serial.println(getIpReadable(Ethernet.localIP()));
    Serial.println();
  }
 
}
 
char* getMacReadable()
{
  //Convert the mac address to a readable string
  sprintf(myMac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return myMac;
}
 
char* getIpReadable(IPAddress ipAddress)
{
  //Convert the ip address to a readable string
  unsigned char octet[4]  = {0,0,0,0};
  for (int i=0; i<4; i++)
  {
    octet[i] = ( ipAddress >> (i*8) ) & 0xFF;
  }
  sprintf(myIPAddress, "%d.%d.%d.%d",octet[0],octet[1],octet[2],octet[3]);
 
  return myIPAddress;
}
 
char* getSamples()
{
  //Get the temperature analog reading and convert it to a string
  float voltage, degreesC, degreesF;
 
  voltage = (analogRead(temperaturePin) * 0.004882814);
  degreesC = (voltage - 0.5) * 100.0;
  degreesF = degreesC * (9.0/5.0) + 32.0;
 
  char tempC[15] = {0}; //Initialize buffer to nulls
  dtostrf(degreesC, 12, 3, tempC); //Convert float to string
 
  char tempF[15] = {0}; //Initialize buffer to nulls
  dtostrf(degreesF, 12, 3, tempF); //Convert float to string
 
  sprintf(samples, "&%s=%s&%s=%s", gsStreamId1, trim(tempC), gsStreamId2, trim(tempF));
 
  return samples;
}
 
char* trim(char* input)                                         
{
  //Trim leading and ending spaces
  int i,j;
  char *output=input;
  for (i = 0, j = 0; i<strlen(input); i++,j++)          
  {
    if (input[i]!=' ')                           
      output[j]=input[i];                     
    else
      j--;                                     
  }
  output[j]=0;
 
  return output;
}