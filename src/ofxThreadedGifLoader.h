//  ofxThreadedGifLoader.h
//  ofxThreadedGifLoader
//
//  almost code from jesusgollonet/ofxGifDecoder
//  MIT
//


#ifndef OFX_THREADED_GIF_LOADER_H
#define OFX_THREADED_GIF_LOADER_H

//#pragma once

#include <deque>
#include "ofThread.h"
#include "ofImage.h"
#include "ofURLFileLoader.h"
#include "ofTypes.h"

#include "ofxThreadedGifDecoder.h"
#include "ofxThreadedGifFile.h"
#include "ofxGifFrame.h"

// must use poco condition not lock for this
#include "Poco/Condition.h"

using namespace std;

class ofxThreadedGifLoader : public ofThread {
public:
    ofxThreadedGifLoader();
    ~ofxThreadedGifLoader();

	void loadFromDisk(string file);
	//void loadFromURL(ofxGifFile& gif, string url);



private:
	void update(ofEventArgs & a);
    void threadedFunction();
	//void urlResponse(ofHttpResponse & response);

    // Where to load form?
    enum ofLoaderType {
        OF_LOAD_FROM_DISK
        ,OF_LOAD_FROM_URL
    };
    
    
    // Entry to load.
    struct ofxGifLoaderEntry {
        ofxGifLoaderEntry() {
            //gif = NULL;
            type = OF_LOAD_FROM_DISK;
            id=0;
        }
        
        ofxGifLoaderEntry(ofLoaderType nType) {
            type = nType;
            id=0;
        }
        ofxThreadedGifFile gif;
        ofLoaderType type;
        string filename;
        string url;
        string name;
        int id;
    };
    
    ofxThreadedGifDecoder decoder;

    typedef deque<ofxGifLoaderEntry>::iterator entry_iterator;
	entry_iterator      getEntryFromAsyncQueue(string name);

	int                 nextID;

    Poco::Condition     condition;

    int                 lastUpdate;

	deque<ofxGifLoaderEntry> images_async_loading; // keeps track of images which are loading async
	deque<ofxGifLoaderEntry> images_to_load_buffer;
    deque<ofxGifLoaderEntry> images_to_update;
    vector<string> working_filenames;
};

class ofxGifLoadedEvent : public ofEventArgs {
public:
    ofxThreadedGifFile gif;
    string path;
    
    ofxGifLoadedEvent() {
        
    }
    
    static ofEvent<ofxGifLoadedEvent> events;
};

#endif