//
//  ofxThreadedGifFile.cpp
//  ofxGifDecoderAddon
//
//  Created by Jesus.Gollonet on 9/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "ofxThreadedGIFFile.h"



ofxThreadedGifFile::ofxThreadedGifFile(){
    w = h = 0;
}

ofxThreadedGifFile::~ofxThreadedGifFile(){}

void ofxThreadedGifFile::setup(int _w, int _h, vector<ofColor> _globalPalette, int _nPages){
    w               = _w;
    h               = _h;
    globalPalette   = _globalPalette;
    nPages          = _nPages;
    gpuQueue.clear();
}

// by now we're copying everything (no pointers)
void ofxThreadedGifFile::addFrame(ofPixels _px, int _left, int _top, GifFrameDisposal disposal, float _duration){
    if(getNumFrames() == 0){
        accumPx = _px; // we assume 1st frame is fully drawn
        Container c;
        c.px = _px;
        c.left = _left;
        c.top = _top;
        c.duration = _duration;
        gpuQueue.push_back(c);
        //f.setFromPixels(_px , _lef    t, _top, _duration); // [!]This function is dangerous
		gifDuration = _duration;
    } else {
        // add new pixels to accumPx
        int cropOriginX = _left;
        int cropOriginY = _top;
    
        // [todo] make this loop only travel through _px, not accumPx
        for (int i = 0; i < accumPx.getWidth() * accumPx.getHeight(); i++) {
            int x = i % accumPx.getWidth();
            int y = i / accumPx.getWidth();
            
            if (x >= _left  && x < _left + _px.getWidth()  &&
                y >= _top   && y < _top  + _px.getHeight()){
                int cropX = x - cropOriginX;  //   (i - _left) % _px.getWidth();
                int cropY = y - cropOriginY;
                //int cropI = cropX + cropY * _px.getWidth();
                if ( _px.getColor(cropX, cropY).a == 0 ){
                    switch ( disposal ) {
                        case GIF_DISPOSAL_BACKGROUND:
                            _px.setColor(x,y,bgColor);
                            break;
                            
                        case GIF_DISPOSAL_LEAVE:
                        case GIF_DISPOSAL_UNSPECIFIED:
                            _px.setColor(x,y,accumPx.getColor(cropX, cropY));
//                            accumPx.setColor(x,y,_px.getColor(cropX, cropY));
                            break;
                            
                        case GIF_DISPOSAL_PREVIOUS:
                            _px.setColor(x,y,accumPx.getColor(cropX, cropY));
                            break;
                    }
                } else {
                    accumPx.setColor(x, y, _px.getColor(cropX, cropY) );
                }
            } else {
                if ( _px.getColor(x, y) == bgColor ){
                    switch ( disposal ) {
                        case GIF_DISPOSAL_BACKGROUND:
                            accumPx.setColor(x,y,bgColor);
                            break;
                            
                        case GIF_DISPOSAL_UNSPECIFIED:
                        case GIF_DISPOSAL_LEAVE:
                            accumPx.setColor(x,y,_px.getColor(x, y));
                            break;
                            
                        case GIF_DISPOSAL_PREVIOUS:
                            _px.setColor(x,y,accumPx.getColor(x, y));
                            break;
                    }
                } else {
                    accumPx.setColor(x, y, _px.getColor(x, y) );
                }
            }
        }
        
        Container c;
        c.px = _px;
        c.left = _left;
        c.top = _top;
        c.duration = _duration;
        gpuQueue.push_back(c);
        //f.setFromPixels(_px,_left, _top, _duration);
    }
    accumPx = _px;
}

int ofxThreadedGifFile::pushTextureToGPU() {
    if (gpuQueue.size() > 0) {
        ofxGifFrame f;
        Container c = gpuQueue.front();
        f.setFromPixels(c.px, c.left, c.top, c.duration);
        gifFrames.push_back(f);
        gpuQueue.pop_front();
    }
    return gpuQueue.size();
}

int ofxThreadedGifFile::getWidth(){
    return w;
}

int ofxThreadedGifFile::getHeight(){
    return h;
}

float ofxThreadedGifFile::getDuration(){
    return gifDuration;
}

void ofxThreadedGifFile::draw(float _x, float _y){
    gifFrames[0].draw(_x, _y);
}

void ofxThreadedGifFile::drawFrame(int _frameNum, float _x, float _y){
    if(_frameNum < 0 || _frameNum >= gifFrames.size()){
        ofLog(OF_LOG_WARNING, "ofxThreadedGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    gifFrames[_frameNum].draw(_x , _y , gifFrames[_frameNum].getWidth(), gifFrames[_frameNum].getHeight());
}

void ofxThreadedGifFile::drawFrame(int _frameNum, float _x, float _y, int _w, int _h){
    if(_frameNum < 0 || _frameNum >= gifFrames.size()){
        ofLog(OF_LOG_WARNING, "ofxThreadedGifFile::drawFrame frame out of bounds. not drawing");
        return;
    }
    gifFrames[_frameNum].draw(_x , _y , _w, _h);
}


void ofxThreadedGifFile::setBackgroundColor(ofColor _c) {
    bgColor = _c;
}

ofColor ofxThreadedGifFile::getBackgroundColor() {
    return bgColor;
}


int ofxThreadedGifFile::getNumFrames() {
    return gifFrames.size();
}

ofxGifFrame * ofxThreadedGifFile::getFrameAt(int _index) {
    return &(gifFrames[_index]); //??
}

ofTexture & ofxThreadedGifFile::getTextureRefAt(int _index) {
    return gifFrames[_index].getTextureReference();
}

vector <ofColor> ofxThreadedGifFile::getPalette(){
    return globalPalette;
}

void ofxThreadedGifFile::clear() {
    gifFrames.clear();
    globalPalette.clear();
}
