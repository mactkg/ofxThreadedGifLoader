//
//  ofxThreadedGifFile.h
//  from ofxGifGile.h of ofxGifDecoder
//
//  Created by Jesus.Gollonet on 9/5/11.
//  forked by Kenta Hara on 28/5/14
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef OFX_THREADED_GIF_FILE_H
#define OFX_THREADED_GIF_FILE_H

//#pragma once

#include "ofMain.h"
#include "ofxGifFrame.h"

enum GifFrameDisposal {
    GIF_DISPOSAL_UNSPECIFIED,
    GIF_DISPOSAL_LEAVE,
    GIF_DISPOSAL_BACKGROUND,
    GIF_DISPOSAL_PREVIOUS
};

// todo. add loop to setup

class ofxThreadedGifFile {
    public:
    
        ofxThreadedGifFile();
        ~ofxThreadedGifFile();
        void setup(int _w, int _h, vector<ofColor> _globalPalette, int _nPages);
        void setBackgroundColor(ofColor _c);
        ofColor getBackgroundColor();
        void addFrame(ofPixels _px, int _left , int _top, GifFrameDisposal disposal = GIF_DISPOSAL_PREVIOUS, float _duration = 0);
        vector <ofColor> getPalette();
        // void numFrames, void isAnimated, void duration
        int getNumFrames();
        int getWidth();
        int getHeight();
        float getDuration();
        int pushTextureToGPU();
    
        ofxGifFrame * getFrameAt(int _index);
        ofTexture & getTextureRefAt(int _index);
    // array operator overload?
    // gif[1] is frame 1, and we can treat is as such
    // gif[1].getTop(); gif[1].draw() ...    

        // void update
        void draw(float _x, float _y);  
        // this should draw with the offsets correctly applied. 
        void drawFrame(int _frameNum, float _x, float _y); 
        void drawFrame(int _frameNum, float _x, float _y, int _w, int _h); 
        void clear();
    
    typedef struct container {
        ofPixels px;
        int left;
        int top;
        float duration;
    } Container;
    
    
    private:
        ofColor bgColor;
        vector <ofxGifFrame> gifFrames;
        deque <Container> gpuQueue;
        vector <ofColor> globalPalette;
        //vector <ofPixels *> rawPixels;
        int w, h, nPages;
        bool bAnimated;    
        bool bLoop;
        float duration;
        ofPixels accumPx;
    	float gifDuration;
};

#endif