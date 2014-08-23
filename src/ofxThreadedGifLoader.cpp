#include "ofxThreadedGIFLoader.h"
#include <sstream>

ofEvent<ofxGifLoadedEvent> ofxGifLoadedEvent::events;

ofxThreadedGIFLoader::ofxThreadedGIFLoader() 
:ofThread()
{
	nextID = 0;
    ofAddListener(ofEvents().update, this, &ofxThreadedGIFLoader::update);
	//ofAddListener(ofURLResponseEvent(),this,&ofxThreadedGIFLoader::urlResponse);
    lastUpdate = 0;
}

ofxThreadedGIFLoader::~ofxThreadedGIFLoader(){
	condition.signal();
    ofRemoveListener(ofEvents().update, this, &ofxThreadedGIFLoader::update);
	//ofRemoveListener(ofURLResponseEvent(),this,&ofxThreadedGIFLoader::urlResponse);
}

// Load an GIF from disk.
//--------------------------------------------------------------
void ofxThreadedGIFLoader::loadFromDisk(string filename) {
    for(auto it = working_filenames.begin(); it != working_filenames.end(); ++it) {
        if (*it == filename)
            return;
    }
    
    ofLogNotice("ofxThreadedGIFLoader") << "Start loading:" << filename;
    
	nextID++;
	ofxGIFLoaderEntry entry(OF_LOAD_FROM_DISK);
	entry.filename = filename;
	entry.id = nextID;
	entry.name = filename;
    working_filenames.push_back(filename);
    
    lock();
    images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();

    if (!isThreadRunning()) {
        startThread();
    }
}


// Load an url asynchronously from an url.
//--------------------------------------------------------------
/*void ofxThreadedGIFLoader::loadFromURL(ofxGifFile& gif, string url) {
	nextID++;
	ofxGIFLoaderEntry entry(gif, OF_LOAD_FROM_URL);
	entry.url = url;
	entry.id = nextID;
	//entry.image->setUseTexture(false);
	entry.name = "image" + ofToString(entry.id);

    lock();
	images_to_load_buffer.push_back(entry);
    condition.signal();
    unlock();
}*/


// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxThreadedGIFLoader::threadedFunction() {
    //ofxOpenGLContextScope scope;
    deque<ofxGIFLoaderEntry> images_to_load;

	while( !images_to_load_buffer.empty() ) {
		lock();
		if(images_to_load_buffer.size() ){
            images_to_load.insert( images_to_load.end(),
                                images_to_load_buffer.begin(),
                                images_to_load_buffer.end() );

            images_to_load_buffer.clear();
        }
		unlock();
        
        
        while( !images_to_load.empty() ) {
            ofxGIFLoaderEntry  & entry = images_to_load.front();
            
            if(entry.type == OF_LOAD_FROM_DISK) {
                ofxThreadedGifFile gif;
                if( decoder.decode(entry.filename) )  {
                    gif = decoder.getFile();
                    entry.gif = gif;
                    images_to_update.push_back(entry);
                } else {
                    ofLogError("ofxThreadedGIFLoader") << "couldn't load file: \"" << entry.filename << "\"";
                }
            }else if(entry.type == OF_LOAD_FROM_URL) {
                lock();
                images_async_loading.push_back(entry);
                unlock();
                
                //ofLoadURLAsync(entry.url, entry.name);
            }

    		images_to_load.pop_front();
        }
	}
}


// When we receive an url response this method is called; 
// The loaded image is removed from the async_queue and added to the
// update queue. The update queue is used to update the texture.
//--------------------------------------------------------------
/*void ofxThreadedGIFLoader::urlResponse(ofHttpResponse & response) {
	if(response.status == 200) {
		lock();
		
		// Get the loaded url from the async queue and move it into the update queue.
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
            if(! decoder.decode(entry.filename) )  {
                (*it).image = &decoder.getFile();
                ofLogError("ofxThreadedGIFLoader") << "couldn't load file: \"" << entry.filename << "\"";
            }
            
			images_to_update.push_back(*it);
			images_async_loading.erase(it);
		}
		
		unlock();
	}else{
		// log error.
		ofLogError("ofxThreadedGIFLoader") << "couldn't load url, response status: " << response.status;
		ofRemoveURLRequest(response.request.getID());
		// remove the entry from the queue
		lock();
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_async_loading.end()) {
			images_async_loading.erase(it);
		}
		unlock();
	}
}*/


// Check the update queue and update the texture
//--------------------------------------------------------------
void ofxThreadedGIFLoader::update(ofEventArgs & a){
    
    // Load 1 image per update so we don't block the gl thread for too long
    lock();
	if (!images_to_update.empty()) {

		ofxGIFLoaderEntry &entry = images_to_update.front();
        
        int num = entry.gif.pushTextureToGPU();
		if(num <= 0) {
            static ofxGifLoadedEvent newEvent;
            newEvent.gif = entry.gif;
            newEvent.path = entry.filename;
            ofNotifyEvent(ofxGifLoadedEvent::events, newEvent);
            
            auto f = find(working_filenames.begin(), working_filenames.end(), entry.filename);
            working_filenames.erase(f);
            images_to_update.pop_front();
        }
    }
    unlock();
}


// Find an entry in the aysnc queue.
//   * private, no lock protection, is private function
//--------------------------------------------------------------
ofxThreadedGIFLoader::entry_iterator ofxThreadedGIFLoader::getEntryFromAsyncQueue(string name) {
	entry_iterator it = images_async_loading.begin();
	for(;it != images_async_loading.end();it++) {
		if((*it).name == name) {
			return it;
		}
	}
	return images_async_loading.end();
}
