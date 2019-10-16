/*
 *  LedsManager.h
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */



#pragma once

#include "Manager.h"
#include "LedGroup.h"

//========================== class LedsManager ==============================
//============================================================================
/** \class LedsManager LedsManager.h
 *	\brief Class managing the LEDs
 *	\details It controls the postion and color of the LEDs
 */


class LedsManager: public Manager
{
    static const string LEDS_FOLDER_PATH;
    
    public:

        //! Constructor
        LedsManager();

        //! Destructor
        ~LedsManager();

        //! Setup the Halo Manager
        void setup();

        //! Update the Led Manager
        void update();
    
        //! Draw the Led Manager
        void draw();
    
        //! Draw the Led Manager Model
        void drawModel();
    
        //! Draw the Led Layout
        void drawLayout();
    
        void setPixels(ofPixelsRef pixels);
    
        bool isNewFrame() const {return m_isNewFrame;}
    
        void setSize(float& value);
    
        void setGrid(bool& value) {m_drawGrid = value;}
    
        void setLedBrightness(float& value){m_ledsBrightness = value;}
    
        bool get2dPosition(int index, ofPoint& position);
    
        const map<string, shared_ptr<LedGroup>> & getLedGroups() const {return m_ledGroups;}
    
    private:
    
        void setupLeds();

        void createLayout();
    
        bool readLeds();
    
        void normalizeLeds();
    
        void map2DpositionsToFbo();
    
        void setupShader();
    
        void updateLeds();
    
        void createLedPositions();
    
        void sortLeds();
    
        void clearAll();
    
        void drawGrid(float stepSize = 1.25f, size_t numberOfSteps = 8,bool x = true, bool y = true, bool z = true ) const;
    
        void drawGridPlane(float stepSize = 1.25f, size_t numberOfSteps = 8 ) const;
    
    
    private:
    
        
        ofRectangle        m_boundingBox;
        bool               m_isNewFrame;
        bool               m_is3D;
        bool               m_drawGrid;
        float              m_ledsBrightness;
    
        ofFbo   m_fbo;
        ofFbo   m_fboMaskee;
        ofFbo   m_fboMask;
        ofShader m_maskShader;
    
        map<string, shared_ptr<LedGroup>> m_ledGroups;
    
        ofPlanePrimitive m_plane;
    
};



