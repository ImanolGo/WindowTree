/*
 *  GuiManager.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */


#include "ofMain.h"

#include "AppManager.h"

#include "GuiManager.h"
#include "GuiTheme.h"


const string GuiManager::GUI_SETTINGS_FILE_NAME = "xmls/GuiSettings.xml";
const string GuiManager::GUI_SETTINGS_NAME = "GUI";
const int GuiManager::GUI_WIDTH = 300;


GuiManager::GuiManager(): Manager(), m_showGui(true)
{
    //Intentionally left empty
}


GuiManager::~GuiManager()
{
    this->saveGuiValues();
    ofLogNotice() <<"GuiManager::Destructor";
}


void GuiManager::setup()
{
    if(m_initialized)
        return;
    
    Manager::setup();
    
    
    this->setupGuiParameters();
    this->setupScenesGui();
    this->setupLedsGui();
    this->setupVideoGui();
    this->setupProcessingGroup();
    this->setupShadersGui();
    this->setupCommunicationsGui();
    this->loadGuiValues();
    
    //this->drawGui();

    
    ofLogNotice() <<"GuiManager::initialized";
    
}

void GuiManager::setupGuiParameters()
{
    //create an actual ImGui context before setting up the addon
    ImGui::CreateContext();
    
    //specify a font to use
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(&ofToDataPath("fonts/roboto/Roboto-Regular.ttf")[0], 16.f);
    
    //ImGui::GetIO().FontGlobalScale = 1.0 / 2.0;
    
    //finally setup the addon ofxImGui::Gui setup;
    m_gui.setup(new GuiTheme());
    ofxImGui::Settings().windowPos  = ofVec2f(0,0);
    ofxImGui::Settings().windowSize = ofVec2f(GUI_WIDTH,ofGetHeight());
    
    m_width = 0;
    m_height = 0;
    
    auto appManager = &AppManager::getInstance();
    
}



void GuiManager::setupScenesGui()
{
     auto scenesManager = &AppManager::getInstance().getSceneManager();
    
    m_sceneNames.clear();
    m_scenesGroup.setName("Scenes");
    m_sceneMode.set("Scene", 0);
    m_sceneMode.addListener(scenesManager, &SceneManager::changeSceneIndex);
    m_scenesGroup.add(m_sceneMode);
    
    for(int i=0; i< scenesManager->getNumberScenes(); i++){
        m_sceneNames.push_back(scenesManager->getSceneName(i));
    }
    
    m_solidColor.set( "Color", ofFloatColor::white );
    m_parameters.add(m_solidColor);
    
    m_colorTemperature.set( "ColorTemperature (K)", 1500, 1500, 15000 );
    m_colorTemperature.addListener(this, &GuiManager::setColorTemperature);
    
    m_useHueCorrection.set("HueCorrection", true);
    m_useHueCorrection.addListener(scenesManager, &SceneManager::setUseHueCorrection);
    m_parameters.add(m_useHueCorrection);
    
    m_useWWAleds.set("WWA Leds", true);
    m_useWWAleds.addListener(this, &GuiManager::setUseWWAleds);
    m_parameters.add(m_useWWAleds);
 
    m_sceneTransitionTime.set("TransitionTime", 0.5, 0.0, 10);
    m_sceneTransitionTime.addListener(scenesManager, &SceneManager::onTransitionTimeChange);
    m_parameters.add(m_sceneTransitionTime);
}

void GuiManager::onSceneChange(int sceneIndex)
{
    int size = AppManager::getInstance().getSceneManager().getNumberScenes();
    ofLogNotice() <<"GuiManager::onSceneChange << size -> " << size;

    if(sceneIndex >=0 && sceneIndex < size){
        m_sceneMode = sceneIndex;
        ofLogNotice() <<"GuiManager::onSceneChange << m_sceneMode -> " << m_sceneMode;
    }
}


void GuiManager::onVideoChange(int videoIndex)
{
    int size = AppManager::getInstance().getSceneManager().getNumberScenes();
    int offset = AppManager::getInstance().getSceneManager().getSceneOffset();
    videoIndex+= offset;
    if(videoIndex >=offset && videoIndex < size){
        m_sceneMode = videoIndex;
    }
}

void GuiManager::setupModesGui()
{
    m_modeGroup.setName("View");
    m_viewMode.set("View Mode", 0);
    m_modeGroup.add(m_viewMode);
}


void GuiManager::setupVideoGui()
{
    auto videoManager = &AppManager::getInstance().getVideoManager();
    
    m_videoGroup.setName("Videos");
    m_videoPath.set("VideoPath", " ");
    m_videoPath.addListener(videoManager, &VideoManager::loadVideos);
    m_parameters.add(m_videoPath);
    m_videoGroup.add(m_videoPath);
    
}


void GuiManager::setupShadersGui()
{
    m_shadersGroup.setName("Shaders");
    
    m_shaderSpeed.set("Speed", 1.0, 0.0, 2.0);
    m_parameters.add(m_shaderSpeed);
    m_shadersGroup.add(m_shaderSpeed);
    
    m_shaderParameter.set("Parameter", 1.0, 0.0, 5.0);
    m_parameters.add(m_shaderParameter);
    m_shadersGroup.add(m_shaderParameter);
    
    m_colorAmount.set("ColorAmount", 0.0, 0.0, 1.0);
    m_parameters.add(m_colorAmount);
    m_shadersGroup.add(m_colorAmount);
    
    m_shaderDirection.set("Direction", 0, 0, 3);
    m_parameters.add(m_shaderDirection);
    m_shadersGroup.add(m_shaderDirection);
    
}


void GuiManager::setupProcessingGroup()
{
    auto scenesManager = &AppManager::getInstance().getSceneManager();
    
    m_postProcessingGroup.setName("Post Processing");
    
    m_contrast.set("Contrast", 1.0, 0.0, 2.0);
    m_contrast.addListener(scenesManager, &SceneManager::setContrast);
    m_parameters.add(m_contrast);
    m_postProcessingGroup.add(m_contrast);
    
    m_saturation.set("Saturation", 1.0, 0.0, 2.0);
    m_saturation.addListener(scenesManager, &SceneManager::setSaturation);
    m_parameters.add(m_saturation);
    m_postProcessingGroup.add(m_saturation);
    
    m_brightness.set("Brightness", 1.0, 0.0, 2.0);
    m_brightness.addListener(scenesManager, &SceneManager::setBrightness);
    m_parameters.add(m_brightness);
    m_postProcessingGroup.add(m_brightness);
    
    m_gamma.set("Gamma", 1.0, 0.0, 2.0);
    m_gamma.addListener(scenesManager, &SceneManager::setGamma);
    m_parameters.add(m_gamma);
    m_postProcessingGroup.add(m_gamma);
    
    m_minInput.set("MinInput", 0.0, 0.0, 1.0);
    m_minInput.addListener(scenesManager, &SceneManager::setMinInput);
    m_parameters.add(m_minInput);
    m_postProcessingGroup.add(m_minInput);
    
    m_maxInput.set("MaxInput", 1.0, 0.0, 1.0);
    m_maxInput.addListener(scenesManager, &SceneManager::setMaxInput);
    m_parameters.add(m_maxInput);
    m_postProcessingGroup.add(m_maxInput);
    
    m_minOutput.set("MinOutput", 0.0, 0.0, 1.0);
    m_minOutput.addListener(scenesManager, &SceneManager::setMinOutput);
    m_parameters.add(m_minOutput);
    m_postProcessingGroup.add(m_minOutput);
    
    m_maxOutput.set("MaxOutput", 1.0, 0.0, 1.0);
    m_maxOutput.addListener(scenesManager, &SceneManager::setMaxOutput);
    m_parameters.add(m_maxOutput);
    m_postProcessingGroup.add(m_maxOutput);
}


void GuiManager::setupLedsGui()
{
    auto ledsManager = &AppManager::getInstance().getLedsManager();
    auto sceneManager = &AppManager::getInstance().getSceneManager();
    
    m_ledsGroup.setName("Leds");
    m_ledsSize.set("Size", 1.0, 0.0, 20.0);
    m_ledsSize.addListener(ledsManager, &LedsManager::setSize);
    m_ledsGroup.add(m_ledsSize);
    m_parameters.add(m_ledsSize);
    
    m_ledsGrid.set("Grid", false);
    m_ledsGrid.addListener(ledsManager, &LedsManager::setGrid);
    m_ledsGroup.add(m_ledsGrid);
    m_parameters.add(m_ledsGrid);
}


void GuiManager::setupCommunicationsGui()
{
    auto udpManager = &AppManager::getInstance().getUdpManager();
    
    m_communicationsGroup.setName("Communications");
    
    m_maxDataPacketSize.set("MaxDataSize", 300, 100, 1500);
    m_maxDataPacketSize.addListener(udpManager, &UdpManager::setMaxDataPacketSize);
    m_communicationsGroup.add(m_maxDataPacketSize);
    m_parameters.add(m_maxDataPacketSize);
    
}


void GuiManager::update()
{
    //m_gui.update();
    m_gui.setTheme(new GuiTheme());

}


void GuiManager::draw()
{
    if(!m_showGui)
        return;
    
    this->drawRectangle();
    this->drawGui();
}


void GuiManager::drawGui()
{
    ofEnableAlphaBlending();
 
    m_gui.begin();
       
        auto mainSettings  = ofxImGui::Settings();
        //ofxImGui::Settings().windowPos  = ofVec2f(-LayoutManager::MARGIN,-LayoutManager::MARGIN);
       // ofxImGui::Settings().windowSize = ofVec2f(GUI_WIDTH,ofGetHeight());
        if (ofxImGui::BeginWindow("GUI", mainSettings, false))
        {
    
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
            
           
            if (ofxImGui::BeginTree(m_scenesGroup, mainSettings))
            {
                ofxImGui::AddParameter(m_sceneTransitionTime);
                ofxImGui::AddParameter(m_solidColor);
                ofxImGui::AddParameter(m_colorTemperature);
                ofxImGui::AddParameter(m_useHueCorrection);
                ofxImGui::AddParameter(m_useWWAleds);          
                ofxImGui::AddCombo(m_sceneMode, m_sceneNames);
                ofxImGui::EndTree(mainSettings);
            }
            
            if (ofxImGui::BeginTree(m_videoGroup, mainSettings))
            {
                ofxImGui::AddParameter(m_videoPath);
                
                if (ImGui::Button("Load Video Folder..."))
                {
                    auto dialogResult = ofSystemLoadDialog("Load Video Folder", true, ofToDataPath(""));
                    if (dialogResult.bSuccess)
                    {
                        m_videoPath = dialogResult.filePath;
                    }
                }
                
                
                ofxImGui::EndTree(mainSettings);
                
            }
            
            if (ofxImGui::BeginTree(m_communicationsGroup, mainSettings))
            {
                ofxImGui::AddParameter(m_maxDataPacketSize);
                ofxImGui::EndTree(mainSettings);
            }
            
            
            if (ofxImGui::BeginTree(m_ledsGroup, mainSettings))
            {
                ofxImGui::AddParameter(m_ledsGrid);
                ofxImGui::AddParameter(m_ledsSize);
                ofxImGui::EndTree(mainSettings);
            }

            
            if (ofxImGui::BeginTree(m_postProcessingGroup, mainSettings))
            {
                //auto & group =
                
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("Contrast"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("Saturation"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("Brightness"));
                //ofxImGui::AddParameter(m_postProcessingGroup.getFloat("Blur"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("Gamma"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("MinInput"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("MaxInput"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("MinOutput"));
                ofxImGui::AddParameter(m_postProcessingGroup.getFloat("MaxOutput"));
                ofxImGui::EndTree(mainSettings);
            }
            
            if (ofxImGui::BeginTree(m_shadersGroup, mainSettings))
            {
                //auto & group =
                ofxImGui::AddParameter(m_shadersGroup.getFloat("Speed"));
                ofxImGui::AddParameter(m_shadersGroup.getFloat("ColorAmount"));
                ofxImGui::AddParameter(m_shadersGroup.getFloat("Parameter"));
                ofxImGui::AddParameter(m_shadersGroup.getInt("Direction"));
                ofxImGui::EndTree(mainSettings);
            }
            
            
        }
    
        ofxImGui::EndWindow(mainSettings);
    m_gui.end();
    
    ofDisableAlphaBlending();
    
    this->updateSize(mainSettings);
    
   
}

void GuiManager::updateSize(const ofxImGui::Settings& settings)
{
    if(m_width!= settings.windowSize.x){
         m_width = settings.windowSize.x;
         AppManager::getInstance().getLayoutManager().windowResized(ofGetWidth(), ofGetHeight());
    }
   
    m_height = settings.windowSize.y;
    m_position = ofPoint(settings.windowPos.x, settings.windowPos.y);
}

void GuiManager::saveGuiValues(string path)
{
    ofLogNotice() <<"GuiManager::saveGuiValues-> saving values from: " << GUI_SETTINGS_FILE_NAME;
    
    ofXml xml;
    ofSerialize(xml, m_parameters);
    //xml.serialize(m_parameters);
    
    if(path.empty()){
        xml.save(GUI_SETTINGS_FILE_NAME);
    }
    else{
        xml.save(path);
    }
   
}

void GuiManager::loadGuiValues(string path)
{
    
    ofLogNotice() <<"GuiManager::loadGuiValues-> loading values from: " << GUI_SETTINGS_FILE_NAME;
    
    ofXml xml;
    if(path.empty()){
         xml.load(GUI_SETTINGS_FILE_NAME);
    }
    else{
         xml.load(path);
    }
    
    //xml.deserialize(m_parameters);
    ofDeserialize(xml, m_parameters);
}



void GuiManager::toggleGui()
{
    m_showGui = !m_showGui;
}

void GuiManager::drawRectangle()
{
    ofPushStyle();
    ofSetColor(15);
    ofDrawRectangle( 0, 0, this->getWidth(), ofGetHeight());
    ofPopStyle();
}
void GuiManager::setColorTemperature(int & value)
{
    if(m_useWWAleds){
        m_solidColor = this->colorTemperatureToWWA(m_colorTemperature);
    }
    else{
        m_solidColor = this->colorTemperatureToRGB(m_colorTemperature);
    }
}

void GuiManager::setUseWWAleds(bool & value)
{
    if(value){
        m_solidColor = this->colorTemperatureToWWA(m_colorTemperature);
    }
    else{
        m_solidColor = this->colorTemperatureToRGB(m_colorTemperature);
    }
    
}


ofColor GuiManager::colorTemperatureToWWA(float kelvin)
{
    float amber = 2000;
    float warm_white = 3000;
    float cold_white = 6000;
    
    float value = ofClamp(kelvin, amber, cold_white);
    ofColor color = ofColor(0);
    
    if(value <= warm_white){
        int amount = (int)ofMap(value, amber, warm_white, 0, 255);
        color.r = 255 - amount;
        color.b = amount;
    }
    else{
        int amount = (int)ofMap(value, warm_white, cold_white, 0, 255);
        color.b = 255 - amount;
        color.g = amount;
    }
    
    return color;
}

ofColor GuiManager::colorTemperatureToRGB(float kelvin)
{
    float temp = kelvin / 100;
    float red, green, blue;
    
    if( temp <= 66 ){
        
        red = 255;
        
        green = temp;
        green = 99.4708025861 * log(green) - 161.1195681661;
        
        
        if( temp <= 19){
            
            blue = 0;
            
        } else {
            
            blue = temp-10;
            blue = 138.5177312231 * log(blue) - 305.0447927307;
            
        }
        
    } else {
        
        red = temp - 60;
        red = 329.698727446 * pow(red, -0.1332047592);
        
        green = temp - 60;
        green = 288.1221695283 * pow(green, -0.0755148492 );
        
        blue = 255;
        
    }
    
    ofColor color;
    color.r = ofClamp(red, 0, 255);
    color.g = ofClamp(green, 0, 255);
    color.b = ofClamp(blue, 0, 255);
    
    return color;
}







