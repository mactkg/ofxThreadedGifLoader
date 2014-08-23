#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetVerticalSync(true);
    
    ofAddListener(ofxGifLoadedEvent::events, this, &ofApp::gifLoaded);
    loader.startThread();
    ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::draw(){
	// draw the images.
	ofSetColor(255);
	for(int i = 0; i < (int)gifs.size(); ++i) {
		gifs[i].drawFrame(playCnt%gifs[i].getNumFrames(), 0, 240*i+40, 360, 240);
	}	
	
	// draw the FPS
	ofRect(0,ofGetHeight()-20,30,20);

	ofSetColor(0);
	ofDrawBitmapString(ofToString(ofGetFrameRate(),0),5,ofGetHeight()-5);
	ofDrawBitmapString(ofToString("Press space key to load",0), 5, 20);
    
    playCnt++;
}

//--------------------------------------------------------------
void ofApp::exit(){
	loader.stopThread();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        int total = 2;
        for(int i = 0; i < total; ++i) {
            loader.loadFromDisk("view" + ofToString(i) + ".gif");
        }
    } else if (key == 'd') {
        gifs.pop_back();
    }
}

//--------------------------------------------------------------
void ofApp::gifLoaded(ofxGifLoadedEvent &e){
    gifs.push_back(e.gif);
}
