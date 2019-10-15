/*
 *  BlankScene.cpp
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */


#include "BlankScene.h"


BlankScene::BlankScene(): ofxScene("Blank"){}

void BlankScene::setup() {
    ofLogNotice("BlankScene::setup");
}

void BlankScene::update() {
    
}

void BlankScene::draw() {
    ofBackground(0);
    //ofBackgroundGradient( ofColor(255,0,0), ofColor(0,255,0), OF_GRADIENT_CIRCULAR );
}

void BlankScene::willFadeIn() {
     ofLogNotice("BlankScene::willFadeIn");
}

void BlankScene::willDraw() {
    ofLogNotice("BlankScene::willDraw");
}

void BlankScene::willFadeOut() {
    ofLogNotice("BlankScene::willFadeOut");
}

void BlankScene::willExit() {
    ofLogNotice("BlankScene::willExit");
}
