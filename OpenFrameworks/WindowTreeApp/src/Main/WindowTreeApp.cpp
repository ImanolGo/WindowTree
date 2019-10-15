/*
 *  WindowTreeApp.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */



#include "AppManager.h"

#include "WindowTreeApp.h"

//--------------------------------------------------------------
void WindowTreeApp::setup(){
    AppManager::getInstance().setup();
}

//--------------------------------------------------------------
void WindowTreeApp::update(){
    AppManager::getInstance().update();
}

//--------------------------------------------------------------
void WindowTreeApp::draw(){
    AppManager::getInstance().draw();
}

void WindowTreeApp::exit()
{
    ofLogNotice() <<"WindowTreeApp::exit";

}

//--------------------------------------------------------------
void WindowTreeApp::keyPressed(int key){

}

//--------------------------------------------------------------
void WindowTreeApp::keyReleased(int key){

}

//--------------------------------------------------------------
void WindowTreeApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void WindowTreeApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void WindowTreeApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void WindowTreeApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void WindowTreeApp::windowResized(int w, int h){
    AppManager::getInstance().getLayoutManager().windowResized(w,h);


}

//--------------------------------------------------------------
void WindowTreeApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void WindowTreeApp::dragEvent(ofDragInfo dragInfo){

}
