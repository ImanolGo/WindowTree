/*
 *  UdpManager.h
 *  Light Painting App
 *
 *  Created by Imanol Gomez on 10/05/18.
 *
 */


#pragma once

#include "ofMain.h"
#include "ofxNetwork.h"
#include "Manager.h"
#include "ofxSimpleTimer.h"


//========================== class UdpManager =======================================
//==============================================================================
/** \class UdpManager UdpManager.h
 *	\brief class for managing the udp connection
 *	\details It writes and send udp data
 */


class UdpManager: public Manager
{

    static const int UDP_MESSAGE_LENGHT; ///Defines the Udp"s message length
    static const int UDP_MTU_ETHERNET; ///Defines the Ethernet's maximum transmission unit
    static const int DATA_HEADER_OVERHEAD; ///Defines the data's header overhead
    
    struct udp_header {
        unsigned char f1;
        unsigned char f2;
        unsigned char f3;
        unsigned short payload_size;
        unsigned short command;
    };
    
public:
    //! Constructor
    UdpManager();

    //! Destructor
    virtual ~UdpManager();

    //! setups the udp manager
    void setup();
    
    //! updates the udp manager
    void update();
    
    void timerCompleteHandler( int &args ) ;
    
    void setMaxDataPacketSize(int& value );
    
private:
    
    void setupUdpConnection();
    
    void setupHeaders();
    
    void setupTimer();
    
    void setupIP();
    
    void updateReveivePackage();
    
    void updateTime();
    
    bool isMessage(char * buffer, int size);
    
    void parseMessage(char * buffer, int size);
    
    void receivedIp(char _id);
    
    void receivedHeartbeat(char _id, char val1, char val2);
    
    void createConnection(string& ip, int port);
    
    void sendAutodiscovery();
    
    void sendConnected();
    
    void sendDiscovered();
    
    void sendTime();
    
    void updatePixels();
    
    string getDataHeader(unsigned short num_pixels);
    
    string getDataPayload(unsigned short channel, unsigned short offset, unsigned short num_pixels, const vector<ofFloatColor>& pixels);
    
    void printHex(const string& message);
    
private:
    
    ofxUDPManager m_udpConnection;
    udp_header    m_dataHeader;
    udp_header    m_connectHeader;
    udp_header    m_autodiscoveryHeader;
    udp_header    m_timeHeader;
    ofxSimpleTimer         m_timer;
    string                 m_broadcast;
    string                 m_ip;
    bool                   m_connected;
    unsigned int            m_frameNumber;
    bool            m_refreshTime;
    bool            m_refreshPixels;
    double          m_elapsedTime;
    
    unsigned short  m_maxNumPixelsPerPacket;
    unsigned short  m_maxDataPacketSize;
};

