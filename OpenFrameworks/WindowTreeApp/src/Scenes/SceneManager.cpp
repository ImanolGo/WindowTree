/*
 *  SceneManager.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */

#include "ofMain.h"


#include "SceneManager.h"
#include "scenes.h"
#include "AppManager.h"

SceneManager::SceneManager(): Manager(), m_alpha(-1), m_transitionTime(0.5), m_sceneOffset(2), m_currentVideoIndex(0), m_status(false), m_useHueCorrection(false)
{
	//Intentionally left empty
}


SceneManager::~SceneManager()
{
    ofLogNotice() <<"SceneManager::Destructor";
}


void SceneManager::setup()
{
	if(m_initialized)
		return;

	Manager::setup();

    this->createScenes();
    this->setupFbo();
    this->setupLevels();
    this->setupShader();
   // this->setupTimer();

    ofLogNotice() <<"SceneManager::initialized";

}


void SceneManager::createScenes()
{
    m_mySceneManager = make_shared<ofxSceneManager>();
    //m_mySceneManager = shared_ptr<ofxSceneManager>(new ofxSceneManager());
    
    //std::shared_ptr<Lhs>(new Lhs("foo"));
    //m_mySceneManager->setTransitionFade();
	m_mySceneManager->setTransitionDissolve();
    
    ofPtr<ofxScene> scene;
    
     m_sceneOffset = 0;
    
    //Create Blank Scene
    scene = ofPtr<ofxScene> (new BlankScene());
    m_mySceneManager->addScene(scene);
    m_sceneOffset++;
    
    //Create Color Scene
    auto colorScene = ofPtr<ColorScene> (new ColorScene());
    colorScene->setup();
    m_mySceneManager->addScene(colorScene);
    m_sceneOffset++;
    
    //Create Test Scene
    auto sceneTest = ofPtr<TestScene> (new TestScene());
    sceneTest->setup();
    m_mySceneManager->addScene(sceneTest);
    m_sceneOffset++;
    
    
//    //Create shader Scene
//    auto shaderScene = ofPtr<ShaderScene> (new ShaderScene("NoiseShader"));
//    shaderScene->setup();
//    m_mySceneManager->addScene(shaderScene);
//    m_sceneOffset++;
//
//    //Create shader Scene
//    shaderScene = ofPtr<ShaderScene> (new ShaderScene("CirclesShader"));
//    shaderScene->setup();
//    m_mySceneManager->addScene(shaderScene);
//    m_sceneOffset++;
//
//    //Create shader Scene
//    shaderScene = ofPtr<ShaderScene> (new ShaderScene("SparklesShader"));
//    shaderScene->setup();
//    m_mySceneManager->addScene(shaderScene);
//    m_sceneOffset++;
//
//    //Create shader Scene
//    shaderScene = ofPtr<ShaderScene> (new ShaderScene("BreathShader"));
//    shaderScene->setup();
//    m_mySceneManager->addScene(shaderScene);
//    m_sceneOffset++;
//
//    //Create shader Scene
//    shaderScene = ofPtr<ShaderScene> (new ShaderScene("CloudsShader"));
//    shaderScene->setup();
//    m_mySceneManager->addScene(shaderScene);
//    m_sceneOffset++;


    
    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
    float height = AppManager::getInstance().getSettingsManager().getAppHeight();
    
    m_mySceneManager->run(width, height);
    this->onTransitionTimeChange(m_transitionTime);
    
    ofLogNotice() <<"SceneManager::createScenes-> w = " << width << ", h = " << height;
    ofLogNotice() <<"SceneManager::createScenes-> transition time = " << m_transitionTime << "s";
}


void SceneManager::setupFbo()
{
    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
    float height = AppManager::getInstance().getSettingsManager().getAppHeight();
    
    //float width = ofGetWidth();
    //float height = ofGetHeight();

    m_fbo.allocate(width, height, GL_RGBA);
    m_fbo.begin(); ofClear(0); m_fbo.end();
    
    m_fboColor.allocate(width, height, GL_RGBA);
    m_fboColor.begin(); ofClear(0); m_fboColor.end();
    
    
}

void SceneManager::setupLevels()
{
    float width = AppManager::getInstance().getSettingsManager().getAppWidth();
    float height = AppManager::getInstance().getSettingsManager().getAppHeight();
    
    m_levels.setup(width,height);
}

void SceneManager::setupShader()
{
    string path = "shaders/ColorHueShader";
    
    m_hueShader.unload();
    if(m_hueShader.load(path))
    {
        ofLogNotice() << "SceneManager::setupShader-> successfully loaded " << path;
    }
    else{
        ofLogNotice() << + "SceneManager::setupShader-> Cannot load " << path;
    }
}


void SceneManager::setupTimer()
{
    
    m_sceneTimer.setup( 1000 );
    m_sceneTimer.start( false ) ;
    ofAddListener( m_sceneTimer.TIMER_COMPLETE , this, &SceneManager::sceneTimerCompleteHandler ) ;
    
    ofLogNotice() <<"SceneManager::setupTimer << Time = : " << time << "s";
}

void SceneManager::onChangeSceneDuration(float& value)
{
    m_sceneTimer.setup( value*1000 );
    m_sceneTimer.start( false ) ;
    ofLogNotice() <<"SceneManager::setupTimer << Time = : " << value << "s";
}

void SceneManager::update()
{
    this->updateScenes();
    this->updateFbo();
    this->updatePixels();
    //this->updateTimer();
}

void SceneManager::updateFbo()
{
    m_levels.begin();
        ofClear(0);
        ofEnableAlphaBlending();
         m_mySceneManager->draw();
         ofDisableAlphaBlending();
    m_levels.end();
    
    m_fboColor.begin();
    ofClear(0,0,0);
       ofPushStyle();
        ofSetColor(255);
        ofEnableAlphaBlending();
            m_levels.draw();
        ofDisableAlphaBlending();
		ofPopStyle();
    m_fboColor.end();
    
    m_fbo.begin();
    ofClear(0,0,0);
    ofPushStyle();
    ofSetColor(255);
    ofEnableAlphaBlending();
    if(m_useHueCorrection){
        auto floatColor = AppManager::getInstance().getGuiManager().getSolidColor();
        m_hueShader.begin();
        m_hueShader.setUniform4f("color", floatColor);
        
    }
    
        m_fboColor.draw(0,0);
    
    if(m_useHueCorrection){
        m_hueShader.end();
    }
    ofDisableAlphaBlending();
    ofPopStyle();
    m_fbo.end();
}


void SceneManager::updatePixels()
{
    ofPixels pixels;
    m_reader.readToPixels(m_fbo, pixels, OF_IMAGE_COLOR_ALPHA);
    //m_fbo.readToPixels(pixels);
    AppManager::getInstance().getLedsManager().setPixels(pixels);
}


void SceneManager::updateScenes()
{
    m_mySceneManager->update();
}

void SceneManager::updateTimer()
{
    m_sceneTimer.update();
}


void SceneManager::draw()
{
    string name = "Scene";
    ofRectangle frame;
    auto rect = AppManager::getInstance().getLayoutManager().getWindowRect(name);
    float ratio = m_fbo.getWidth()/ m_fbo.getHeight();
    frame.height = rect->getHeight();
    frame.width = frame.height*ratio;
    
    if( frame.width > rect->getWidth() ){
        frame.width = rect->getWidth();
        frame.height = frame.width/ratio;
    }
    
    frame.x = rect->getWidth()*0.5 - frame.width*0.5;
    frame.y = rect->getHeight()*0.5 - frame.height*0.5;
    
    m_fbo.draw(frame);
}


void SceneManager::draw(const ofRectangle& rect)
{
    m_fbo.draw(rect.x,rect.y,rect.width,rect.height);
}


void SceneManager::removeVideos()
{
    ofLogNotice() <<"SceneManager::removeVideos";

    int index = 0;
    //this->changeSceneIndex(index);
    //AppManager::getInstance().getGuiManager().onSceneChange(0);
    //changeSceneIndex
    
    auto& videoPaths = AppManager::getInstance().getVideoManager().getVideoResourcesPath();
    auto scenes = m_mySceneManager->scenes;

    for(int i=m_sceneOffset ; i < scenes.size(); i++){
        ofLogNotice() <<"SceneManager::remove video -> " << scenes[i]->getName();
        m_mySceneManager->removeScene(scenes[i]);
    }
}


void SceneManager::removeScenes()
{
    ofLogNotice() <<"SceneManager::removeScenes";
    
    int index = 0;
    //this->changeSceneIndex(index);
    //AppManager::getInstance().getGuiManager().onSceneChange(0);
    //changeSceneIndex
    
    auto& videoPaths = AppManager::getInstance().getVideoManager().getVideoResourcesPath();
    auto scenes = m_mySceneManager->scenes;
    
    for(int i=0 ; i < scenes.size(); i++){
        ofLogNotice() <<"SceneManager::remove video -> " << scenes[i]->getName();
        m_mySceneManager->removeScene(scenes[i]);
    }
}


void SceneManager::addVideos()
{
     
    //this->removeVideos();
    this->removeScenes();
    this->createScenes();
    
    auto& videoPaths = AppManager::getInstance().getVideoManager().getVideoNames();
  
    for(auto& path : videoPaths){
        auto videoScene = ofPtr<VideoScene>(new VideoScene(path));
        videoScene->setup();
        ofLogNotice() <<"SceneManager::add video scene -> " << path;
        m_mySceneManager->addScene(videoScene);
    }
    
    AppManager::getInstance().getGuiManager().setupScenesGui();
}



void SceneManager::changeScene(string sceneName)
{
    for(auto scene: m_mySceneManager->scenes){
        if(scene->getName() == sceneName){
            ofLogNotice() <<"SceneManager::changeScene -> " << sceneName;
            m_mySceneManager->changeScene(sceneName);
            m_sceneTimer.start(false,true);
            m_currentSceneName = sceneName;
            //this->sendSceneChange();
            
            break;
        }
    }
}

void SceneManager::setVideoIndex(int value)
{
    int size = m_mySceneManager->scenes.size() - m_sceneOffset;
    if(value >= 0  &&  value<size){
        m_currentVideoIndex = value;
    }
}

void SceneManager::changeSceneIndex(int& sceneIndex)
{
    if(sceneIndex < 0 || sceneIndex >= m_mySceneManager->scenes.size()){
        return;
    }
    
    ofLogNotice() <<"SceneManager::changeSceneIndex << scene index " << sceneIndex;

     m_mySceneManager->changeScene(sceneIndex);
     m_sceneTimer.start(false,true);
     m_currentSceneName = this->getSceneName(sceneIndex);
    
     ofLogNotice() <<"SceneManager::changeSceneIndex << scene name " << m_currentSceneName;
    
    
    if(sceneIndex < m_sceneOffset){
        m_status = false;
    }
    else{
         m_status = true;
        m_currentVideoIndex = sceneIndex - m_sceneOffset;
    }
}


void SceneManager::onTransitionTimeChange(float& value)
{
   m_transitionTime = value;
   m_mySceneManager->setSceneDuration(m_transitionTime,m_transitionTime);
   m_sceneTimer.start(false,true);
    
}

string SceneManager::getSceneName(int sceneIndex)
{
    string name = "";
    if(sceneIndex < 0 || sceneIndex >= m_mySceneManager->scenes.size()){
        return name;
    }
    
    return m_mySceneManager->scenes[sceneIndex]->getName();
   
}

int SceneManager::getIndex(string& sceneName)
{
    for(int i = 0; i< m_mySceneManager->scenes.size(); i++){
        if(m_mySceneManager->scenes[i]->getName() == sceneName){
            return i;
        }
    }
    
    return -1;
}

void SceneManager::sceneTimerCompleteHandler( int &args )
{
    m_sceneTimer.start(false);

}


void SceneManager::sendSceneChange()
{
    
    string address = "preset" + m_currentSceneName;
    
    ofLogNotice() <<"SceneManager::sendSceneChange << OSC Message Scene: " << address;
    
   // AppManager::getInstance().getOscManager().sendIntMessage(1, address);
  
}




int SceneManager::getNumberScenes(){
    
    if(m_mySceneManager){
        return m_mySceneManager->scenes.size();
    }
    return 0;
}
