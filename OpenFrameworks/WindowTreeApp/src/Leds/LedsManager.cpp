/*
 *  LedsManager.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */



#include "ofMain.h"

#include "LedsManager.h"
#include "AppManager.h"


const string LedsManager::LEDS_FOLDER_PATH = "leds/";



LedsManager::LedsManager(): Manager(), m_isNewFrame(false), m_is3D(true), m_ledsBrightness(1.0),  m_drawGrid(true)
{
	//Intentionally left empty
}


LedsManager::~LedsManager()
{
    ofLogNotice() <<"LedsManager::Destructor" ;
}


void LedsManager::setup()
{
	if(m_initialized)
		return;


	Manager::setup();
    
    this->setupLeds();
    this->setupShader();
    
    ofLogNotice() <<"LedsManager::initialized" ;
    
}


void LedsManager::setupLeds()
{
    this->createLedPositions();
    this->readLeds();
    this->normalizeLeds();
    this->createLayout();
    this->map2DpositionsToFbo();
}



void LedsManager::createLedPositions()
{
    ofBuffer buffer;
    
    int num_rows = 90;
    int num_cols = 60;
    float ratio = float(num_cols)/num_rows;
    float width = 20;
    float height = width/ratio;
    float x_step = width/num_cols;
    float y_step = height/num_rows;
    
    for(int i = 0; i<num_rows; i++){
        for(int j = 0; j<num_cols; j++)
        {
            float x = j*x_step - width*0.5;
            float y = i*y_step - height*0.5;
            glm::vec3 pos (x,y, 0.0) ;
            string line = ofToString(pos.x) + "," + ofToString(pos.y) + "," + ofToString(pos.z) + "\n";
            buffer.append(line);
        }

    }
    
    
    string path2d = LEDS_FOLDER_PATH + "frame/frame_POINTS_2D.txt";
    string path3d = LEDS_FOLDER_PATH + "frame/frame_POINTS_3D.txt";

    bool fileWritten = ofBufferToFile(path2d, buffer);
    if(fileWritten){
        ofLogNotice() <<"LedsManager::createLedPositions -> saved buffer to: " <<  path2d;
    }
    else{
        ofLogNotice() <<"LedsManager::createLedPositions -> couldn't save buffer to: " <<  path2d;
    }
    
    fileWritten = ofBufferToFile(path3d, buffer);
    if(fileWritten){
        ofLogNotice() <<"LedsManager::createLedPositions -> saved buffer to: " <<  path3d;
    }
    else{
        ofLogNotice() <<"LedsManager::createLedPositions -> couldn't save buffer to: " <<  path3d;
    }
   
    
    
}

bool LedsManager::readLeds()
{
    
    ofDirectory dir(LEDS_FOLDER_PATH);
    if( !dir.exists()){
        ofLogNotice() <<"LedsManager::readLeds -> Folder not found: " << LEDS_FOLDER_PATH;
        return false;
    }
    
    dir.listDir();
    
    ofLogNotice() <<"LedsManager::readLeds-> directory size: " << dir.size();
    
    
    this->clearAll();
    
    for(int i = 0; i < dir.size(); i++)
    {
        string name =  dir.getName(i);
        ofLogNotice() <<"LedsManager::readLeds-> sub-directory name: " << name;
        auto ledGroup = shared_ptr<LedGroup>(new LedGroup(name));
        if(ledGroup->setup()){
            m_ledGroups[name] = ledGroup;
        }
    }
    
    return true;
}


void LedsManager::normalizeLeds()
{
    if(m_ledGroups.empty()){
        return;
    }
    
    auto box = m_ledGroups.begin()->second->getBoundingBox();
    
    glm::vec2 min = box.getMin();
    glm::vec2 max = box.getMax();
    
    for(auto & ledGroup: m_ledGroups){
        box = ledGroup.second->getBoundingBox();
        if(min.x > box.getMinX()){
            min.x = box.getMinX();
        }
        if(min.y > box.getMinY()){
            min.y =  box.getMinY();
        }
        if(max.x < box.getMaxX()){
            max.x =  box.getMaxX();
        }
        if(max.y < box.getMaxY()){
            max.y =  box.getMaxY();
        }
    }
    
    m_boundingBox.set(min, max);
    
    
    //Add margin
    float scalingFactor = 1.02;
    m_boundingBox.scaleFromCenter(scalingFactor);
    
    for(auto & ledGroup: m_ledGroups){
        ledGroup.second->setBoundingBox(m_boundingBox);
    }
    
}

void LedsManager::setupShader()
{
    ofLogNotice() <<"LedsManager::setupShader ";
    m_maskShader.load("shaders/LuminanceMaskingShader");
}

void LedsManager::createLayout()
{
    float resolution = 1500;
    float percentage = 1.0;
    float width = m_boundingBox.getWidth();
    float height = m_boundingBox.getHeight();
    float ratio = width/height;
    if(ratio > 1.0){
        width = resolution;
        height = width/ratio;
    }
    else{
        height = resolution;
        width = height*ratio;
    }
    
   
    ofPixels pix;
    
    ofFbo fbo;
    fbo.allocate(width, height);
    fbo.begin();
    ofClear(0, 0, 0, 255);
    ofSetColor(255);
    
    float size = 1;
    
    for(auto & ledGroup: m_ledGroups){
        auto & positions = ledGroup.second->get2dPositions();
        for(auto led: positions){
            float x = ofMap(led.x, m_boundingBox.getMinX(), m_boundingBox.getMaxX(), 0.0, width-1);
            float y = ofMap(led.y, m_boundingBox.getMinY(), m_boundingBox.getMaxY(), height-1,0);
            ofDrawRectangle(x - size*0.5, y - size*0.5, size, size);
        }
    }
   
   
    fbo.end();
    
    fbo.readToPixels(pix);
    ofSaveImage(pix, "images/layout/leds_layout.png");
    
    width*=percentage;
    height*=percentage;
    
    m_fboMask.allocate(width, height, GL_RGB);
    m_fbo.allocate(width, height, GL_RGB);
    m_fboMaskee.allocate(width, height, GL_RGB);
    
    size = 4;
    m_fboMask.begin();
    ofSetColor(255);
    
    for(auto & ledGroup: m_ledGroups){
        auto & positions = ledGroup.second->get2dPositions();
        for(auto led: positions){
            float x = ofMap(led.x, m_boundingBox.getMinX(), m_boundingBox.getMaxX(), 0.0, width-1);
            float y = ofMap(led.y, m_boundingBox.getMinY(), m_boundingBox.getMaxY(), height-1,0);
            ofDrawRectangle(x - size*0.5, y - size*0.5, size, size);
        }
    }
    
    m_fboMask.end();
    
    //ofLogNotice() <<"LedsManager::createLayout: saved layout -> w = " << width << ", h = " << height << ", ratio = " << ratio;
    ofLogNotice() <<"LedsManager::createLayout: new app width -> w = " << width << ", new app height = " << height << ", ratio = " << ratio;
    AppManager::getInstance().getSettingsManager().setAppWidth(width);
    AppManager::getInstance().getSettingsManager().setAppHeight(height);
    
    
    
}

void LedsManager::map2DpositionsToFbo()
{
//    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
//    float height = AppManager::getInstance().getSettingsManager().getAppHeight();
//    for (auto& position: m_points2D)
//    {
//        float x = ofMap(position.x, m_minPos.x, m_maxPos.x, 0.0, width-1);
//        float y = ofMap(position.y, m_minPos.y, m_maxPos.y, height-1, 0);
//        position.x = x;
//        position.y = y;
//    }
}



void LedsManager::update()
{
    this->updateLeds();
}


void LedsManager::updateLeds()
{
    if(m_isNewFrame){
        m_isNewFrame = false;
    }
    
}

void LedsManager::setPixels(ofPixelsRef pixels)
{
    for(auto& groups: m_ledGroups){
        groups.second->setPixels(pixels);
    }
    
    m_isNewFrame = true;
}

void LedsManager::draw()
{
    
    if(m_drawGrid){
        float size = 5.0;
        float numSteps = 10;
        this->drawGrid(size, numSteps, false, true, false);
    }
    
   
    for(auto& groups: m_ledGroups){
        groups.second->draw();
    }
}

void LedsManager::drawModel()
{
    for(auto& groups: m_ledGroups){
        groups.second->drawModel();
    }
}

void LedsManager::drawLayout()
{
    ofEnableAlphaBlending();
    
    ofRectangle rect1(0,0,m_fboMask.getWidth(), m_fboMask.getHeight());
    m_fboMaskee.begin();
        ofClear(0);
        AppManager::getInstance().getSceneManager().draw(rect1);
    m_fboMaskee.end();
    
    
    m_fbo.begin();
        ofClear(0);
        m_maskShader.begin();
        m_maskShader.setUniformTexture("imageMask", m_fboMask.getTexture(), 1);
        m_fboMaskee.draw(0,0);
        m_maskShader.end();
    m_fbo.end();
    
    string name = "2D";
    auto rect = AppManager::getInstance().getLayoutManager().getWindowRect(name);
    float ratio = m_fboMask.getWidth()/ m_fboMask.getHeight();
    float height = rect->getHeight();
    float width = height*ratio;
    
    if( width > rect->getWidth() ){
        width = rect->getWidth();
        height = width/ratio;
    }
    
    float x = rect->getWidth()*0.5 - width*0.5;
    float y = rect->getHeight()*0.5 - height*0.5;
    
    m_fbo.draw(x,y, width, height);
    
}


void LedsManager::drawGrid(float stepSize, size_t numberOfSteps,  bool x, bool y, bool z) const
{
    if (x) {
        this->drawGridPlane(stepSize, numberOfSteps);
    }
    if (y) {
        glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::half_pi<float>(), glm::vec3(0,0,-1));
        ofPushMatrix();
        ofMultMatrix(m);
        this->drawGridPlane(stepSize, numberOfSteps);
        ofPopMatrix();
    }
    if (z) {
        glm::mat4 m = glm::rotate(glm::mat4(1.0), glm::half_pi<float>(), glm::vec3(0,1,0));
        ofPushMatrix();
        ofMultMatrix(m);
        this->drawGridPlane(stepSize, numberOfSteps);
        ofPopMatrix();
    }
    
    
    ofDrawAxis(stepSize*numberOfSteps);
}


void LedsManager::drawGridPlane(float stepSize, size_t numberOfSteps) const
{
    ofPushStyle();
    ofSetColor(50);
    float scale = stepSize * numberOfSteps;
    
    for (int iDimension=0; iDimension<2; iDimension++)
    {
        for (size_t i=0; i <= numberOfSteps; i++)
        {
            float yz = i * stepSize;
            
            if (iDimension == 0 ) {
                ofDrawLine(0, yz, -scale, 0, yz, scale);
                if (yz !=0) ofDrawLine(0, -yz, -scale, 0, -yz, scale);
            } else {
                ofDrawLine(0, -scale, yz, 0, scale, yz);
                if (yz !=0) ofDrawLine(0, -scale, -yz, 0, scale, -yz);
            }
        }
    }
    
     ofPopStyle();
}

void LedsManager::setSize(float& value)
{
    for(auto& groups: m_ledGroups){
        groups.second->setSize(value);
    }
}



void LedsManager::clearAll()
{
    for(auto& groups: m_ledGroups){
        groups.second->clear();
    }
}







