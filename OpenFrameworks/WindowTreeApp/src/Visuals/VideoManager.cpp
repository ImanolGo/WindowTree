/*
 *  VideoManager.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */



#include "ofMain.h"

#include "VideoManager.h"
#include "AppManager.h"
#include "natural_sort.hpp"



VideoManager::VideoManager(): Manager(), m_frameNumber(-1), m_blurScale(0), m_currentVideoIndex(0), m_status(false), m_playing(false)
{
    //m_videoPlayer = new ofxAVFVideoPlayer();
}


VideoManager::~VideoManager()
{
    ofLogNotice() <<"VideoManager::Destructor" ;
}


void VideoManager::setup()
{
    if(m_initialized)
        return;
    
    
    Manager::setup();
    
    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
    float height  = AppManager::getInstance().getSettingsManager().getAppHeight();
    
    this->setupFbo();
    this->setupShaders(width,height);
    
    ofLogNotice() <<"VideoManager::initialized" ;
    
}



void VideoManager::loadVideos(string& path)
{
    ofDirectory dir(path);
    if( !dir.exists()){
        ofLogNotice() <<"VideoManager::loadVideos -> Folder not found: " << path;
        return;
    }
    
     //only show video files
    dir.allowExt("mp4");
    dir.allowExt("mov");
    
//    if( dir.listDir() == 0){
//        ofLogNotice() <<"VideoManager::loadVideos -> No video files found in: " << path;
//        return;
//    }
    
    dir.sort();
   
    m_videoNames.clear();
    m_videoResourcesPath.clear();
    
    
    vector<string> paths;
    vector<string> names;
    
    for(int i = 0; i < dir.size(); i++)
    {
        paths.push_back( dir.getPath(i));
        names.push_back( dir.getName(i));
    }
    
    SI::natural::sort(paths);
    SI::natural::sort(names);
    
//    sort(paths.begin(), paths.end());
//    sort(names.begin(), names.end());
    
    //go through and print out all the paths
    for(int i = 0; i < dir.size(); i++){
        string path = paths[i];
        string name = names[i];
    
        m_videoNames.push_back(name);
        m_videoResourcesPath[name] = path;
        ofLogNotice() <<"VideoManager::loadVideos -> Loaded name: " << name;
        ofLogNotice() <<"VideoManager::loadVideos -> Loaded path: " << path;
    }
    

    
    AppManager::getInstance().getSceneManager().addVideos();
}


void VideoManager::setupFbo()
{
    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
    float height  = AppManager::getInstance().getSettingsManager().getAppHeight();
    
    m_fbo.allocate(width,height);
    m_fbo.begin(); ofClear(0); m_fbo.end();
    
    m_exportFbo.allocate(width,height, GL_RGB);
    m_exportFbo.begin(); ofClear(0); m_exportFbo.end();
    
}

void VideoManager::setupShaders(float width,float height)
{
    this->setupLevels(width, height);
    this->setupBlur(width, height);
}

void VideoManager::setupLevels(float width, float height)
{
    m_levels.setup(width,height);
}

void VideoManager::setupBlur(float width, float height)
{
    m_blur.setup(width,height, 10, .2, 4);
    m_blur.setScale(m_blurScale);
}

void VideoManager::setBlurScale(float& value)
{
    m_blurScale = value;
    m_blur.setScale(m_blurScale);
}


bool VideoManager::isVideo(string& name_)
{
    for (auto name: m_videoNames){
        if(name_ == name){
            return true;
        }
    }
    
    return false;
}
void VideoManager::load(string& name_)
{
    if(!this->isVideo(name_)){
        return;
    }
    
    
    if(m_videoResourcesPath.find(name_) == m_videoResourcesPath.end()){
         ofLogNotice() <<"VideoManager::load ->name not found " << name_;
         return;
    }
    
    string path = m_videoResourcesPath.at(name_);
    
    if(m_videoPlayer.load(path))
    {
        m_exportFbo.allocate(m_videoPlayer.getWidth()*0.5, m_videoPlayer.getHeight()*0.5, GL_RGB);
        m_exportFbo.begin();  ofClear(0); m_exportFbo.end();
        
        
        if(m_status){
             m_videoPlayer.play();
             m_videoPlayer.update();
             m_frameNumber =  m_videoPlayer.getCurrentFrame();
             m_playing = false;
        }
       
        
        this->setupShaders(m_exportFbo.getWidth(), m_exportFbo.getHeight());
        
        
        ofLogNotice() <<"VideoManager::load -> successfully loaded: " << name_;
        
    }
    else{
        ofLogNotice() <<"VideoManager::load -> unable to load: " << name_;
    }
}




void VideoManager::updateVideo()
{
    if (!m_videoPlayer.isLoaded() || !m_status) {
        return;
    }
    
    m_videoPlayer.update();
    
    this->updateFbos();
    
    //m_frameNumber = frame;
    
    
    if(m_videoPlayer.isFrameNew())
    {

        int frame = m_videoPlayer.getCurrentFrame();

//        if(!m_playing){
//            if(frame < m_frameNumber || frame == 0){
//                m_playing = true;
//            }
//            else{
//                return;
//            }
//        }


        if(m_frameNumber != frame)
        {
            this->updateFbos();
            m_frameNumber = frame;
            ofPixels pixels;

            m_reader.readToPixels(m_exportFbo, pixels, OF_IMAGE_COLOR_ALPHA);
            //m_exportFbo.readToPixels(pixels);
            AppManager::getInstance().getLedsManager().setPixels(pixels);
        }
    }
}

void VideoManager::update()
{
    this->updateVideo();
}


void VideoManager::updateFbos()
{
    if (m_videoPlayer.isLoaded()) {
        m_exportFbo.begin();
            ofClear(0);
            this->drawVideo();
            //m_videoPlayer.draw(0, 0);
        m_exportFbo.end();
    }
}

void VideoManager::drawVideo()
{
    m_levels.begin();
    ofClear(0);
    m_videoPlayer.draw(0, m_exportFbo.getHeight(), m_exportFbo.getWidth(), -m_exportFbo.getHeight());
    m_levels.end();
    
    m_blur.begin();
    m_levels.draw();
    m_blur.end();
    
    m_blur.draw();
}





void VideoManager::draw()
{
	 this->drawFbo();
}

void VideoManager::drawFbo()
{
    string name = "Video";
    auto rect = AppManager::getInstance().getLayoutManager().getWindowRect(name);
    float ratio = m_videoPlayer.getWidth()/ m_videoPlayer.getHeight();
    float height = rect->getHeight();
    float width = height*ratio;
    
    if( width > rect->getWidth() ){
        width = rect->getWidth();
        height = width/ratio;
    }
    
    float x = rect->getWidth()*0.5 - width*0.5;
    float y = rect->getHeight()*0.5 - height*0.5;
    
    m_exportFbo.draw(x,y, width, height);
    
    //m_fbo.draw(x,y, width, height);
}

void VideoManager::draw(float width, float height)
{
    m_exportFbo.draw(0,0, width, height);
}


void VideoManager::setVideoIndex(int i)
{
    if(i<0 || i>= m_videoNames.size()){
        return;
    }
    
    ofLogNotice() <<"VideoManager::setVideoIndex ->  " << i;
    m_currentVideoIndex = i;
    this->load(m_videoNames[m_currentVideoIndex]);
    
}
void VideoManager::play()
{
   m_videoPlayer.play();
   m_status = true;
}

void VideoManager::next()
{
    if(m_videoNames.empty()){
        return;
    }
    
    ofLogNotice() <<"VideoManager::loadVideos -> Next ";
    
    m_currentVideoIndex = (m_currentVideoIndex + 1) % m_videoNames.size();
    this->setVideoIndex(m_currentVideoIndex);
}

void VideoManager::pause()
{
    m_videoPlayer.setPaused(true);
    m_status = false;
}

void VideoManager::stop()
{
    m_videoPlayer.stop();
    m_videoPlayer.setFrame(0);
    m_videoPlayer.closeMovie();
    m_videoPlayer.unbind();
    m_exportFbo.begin(); ofClear(0); m_exportFbo.end();
    ofPixels pixels;
    m_reader.readToPixels(m_exportFbo, pixels);
    //m_exportFbo.readToPixels(pixels);
    AppManager::getInstance().getLedsManager().setPixels(pixels);
    m_status = false;
}


void  VideoManager::setSpeed(float& value)
{
    m_videoPlayer.setSpeed(value);
}



