/*
 *  LedGroup.h
 *  Window Tree App
 *
 *  Created by Imanol Gomez on 15/10/19.
 *
 */


#pragma once

#include "ofMain.h"

class LedGroup{
    
    public:
    
        LedGroup(string& name);
    
        virtual ~LedGroup();
    
        bool setup();
    
        void draw();
    
        void drawModel();
    
        void setSize(float value) ;
    
        void setSizeScale(float value) {m_sizeScale = value;}
    
        void clear();
    
        const string& getName() const {return m_name;}
    
        const ofRectangle & getBoundingBox() const {return m_boundingBox;}
    
        void setBoundingBox(const ofRectangle & box) {m_boundingBox = box;}
    
        const vector <glm::vec3>& get2dPositions() const {return m_points2D;}
    
        void setPixels(ofPixelsRef pixels);
    
        const vector <ofFloatColor>& getColors() const {return m_colors;}
    
    protected:
    
        bool readLeds();
    
        void setupModel();
    
        void setupShader();
    
        void arrangeLeds();
    
        bool loadLedPair(string& pathTwoD, string& pathThreeD);
    
        bool addLedPair(string& pathTwoD, string& pathThreeD);
    
        bool isValidLedFile(const string& path);
    
        bool parseLedLine(string& line, glm::vec3& position);
    
        int createLedPair(const glm::vec3& position2D,const glm::vec3& position3D);
    
        void normalizeLeds();
    
        void normalize2DLeds();
    
        void normalize3DLeds();
    
        void  centreLeds();
        
        void  centre3DLeds();
        
        void  centre2DLeds(float margin_percentage);
    
        virtual void setPixelColor(ofPixelsRef pixels, int index);
    
    protected:
            
        float m_sizeScale;
    
        ofRectangle        m_boundingBox;
        float              m_offset;
    
        ofVbo   m_vbo;
        ofVbo   m_vboModel;
        ofShader m_vboShader;
        ofShader m_maskShader;
        ofTexture m_texture;
        ofTexture m_textureModel;
    
        vector <glm::vec3> m_points3D;
        vector <glm::vec3> m_points2D;
        vector <glm::vec3> m_sizes;
        vector <glm::vec3> m_sizesModel;
        vector <ofFloatColor> m_colors;
        vector <ofFloatColor> m_colorsBlack;
    
        string  m_name;

};


class GoetheGroup: public LedGroup
{
    public:
    
        GoetheGroup(string& name);
    
        virtual ~GoetheGroup();
    
    protected:
    
        virtual void setPixelColor(ofPixelsRef pixels, int index);
    
    
};







