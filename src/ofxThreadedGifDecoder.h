//
//  ofxThreadedGifDecoder.h
//  ofxThreadedGifLoader
//
//  almost code from jesusgollonet/ofxGifDecoder
//  MIT
//
#ifndef OFX_THREADED_GIF_DECORDER_H
#define OFX_THREADED_GIF_DECORDER_H

//#pragma once
#include "ofMain.h"
#include "ofxThreadedGifFile.h"

#include "FreeImage.h"

// give it an animated gif and get a frame
class ofxThreadedGifDecoder {
public:
    ofxThreadedGifDecoder();
    bool decode(string fileName);
    ofxThreadedGifFile getFile();
    
    void clear();
    
    vector<ofPixels *> pxs;    
    vector <ofColor> palette;
    
private :
    
    ofxThreadedGifFile  gifFile;
    
    void reset();
    void createGifFile(FIBITMAP * bmp, int _nPages);
    void processFrame(FIBITMAP * bmp, int _frameNum);
    
    int globalPaletteSize;
    RGBQUAD * globalPalette;
    

};

#endif