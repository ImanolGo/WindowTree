/*
 *  LedGroup.h
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */



#include "LedGroup.h"
#include "AppManager.h"

LedGroup::LedGroup(string& name, unsigned char channel): m_name(name), m_sizeScale(1.0), m_offset(2.5), m_channel(channel)
{
    //Intentionaly left empty
}

LedGroup::~LedGroup()
{
    //Intentionaly left empty
}

bool LedGroup::setup()
{
    if(this->readLeds())
    {
        this->arrangeLeds();
        this->setupModel();
        this->setupShader();
        return true;
    }
    return false;
}

bool LedGroup::readLeds()
{
    string path =  "leds/" + m_name;
    ofDirectory dir(path);
    
    if( !dir.exists()){
        ofLogNotice() <<"LedGroup::readLeds -> Folder not found: " << path;
        return false;
    }
    
    dir.listDir();
    dir.sort();
    dir.allowExt("txt");//only show txt files
    
    if( dir.listDir() == 0){
        ofLogNotice() <<"LedGroup::setupLeds -> No led files found in: " << dir.getAbsolutePath();
        return false;
    }
    
    ofLogNotice()<< "LedGroup::loadLedSubfolder-> Path: " << dir.getAbsolutePath();
    ofLogNotice()<< "LedGroup::loadLedSubfolder-> Size: " << dir.size();
    
    string twodfile = "";
    string threedfile = "";
    
    //go through and print out all the paths
    for(int i = 0; i < dir.size(); i++){
        string path = dir.getPath(i);
        if(ofIsStringInString(path, "POINTS_2D")){
            twodfile = path;
        }
        
        if(ofIsStringInString(path, "POINTS_3D")){
            threedfile = path;
        }
    }
    
    if(twodfile.empty() || threedfile.empty()){
        ofLogNotice()<< "LedGroup::loadLedSubfolder-> No position's pair found ";
        return false;
    }
    
    return this->loadLedPair(twodfile, threedfile);
}


bool LedGroup::loadLedPair(string& pathTwoD, string& pathThreeD)
{
    bool success = isValidLedFile(pathTwoD) && isValidLedFile(pathThreeD) ;
    if(success){
        return this->addLedPair(pathTwoD,pathThreeD);
    }
    
    return success;
}


bool LedGroup::addLedPair(string& pathTwoD, string& pathThreeD)
{
    ofBuffer buffer2D = ofBufferFromFile(pathTwoD);
    ofBuffer buffer3D = ofBufferFromFile(pathThreeD);
    
    ofLogNotice() <<"LedGroup::addLedPair -> buffer2D: " << pathTwoD ;
    ofLogNotice() <<"LedGroup::addLedPair -> buffer3D: " << pathThreeD ;
    
    if(buffer2D.size()== 0 || buffer2D.size()== 0){
        ofLogNotice() <<"LedGroup::addLedPair -> zero buffer size" ;
        return false;
    }
    
    ofBuffer::Line it2d = buffer2D.getLines().begin();
    ofBuffer::Line it3d = buffer3D.getLines().begin();
    
    glm::vec3 ledPosition2D;
    glm::vec3 ledPosition3D;
    int id = 0;
    
    while(it2d != buffer2D.getLines().end() ||  it3d != buffer3D.getLines().end()){
        
        string line2D = *it2d;
        string line3D = *it3d;
        
        if(!line2D.empty() && parseLedLine(line2D,ledPosition2D) &&  !line3D.empty() && parseLedLine(line3D,ledPosition3D))
        {
            ledPosition2D.y = ledPosition2D.z;
            ledPosition2D.z = 0.0;
            this->createLedPair(ledPosition2D, ledPosition3D);
        }
        
        ++it2d; ++it3d;
        id++;
    }
    
    return true;
    
}


void LedGroup::draw()
{
    m_vboShader.begin();
    m_texture.bind();
    m_vbo.draw(GL_POINTS, 0, (int)m_points3D.size());
    m_texture.unbind();
    m_vboShader.end();
}

void LedGroup::drawModel()
{
    m_vboShader.begin();
    m_textureModel.bind();
    m_vboModel.draw(GL_POINTS, 0, (int)m_points3D.size());
    m_textureModel.unbind();
    m_vboShader.end();
}

void LedGroup::setupShader()
{
    ofLogNotice() <<"LedGroup::setupShader ";
    ofFile file;
    string texture1 = "images/leds/" + m_name + ".png";
    if(!file.doesFileExist(texture1)){
       texture1  = "images/general/dot.png";
    }
    
    string texture2 = "images/leds/" + m_name + "_black.png";
    if(!file.doesFileExist(texture2)){
        texture2  = "images/general/black_dot.png";
    }
    
    m_vboShader.load("shaders/vboShader");
    
    ofDisableArbTex();
    ofLoadImage(m_texture, texture1);
    ofLoadImage(m_textureModel, texture2);
    ofEnableArbTex();
}

void LedGroup::arrangeLeds()
{
    this->normalizeLeds();
    this->centreLeds();
}

void LedGroup::setupModel()
{
    
    int total = (int)m_points3D.size();
    m_vbo.setVertexData(&m_points3D[0], total, GL_DYNAMIC_DRAW);
    m_vbo.setNormalData(&m_sizes[0], total, GL_DYNAMIC_DRAW);
    m_vbo.setColorData(&m_colors[0], m_points3D.size(), GL_DYNAMIC_DRAW);
    
    
    m_vboModel.setVertexData(&m_points3D[0], total, GL_DYNAMIC_DRAW);
    m_vboModel.setNormalData(&m_sizesModel[0], total, GL_DYNAMIC_DRAW);
    m_vboModel.setColorData(&m_colorsBlack[0], m_points3D.size(), GL_DYNAMIC_DRAW);
    
}


void LedGroup::clear()
{
    m_colors.clear();
    m_colorsBlack.clear();
    m_vbo.clear();
    m_vboModel.clear();
    m_points3D.clear();
    m_points2D.clear();
    m_sizes.clear();
    m_sizesModel.clear();
}

void LedGroup::setPixels(ofPixelsRef pixels)
{
    for(int i=0; i<m_points2D.size(); i++){
        this->setPixelColor(pixels, i);
    }
    
    m_vbo.setColorData(&m_colors[0], m_points3D.size(), GL_DYNAMIC_DRAW);
}

void LedGroup::setPixelColor(ofPixelsRef pixels, int index)
{
    if(index<0 || index>=m_points2D.size()){
        return;
    }
    
    if(!m_boundingBox.inside(m_points2D[index])){
        m_colors[index] = ofColor::black;
        return;
    }
    
    
    int x = (int) ofMap(m_points2D[index].x, m_boundingBox.getMinX(), m_boundingBox.getMaxX(), 0, pixels.getWidth()-1, true);
    int y = (int) ofMap(m_points2D[index].y, m_boundingBox.getMinY(), m_boundingBox.getMaxY(), 0, pixels.getHeight()-1, true);
    
    auto color = pixels.getColor(x, y);
    
    
//        if(index == 20){
//            ofLogNotice() << "pixelPos.x = " << x;
//            ofLogNotice() << "pixelPos.y = " << y;
//            ofLogNotice() << color;
//        }
    
    //m_colors[index] = ofFloatColor(color.r/255.0f, color.g/255.0f, color.b/255.0f);
    m_colors[index] = color;
}


bool LedGroup::isValidLedFile(const string& path)
{
    ofBuffer buffer = ofBufferFromFile(path);
    
    if(buffer.size()==0){
        ofLogNotice() <<"LedGroup::isValidLedFile -> empty file";
        return false;
    }
    
    glm::vec3 ledPosition;
    
    for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it)
    {
        
        string line = *it;
        
        //ofLogNotice() << line;
        
        // copy the line to draw later
        // make sure its not a empty line
        if(!line.empty() && !parseLedLine(line,ledPosition)) {
            ofLogNotice() <<"LedGroup::isValidLedFile -> File not valid";
            return false;
        }
        
        // print out the line
        //cout << line << endl;
        
    }
    
    return true;
}

bool LedGroup::parseLedLine(string& line, glm::vec3& position)
{
    if(line.size() == 0){
        return false;
    }
    
    
    vector <string> positionsStrings = ofSplitString(line, "," );
    
    if(positionsStrings.size()!=3){
        return false;
    }
    
    position.x = ofToFloat(positionsStrings[0]);
    position.z = -ofToFloat(positionsStrings[1]);
    position.y = ofToFloat(positionsStrings[2]);
    
    return true;
}

int LedGroup::createLedPair(const glm::vec3& position2D,const glm::vec3& position3D)
{
    float size = AppManager::getInstance().getGuiManager().getLedsSize();
    
    m_points3D.push_back(position3D);
    m_points2D.push_back(position2D);
    m_sizes.push_back(ofVec3f(size*m_sizeScale));
    m_sizesModel.push_back(ofVec3f(size*m_sizeScale));
    m_colors.push_back(ofFloatColor(0,0,0));
    m_colorsBlack.push_back(ofFloatColor(255,255,255));
 
    return m_colors.size();
}

void LedGroup::normalizeLeds()
{
    //this->normalize2DLeds();
    //this->normalize3DLeds();
}

void LedGroup::normalize2DLeds()
{
    float max = 0;
    for (auto position: m_points2D)
    {
        
        if(max < abs(position.x)){
            max = abs(position.x);
        }
        
        if(max < abs(position.y)){
            max = abs(position.y);
        }
    }
    
    ofLogNotice() <<"LedGroup::normalize2DLeds -> max value =  " << max;
    
    int id = 0;
    for (auto& position: m_points2D)
    {
        position/=max;
        ofLogNotice() <<"LedGroup::normalizeLeds -> id " << id << ", x = "  << position.x << ", y = "  << position.y ;
        id++;
    }
    
}


void LedGroup::normalize3DLeds()
{
    float max = 0;
    for (auto position: m_points3D)
    {
        
        if(max < abs(position.x)){
            max = abs(position.x);
        }
        
        if(max < abs(position.y)){
            max = abs(position.y);
        }
        
        if(max < abs(position.z)){
            max = abs(position.z);
        }
    }
    
    ofLogNotice() <<"LedGroup::normalize3DLeds -> max value =  " << max;
    
    int id = 0;
    for (auto& position: m_points3D)
    {
        position/=max;
        
        //ofLogNotice() <<"LedGroup::normalizeLeds -> id " << id << ", x = "  << position.x << ", y = "  << position.y << ", z = " << position.z ;
        id++;
    }
    
}

void LedGroup::centreLeds()
{
    this->centre2DLeds(2);
    this->centre3DLeds();
}

void LedGroup::centre3DLeds()
{
    
    ofPoint maxPos, minPos;
    bool firstIteration = true;
    
    for (auto position: m_points3D)
    {
        if(firstIteration){
            firstIteration = false;
            maxPos = position;
            minPos = position;
        }
        
        if(maxPos.x < position.x){
            maxPos.x = position.x;
        }
        
        if(maxPos.y < position.y){
            maxPos.y = position.y;
        }
        
        if(maxPos.z < position.z){
            maxPos.z = position.z;
        }
        
        if(minPos.x > position.x){
            minPos.x = position.x;
        }
        
        if(minPos.y > position.y){
            minPos.y = position.y;
        }
        
        if(minPos.z > position.z){
            minPos.z = position.z;
        }
        
    }
    
    ofLogNotice() <<"LedGroup::centreLeds -> min position: x = "  << minPos.x << ", y = "  << minPos.y << ", z = " << minPos.z ;
    ofLogNotice() <<"LedGroup::centreLeds -> max position: x = "  << maxPos.x << ", y = "  << maxPos.y << ", z = " << maxPos.z ;
    
    ofPoint shift = (maxPos- minPos)*0.5  + minPos;
    
    ofLogNotice() <<"LedGroup::centreLeds -> shift position: x = "  << shift.x << ", y = "  << shift.y << ", z = " << shift.z ;
    
    for (auto& position: m_points3D)
    {
       // position-=shift;
        position*=m_offset;
    }
    
    //    maxPos -=shift;
    //    minPos -=shift;
}


void LedGroup::setSize(float value)
{
    value*= m_sizeScale;
    for(auto& size: m_sizes){
        size = glm::vec3(value);
    }
    
    for(auto& size: m_sizesModel){
        size = glm::vec3(value);
    }
    
    m_vbo.setNormalData(&m_sizes[0], m_sizes.size(), GL_DYNAMIC_DRAW);
    m_vboModel.setNormalData(&m_sizesModel[0], m_sizesModel.size(), GL_DYNAMIC_DRAW);
}

void LedGroup::centre2DLeds(float margin_percentage)
{
    
    glm::vec2 maxPos, minPos;
    bool firstIteration = true;
    
    for (auto position: m_points2D)
    {
        if(firstIteration){
            firstIteration = false;
            maxPos = position;
            minPos = position;
        }
        
        if(maxPos.x < position.x){
            maxPos.x = position.x;
        }
        
        if(maxPos.y < position.y){
            maxPos.y = position.y;
        }
        
        
        if(minPos.x > position.x){
            minPos.x = position.x;
        }
        
        if(minPos.y > position.y){
            minPos.y = position.y;
        }
        
    }
    
    if(maxPos.x == minPos.x){
        maxPos.x +=1;
        minPos.x -=1;
    }
    
    if(maxPos.y == minPos.y){
        maxPos.y +=1;
        minPos.y -=1;
    }
    
    m_boundingBox.set(minPos, maxPos);
    
    ofLogNotice() <<"LedGroup::centreLeds -> min position: x = "  << m_boundingBox.getMinX() << ", y = "  << m_boundingBox.getMinY() ;
    ofLogNotice() <<"LedGroup::centreLeds -> max position: x = "  << m_boundingBox.getMaxX() << ", y = "  <<m_boundingBox.getMaxY();
    
//    //Add margin
//    float max_x = abs(maxPos.x - minPos.x);
//    float max_y = abs(maxPos.y - minPos.y);
//    float max = max_y;
//    if(max_x<max_y){
//        max = max_x;
//    }
//
//    float margin = max*margin_percentage/100.0;
//
//    maxPos.x+=margin;
//    maxPos.y+=margin;
//    minPos.x-=margin;
//    minPos.y-=margin;
//
//    glm::vec2 shift = minPos;
//
//    ofLogNotice() <<"LedGroup::centreLeds -> shift position: x = "  << shift.x << ", y = "  << shift.y;
//
//    for (auto& position: m_points2D)
//    {
//        position-=shift;
//    }
//
//
//    maxPos -=shift;
//    minPos -=shift;
//
//    ofLogNotice() <<"LedGroup::centreLeds -> min position: x = "  << minPos.x << ", y = "  << minPos.y ;
//    ofLogNotice() <<"LedGroup::centreLeds -> max position: x = "  << maxPos.x << ", y = "  << maxPos.y;
}




