///////////////////////////////////////////////////////////////////
// Class controlling the wifi connection
//
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2019, Imanol Gomez
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"
#include "Config.h"
#include <WiFi.h>
#include <WiFiUdp.h>
//#include <WiFiMulti.h>
#include "LedsManager.h"


//The udp library class
WiFiUDP UdpReceive;
WiFiUDP UdpDiscovery;

bool wifiConnected = false;

class WifiManager
{
  private:
 
  LedsManager* ledsManager;  ///< it sends the commands to the leds manager
  

  public:
    
    WifiManager(LedsManager* ledsManager);
    
    void setup();
    void update();

    static void WiFiEvent(WiFiEvent_t event);

  private:

    void initializeWifi();
    void parseUdp();
    void connectToWiFi(const char * ssid, const char * pwd);
    void connectWifi();
    void checkWifiConnection();
    void sendAutodiscovery();
    void noReceive();

    bool parseHeader(const unsigned char* messagein, int messagelength, unsigned short& command);
    int BtoI(byte a, byte b);

    String ssid;
    String pass;
    bool is_connected;
    unsigned long autodiscovery_timer;
    unsigned long no_data_timer;

    //WiFiUDP Udp;
    unsigned char packetBuffer[BUFFER_MAX];
    int receivedUdpLength;

    char PACKET_START;
    char AUTO_DISCOVERY_COMMAND;
    char PACKET_END;
  
};


WifiManager::WifiManager(LedsManager* ledsManager)
{
    this->ledsManager=ledsManager;

    ssid     =  "Don't worry, be happy!";
    pass = "whyistheskysohigh?";
    

    wifiConnected = false;

    receivedUdpLength = 0;
    autodiscovery_timer =  millis();
    no_data_timer =  millis();
    is_connected = false;
}

void WifiManager::setup()
{
    Serial.println("WifiManager::setup");
    initializeWifi();
   // connectWifi();
}

void WifiManager::initializeWifi()
{
  
    UdpReceive.stop();
    UdpReceive.flush();
    UdpDiscovery.stop();
    UdpDiscovery.flush();
  

    Serial.println("WifiManager::connect wifi");
    connectToWiFi(ssid.c_str(), pass.c_str());
    //connectWifi();
}


void WifiManager::connectWifi() {

   wifiConnected = true;
     // attempt to connect to WiFi network:
   //Serial.print("Attempting to connect to SSID: ");
   //Serial.println(ssid);
   WiFi.begin(ssid.c_str(), pass.c_str());

   int attempts = 0;
 
    unsigned long connect_start = millis();
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("WifiManager::WIFI STATUS ");Serial.println(WiFi.status());
      Serial.print("..");
      attempts++;
      //connected = true;
      if(attempts>=15){
        wifiConnected =  false;
        break;
      }
   }

   wifiConnected = true;
   if(wifiConnected){
         Serial.print("\nWifiManager::Connected to SSID: ");
         Serial.println(ssid);
      
         Serial.println("WifiManager::IP address: ");
         Serial.println(WiFi.localIP());
        
         Serial.print("\nWifiManager::Starting connection to UDP port ");
         Serial.println(LOCAL_PORT);

         // if you get a connection, report back via serial:
         UdpReceive.begin(LOCAL_PORT);
         UdpReceive.flush();
    

   }

   else{
        Serial.print("\nUnable to connect to: ");
        Serial.println(ssid);
   }

    Serial.print("\nConnected:  ");
    Serial.println(wifiConnected);
  
    //connected = true;
}



void WifiManager::update()
{
    parseUdp();
    sendAutodiscovery();
    noReceive();
}


void WifiManager::parseUdp()
{
  // if there's data available, read a packet
  int packetSize = UdpReceive.parsePacket();
  if (packetSize)
  {   
//      Serial.print("WifiManager::New Message: Size -> ");
//      Serial.println(packetSize);
      UdpReceive.read(packetBuffer,BUFFER_MAX); //read UDP packet
      unsigned short command;
      if(parseHeader(packetBuffer, packetSize, command))
      {     
            //Serial.println(command);
            if(command == COMMAND_DATA){
//              Serial.println(command);
//              Serial.println("WifiManager::parseUdp-> Received Data!!!");
              no_data_timer =  millis();
              this->ledsManager->parseRGBReceived(packetBuffer, packetSize); //process data function
            }
            else if(command== COMMAND_CONNECT){
               is_connected = true;
               Serial.println(command);
               Serial.println("WifiManager::parseUdp-> Device Connected!!!");
               no_data_timer =  millis();
               // if you get a connection, report back via serial:
//               Udp.begin(LOCAL_PORT);
//               Udp.flush();
            }

            else if(command == COMMAND_AUTODISCOVERY){
              Serial.println(command);
              Serial.print("WifiManager::parseUdp-> Send Autodiscovery ");
               is_connected = false;
            }

            else if(command == COMMAND_TIME){
               this->ledsManager->show();
            }
           
           
              
      }
  }
}



bool WifiManager::parseHeader(const unsigned char* messagein, int messagelength, unsigned short& command) 
{

    if(messagelength < HEADER_SIZE)
    {
      return false;
    }

    unsigned short index = 0;
    
    if ( messagein[index++] == HEADER_F1 && messagein[index++] == HEADER_F2 && messagein[index++] == HEADER_F3) 
    { 
        unsigned short payload_size =  ByteToShort(messagein[index++], messagein[index++]);
        if( messagelength - HEADER_SIZE == payload_size)
            {
              command = ByteToShort(messagein[index++], messagein[index++]);
              return true;
            }
        
     }
  return false;
}

void WifiManager::connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("WifiManager::Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handlerpin
  WiFi.onEvent(WiFiEvent);
 // WiFi.config(ip, gateway, subnet);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("WifiManager::Waiting for WIFI connection...");
}


void WifiManager::checkWifiConnection(){
   if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WifiManager::WiFi connection lost. Reconnecting...");
      this->connectWifi();
    } else {
      Serial.println("WifiManager::WiFi OK");
    }
}


//wifi event handler
void WifiManager::WiFiEvent(WiFiEvent_t event){
    
    IPAddress ip;
  
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WifiManager::WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          UdpReceive.begin(LOCAL_PORT);
          UdpDiscovery.begin(DISCOVERY_PORT);
          Serial.print("Listening to port: ");
          Serial.println(LOCAL_PORT); 
          wifiConnected = true;
          ip = WiFi.localIP();  ip[3] = 255;
 
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WifiManager::WiFi lost connection");
          wifiConnected = false;
          //software_Reset();
          break;
    }
}


void WifiManager::sendAutodiscovery()
{
  //if(is_connected || !wifiConnected) return;

  if (!wifiConnected) return;

  if(is_connected) return;

  if( millis() - autodiscovery_timer > DISCOVERY_TIMER)
  {
      IPAddress ip = WiFi.localIP();
      ip[3] = 255;

      int packetLength = 8;
      char bffr[packetLength];
      bffr[0] = HEADER_F1;
      bffr[1] = HEADER_F2;
      bffr[2] = HEADER_F3;
      bffr[3] = 0;
      bffr[4] = 1;
      bffr[5] = 0;
      bffr[6] = 'c';
      bffr[7] = 'c';
      
      // transmit broadcast package
   
      UdpDiscovery.beginPacket(ip, SEND_PORT);
      UdpDiscovery.write((uint8_t *)bffr,packetLength);
      UdpDiscovery.endPacket();

      Serial.println("WifiManager::Autodiscovery sent!");
      autodiscovery_timer = millis();
  }
}

void WifiManager::noReceive()
{
    if((millis() - no_data_timer) > NO_DATA_TIMEOUT) //is the time since the counter changed greater than 5 seconds?
    {
       Serial.println("WifiManager::No received Data!!");
       this->ledsManager->setAllColor(CRGB::Black);
        no_data_timer =  millis();
        //autodiscovery_timer = millis();
        is_connected = false;
    }
    
}
