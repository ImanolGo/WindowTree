///////////////////////////////////////////////////////////////////
// Class controlling the leds and its effects
//
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2019, Imanol Gomez
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"
#include "Config.h"
#include "FastLED.h"
#include "WiFiManager.h"

const byte channelwidth = 3; //3 channels per pixel


class LedsManager{

  public:
    
    LedsManager();
    
    void setup();
    void update();

    void parseRGBReceived(unsigned char* pbuff, int packetSize);
    void setAllColor(CRGB color);
    
  private:

    void setupLeds();
    void initTest();
    void checkFps();

    //int data_pins[NUM_CHANNELS];
    //int clock_pins[NUM_CHANNELS];
    CRGBArray<NUM_LEDS> leds;
    
};

LedsManager::LedsManager()
{

}

void LedsManager::setup()
{
    Serial.println("LedsManager::setup");
    this->setupLeds(); 
    //this->initTest();
}


void LedsManager::setupLeds()
{

   FastLED.addLeds<LED_TYPE,DATA_PIN_,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(UncorrectedColor);
 
   FastLED.setMaxPowerInVoltsAndMilliamps (5, 2100);
   FastLED.setDither( 0 );
   FastLED.clear();  
   this->setAllColor(CRGB::Black); 
   Serial.println("LedsManager::setupLeds");
}


void LedsManager::update()
{
    this->checkFps();
}

void LedsManager::parseRGBReceived(unsigned char* pbuff, int packetSize) 
{
    if(packetSize<HEADER_SIZE+DATA_HEADER_SIZE)
        return;

     unsigned short index = HEADER_SIZE;
     unsigned short channel = ByteToShort(pbuff[index++], pbuff[index++]);
     unsigned short offset = ByteToShort(pbuff[index++], pbuff[index++]);
     unsigned short num_pixels = ByteToShort(pbuff[index++], pbuff[index++]);

//     Serial.print("LedsManager::parseRGBReceived -> channel: ");
//     Serial.println(channel);
//
//     Serial.print("LedsManager::parseRGBReceived -> offset: ");
//     Serial.println(offset);
//
//     Serial.print("LedsManager::parseRGBReceived -> num pixels: ");
//     Serial.println(num_pixels);

     int data_bytes = CHANNEL_WIDTH*num_pixels;

    if(packetSize == HEADER_SIZE+DATA_HEADER_SIZE + data_bytes)
    {
        int start_index = offset;
        if(start_index>=NUM_LEDS){
          return;
        }
        int end_index = start_index + num_pixels;
        if(end_index > NUM_LEDS){
           end_index = NUM_LEDS;
        }
  
        int channel = HEADER_SIZE+DATA_HEADER_SIZE; 
        for (int i = start_index; i < end_index; i++) //loop to assign 3 channels to each pixel
        {
            leds[i] = CRGB(pbuff[channel++], pbuff[channel++], pbuff[channel++]);
        }
  
        
    }
    
    FastLED.show(); //send data to pixels
}

void LedsManager::checkFps()
{
    EVERY_N_MILLISECONDS(FPS_CHECK_TIME_MS) 
    {
      Serial.print("LedsManager::fsp-> ");
      Serial.println(LEDS.getFPS());                       
   }
}

void LedsManager::setAllColor(CRGB color) 
{
  fill_solid(leds,NUM_LEDS, color);
  FastLED.show();
}
void LedsManager::initTest() //runs at board boot to make sure pixels are working
{
  FastLED.setBrightness(MAX_BRIGHTNESS);       // set to full power
  
  FastLED.clear(); // on power up, one or more leds can be in a funky state.
  
  
  Serial.println ("LedsManager::Red:");
  this->setAllColor(CRGB::Red);
  
  FastLED.delay(TEST_DELAY);
  
  Serial.println ("LedsManager::Green");
  this->setAllColor(CRGB::Green);
  
  FastLED.delay(TEST_DELAY);
  
  Serial.println ("LedsManager::Blue");
  this->setAllColor(CRGB::Blue);
  
  FastLED.delay(TEST_DELAY);

  Serial.println ("LedsManager::Yellow");
  this->setAllColor(CRGB::Yellow);
  
  FastLED.delay(TEST_DELAY);

  Serial.println ("LedsManager::Violet");
  this->setAllColor(CRGB::Violet);
 
  FastLED.delay(TEST_DELAY);

  Serial.println ("LedsManager::White  - Check Power!");
  this->setAllColor(CRGB::White);
  
  FastLED.delay(TEST_DELAY);

  Serial.println("LedsManager::Show Time...");
  FastLED.clear();
  this->setAllColor(CRGB::Black);
  //FastLED.setBrightness(MAX_BRIGHTNESS);
}

    
