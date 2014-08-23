#pragma once

#include "ofMain.h"
#include "ofxThreadedGifLoader.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void draw();
		void exit();
		
		void keyPressed(int key);
        void gifLoaded(ofxGifLoadedEvent &e);
		
		ofxThreadedGifLoader loader;
        vector<ofxThreadedGifFile> gifs;
        int playCnt = 0;
};
