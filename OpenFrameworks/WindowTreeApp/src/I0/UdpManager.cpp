/*
 *  UdpManager.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 16/10/19.
 *
 */


#include "UdpManager.h"
#include "AppManager.h"
#include "ofxNetworkUtils_.h"

const int UdpManager::UDP_MESSAGE_LENGHT = 100;
const int UdpManager::UDP_MTU_ETHERNET = 1450;
const int UdpManager::DATA_HEADER_OVERHEAD = 7;

UdpManager::UdpManager(): Manager(), m_connected(false), m_maxNumPixelsPerPacket(100),  m_frameNumber(1), m_refreshPixels(false), m_refreshTime(true)
{
    //Intentionally left empty
}

UdpManager::~UdpManager()
{
   ofLogNotice() << "UdpManager::destructor";
}


//--------------------------------------------------------------

void UdpManager::setup()
{
    if(m_initialized)
        return;
    
    Manager::setup();
    
    this->setupHeaders();
    this->setupIP();
    this->setupTimer();
    this->sendAutodiscovery();
    this->setupUdpConnection();
    
    ofLogNotice() <<"UdpManager::initialized" ;
}


void UdpManager::setupHeaders()
{
    m_dataHeader.f1 = 0x10;
    m_dataHeader.f2 = 0x41;
    m_dataHeader.f3 = 0x37;
    m_dataHeader.payload_size = 0;
    m_dataHeader.command = 'd';
    
    m_connectHeader.f1 = 0x10;
    m_connectHeader.f2 = 0x41;
    m_connectHeader.f3 = 0x37;
    m_connectHeader.payload_size = 1;
    m_connectHeader.command = 'c';
    
    m_autodiscoveryHeader.f1 = 0x10;
    m_autodiscoveryHeader.f2 = 0x41;
    m_autodiscoveryHeader.f3 = 0x37;
    m_autodiscoveryHeader.payload_size = 1;
    m_autodiscoveryHeader.command = 'a';
    
    m_timeHeader.f1 = 0x10;
    m_timeHeader.f2 = 0x41;
    m_timeHeader.f3 = 0x37;
    m_timeHeader.payload_size = 4;
    m_timeHeader.command = 't';
    
    m_maxDataPacketSize = UDP_MTU_ETHERNET;
    m_maxNumPixelsPerPacket = (m_maxDataPacketSize-DATA_HEADER_OVERHEAD)/3;
}


void UdpManager::setupUdpConnection()
{
    int portReceive = AppManager::getInstance().getSettingsManager().getUdpPortReceive();
    ofLogNotice() <<"UdpManager::setupUdpReceiver -> listening for udp messages on port  " << portReceive;
    
    //m_udpConnection.SetEnableBroadcast(true);
    m_udpConnection.Create(); //create the socket
    m_udpConnection.Bind(portReceive); //and bind to port
    m_udpConnection.SetNonBlocking(true);
    
    
    //string ip = AppManager::getInstance().getSettingsManager().getIpAddress();
    int portSend = AppManager::getInstance().getSettingsManager().getUdpPortSend();
    
    m_udpConnection.Connect(m_broadcast.c_str(),portSend);
    //m_udpConnection.Connect(ip.c_str(),portSend);
    m_udpConnection.SetEnableBroadcast(true);
    
    ofLogNotice() <<"UdpManager::setupUdpReceiver -> sending to IP " << m_broadcast <<" to port " << portSend;
    
    m_udpConnection.SetNonBlocking(true);
    
}

void UdpManager::createConnection(string& ip, int send)
{
    ofLogNotice() <<"UdpManager::createConnection -> sending to IP " << ip.c_str() <<" to port " << send;
    int portReceive = AppManager::getInstance().getSettingsManager().getUdpPortReceive();
    
    m_udpConnection.Close();
    
    m_udpConnection.SetEnableBroadcast(false);
    m_udpConnection.Create(); //create the socket
    m_udpConnection.Bind(portReceive); //and bind to port
    m_udpConnection.SetNonBlocking(true);
    
    
    m_udpConnection.Connect(ip.c_str(),send);
    m_udpConnection.SetNonBlocking(true);
    m_connected = true;
    m_timer.stop();
}


void UdpManager::setupTimer()
{
    m_timer.setup( 1000 );
    
    m_timer.start( false ) ;
    ofAddListener( m_timer.TIMER_COMPLETE , this, &UdpManager::timerCompleteHandler ) ;
}

void UdpManager::setupIP()
{
    Poco::Net::NetworkInterface::List siteLocalInterfaces = ofxNet::NetworkUtils::listNetworkInterfaces(ofxNet::NetworkUtils::SITE_LOCAL);
    
    if(!siteLocalInterfaces.empty()){
        m_ip = siteLocalInterfaces[0].address().toString();
    }
    
    //m_ip = AppManager::getInstance().getSettingsManager().getIpAddress();
    ofLogNotice() <<"UdpManager::setupIP -> IP address: " << m_ip;
    
    m_broadcast = "";
    auto stringSplit = ofSplitString(m_ip, ".");
    for(int i=0; i<stringSplit.size(); i++){
        
        if(i<stringSplit.size()-1){
            m_broadcast += stringSplit[i];
            m_broadcast += ".";
        }
        
    }

    m_broadcast+="255";
    
    ofLogNotice() <<"UdpManager::setupIP -> Broadcast IP address: " << m_broadcast;
    
}

void UdpManager::update()
{
    this->updateReveivePackage();
    
    if(m_refreshPixels){
        this->updatePixels();
        
    }
    else if(m_refreshTime){
        this->updateTime();
    }
    
    m_timer.update();
    
}

void UdpManager::updatePixels()
{
    if(!m_connected){
        return;
    }
    
    
    const auto & ledGroups = AppManager::getInstance().getLedsManager().getLedGroups();
    for(auto& group: ledGroups)
    {
        
        const auto & pixels = group.second->getColors();
        unsigned short division = pixels.size()/m_maxNumPixelsPerPacket;
        unsigned short remainder = pixels.size()%m_maxNumPixelsPerPacket;
        unsigned short offset = 0;
        
        
        
        
        string message = this->getDataHeader(m_maxNumPixelsPerPacket);
        message+=this->getDataPayload(group.second->getChannel(), offset,m_maxNumPixelsPerPacket,pixels);
        m_udpConnection.Send(message.c_str(),message.length());
        offset+=m_maxNumPixelsPerPacket;
        //this->printHex(message);
        
        m_udpConnection.Send(message.c_str(),message.length());
        
//        for(int i=0; i<division; i++){
//
//            string message = this->getDataHeader(m_maxNumPixelsPerPacket);
//            message+=this->getDataPayload(group.second->getChannel()+offset, offset,m_maxNumPixelsPerPacket,pixels);
//            m_udpConnection.Send(message.c_str(),message.length());
//            offset+=m_maxNumPixelsPerPacket;
//            //this->printHex(message);
//
//            m_udpConnection.Send(message.c_str(),message.length());
//        }
//
//        if(remainder!=0)
//        {
//            string message = this->getDataHeader(remainder);
//            message+=this->getDataPayload(group.second->getChannel()+offset,offset,remainder,pixels);
//            m_udpConnection.Send(message.c_str(),message.length());
//            //this->printHex(message);
//        }
    }
    
    
    m_refreshTime = true;
    m_refreshPixels = false;
}

void UdpManager::updateReveivePackage()
{
    char udpMessage[UDP_MESSAGE_LENGHT];
    m_udpConnection.Receive(udpMessage,UDP_MESSAGE_LENGHT);
    string message=udpMessage;
    
    if(message!="")
    {
        ofLogNotice() <<"UdpManager::updateReveivePackage -> SIZE " << message.size();
        ofLogNotice() <<"UdpManager::updateReveivePackage -> message " << message;
        
        this->parseMessage(udpMessage, UDP_MESSAGE_LENGHT);
    }
}

void UdpManager::updateTime()
{
    //ofLogNotice() <<"UdpManager::updateTime -> " << ofGetElapsedTimeMillis() << "ms";
    
    this->sendTime();
    //m_udpConnection.Send(message.c_str(),message.length());
    m_frameNumber++;
    m_refreshTime = false;
    m_refreshPixels = true;
}


bool UdpManager::isMessage(char * buffer, int size)
{
    if(buffer[0] != m_connectHeader.f1  && buffer[1] != m_connectHeader.f2  && buffer[2] != m_connectHeader.f3 ){
        ofLogNotice() <<"UdpManager::isMessage -> FALSE ";
        return false;
    }
    
    
    ofLogNotice() <<"UdpManager::isMessage -> TRUE ";
    return true;
}

string UdpManager::getDataHeader(unsigned short num_pixels)
{
    unsigned short ledsPerPixel = 3;
    unsigned short data_header_size = 6;
    
    string message="";
    message+= m_dataHeader.f1; message+= m_dataHeader.f2; message+= m_dataHeader.f3;
    m_dataHeader.payload_size = ledsPerPixel*num_pixels + data_header_size;
    unsigned char * s = (unsigned char*)& m_dataHeader.payload_size;
    message+= s[1] ;  message+=  s[0];
    s = (unsigned char*)& m_dataHeader.command;
    message+= s[1] ;  message+=  s[0];


    return message;
}

string UdpManager::getDataPayload(unsigned short channel, unsigned short offset, unsigned short num_pixels, const vector<ofFloatColor>& pixels)
{
    string message="";
    
    unsigned char * s = (unsigned char*)& channel;
    message+= s[1] ;  message+=  s[0];
    
    s = (unsigned char*)& offset;
    message+= s[1] ;  message+=  s[0];
    
    s = (unsigned char*)& num_pixels;
    message+= s[1] ;  message+=  s[0];
    
    for(int j=0; j< num_pixels; j++)
    {
        message+=pixels[offset+j].r*255;
        message+=pixels[offset+j].g*255;
        message+=pixels[offset+j].b*255;
    }
    
    return message;
}

void UdpManager::parseMessage(char * buffer, int size)
{
    if(isMessage(buffer, size))
    {
        if(buffer[6] == m_connectHeader.command)
        {
            ofLogNotice() <<"UdpManager::isMessage -> Received Connect Command: " << m_connectHeader.command;
            string ip; int port;
            m_udpConnection.GetRemoteAddr(ip, port);
            int portSend = AppManager::getInstance().getSettingsManager().getUdpPortSend();
            this->createConnection(ip, portSend );
            this->sendConnected();
            m_timer.stop();
        }
        
    }
    
}

void UdpManager::timerCompleteHandler( int &args )
{
    m_timer.start(false);
    //  cout<<"TIMER COMPLETED"<<endl;
    this->sendAutodiscovery();
}

void UdpManager::sendConnected()
{
    string message="";
    message+= m_connectHeader.f1; message+= m_connectHeader.f2; message+= m_connectHeader.f3;
    unsigned char * s = (unsigned char*)& m_connectHeader.payload_size;
    message+= s[1] ;  message+=  s[0];
    s = (unsigned char*)& m_connectHeader.command;
    message+= s[1] ;  message+=  s[0];
    message+='c';
    
    m_udpConnection.Send(message.c_str(),message.length());
    
    ofLogNotice() <<"UdpManager::sendConnected -> Send Connected ";
}

void UdpManager::sendTime()
{
    string message="";
    message+= m_timeHeader.f1; message+= m_timeHeader.f2; message+= m_timeHeader.f3;
    unsigned char * s = (unsigned char*)& m_timeHeader.payload_size;
    message+= s[1] ;  message+=  s[0];
    s = (unsigned char*)& m_timeHeader.command;
    message+= s[1] ;  message+=  s[0];
    s = (unsigned char*)& m_frameNumber;
    message+= s[3];  message+=  s[2];message+= s[1] ;  message+=  s[0];
    
    m_udpConnection.Send(message.c_str(),message.length());
    
    //ofLogNotice() <<"UdpManager::sendConnected -> Send Time ";
}

void UdpManager::sendAutodiscovery()
{
    string message="";
    message+= m_connectHeader.f1; message+= m_connectHeader.f2; message+= m_connectHeader.f3;
    unsigned char * s = (unsigned char*)& m_autodiscoveryHeader.payload_size;
    message+= s[1] ;  message+=  s[0];
    s = (unsigned char*)& m_autodiscoveryHeader.command;
    message+= s[1] ;  message+=  s[0];
    message+='a';
    
    m_udpConnection.Send(message.c_str(),message.length());
    
    ofLogNotice() <<"UdpManager::sendConnected -> Send Autodiscovery ";
}

void UdpManager::printHex(const string& message)
{
    std::stringstream ss;
    for(int i=0; i<message.length(); ++i){
        ss << std::hex << (int)message[i] << " ";
    }
    std::string mystr = ss.str();
    
    ofLogNotice() <<"UdpManager::printHex ->  hex: " << mystr;
}

void UdpManager::setMaxDataPacketSize(int& value )
{
    m_maxDataPacketSize = (unsigned short) value;
    m_maxNumPixelsPerPacket = (m_maxDataPacketSize-DATA_HEADER_OVERHEAD)/3;
    ofLogNotice() <<"UdpManager::setMaxDataPacketSize -> num pixels: " << m_maxNumPixelsPerPacket;
}






