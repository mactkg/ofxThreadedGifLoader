//
//  ofxThreadedGifDecoder.cpp
//  ofxThreadedGifLoader
//
//  almost code from jesusgollonet/ofxGifDecoder
//  MIT
//

#include "ofxThreadedGifDecoder.h"
//#include "FreeImage.h"
#define DWORD uint32_t

ofxThreadedGifDecoder::ofxThreadedGifDecoder(){
    globalPalette = NULL;
    globalPaletteSize = 0;
}

// return a bool if succesful?
bool ofxThreadedGifDecoder::decode(string fileName) {
    reset();
	int width, height, bpp;
	fileName                    = ofToDataPath(fileName);
	bool bDecoded               = false;
	FIMULTIBITMAP	* multiBmp  = NULL;
    
    
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(fileName.c_str(), 0);
    
	if(fif != FIF_GIF) {
        ofLog(OF_LOG_WARNING, "ofxThreadedGifDecoder::decode. this is not a gif file. not processing");
        
        return bDecoded;
	}
    
    multiBmp = FreeImage_OpenMultiBitmap(fif, fileName.c_str(), false, false,true, GIF_LOAD256);
    
    if (multiBmp){

        // num frames
        int nPages = FreeImage_GetPageCount(multiBmp);
        
        // here we process the first frame
        for (int i = 0; i < nPages; i++) {
            FIBITMAP * dib = FreeImage_LockPage(multiBmp, i);            
            if(dib) {
                if (i == 0) {
                    createGifFile(dib, nPages);
                    bDecoded = true;   // we have at least 1 frame
                }
                processFrame(dib, i);
                FreeImage_UnlockPage(multiBmp, dib, false);
            } else {
                ofLog(OF_LOG_WARNING, "ofxThreadedGifDecoder::decode. problem locking page");
                
            }
        }
        FreeImage_CloseMultiBitmap(multiBmp, 0);
    }else {
        ofLog(OF_LOG_WARNING, "ofxThreadedGifDecoder::decode. there was an error processing.");
	}
    return bDecoded;
}

void ofxThreadedGifDecoder::createGifFile(FIBITMAP * bmp, int _nPages){

    FITAG *tag;

    int logicalWidth, logicalHeight;
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalWidth", &tag)) {
        logicalWidth = *(unsigned short *)FreeImage_GetTagValue(tag);
    }
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "LogicalHeight", &tag)) {
        logicalHeight = *(unsigned short *)FreeImage_GetTagValue(tag);
    }
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "GlobalPalette", &tag) ) {
        globalPaletteSize = FreeImage_GetTagCount(tag);
        if( globalPaletteSize >= 2 ) {
            globalPalette = (RGBQUAD *)FreeImage_GetTagValue(tag);
            for (int i = 0 ; i < globalPaletteSize; i++) {
                ofColor c;
                c.set(globalPalette[i].rgbRed, globalPalette[i].rgbGreen, globalPalette[i].rgbBlue);
                palette.push_back(c);
            }
        }
    }
    
    gifFile.setup(logicalWidth, logicalHeight, palette, _nPages);    

    RGBQUAD bgColor;
    if(FreeImage_GetBackgroundColor(bmp, &bgColor)){
       gifFile.setBackgroundColor(ofColor(bgColor.rgbRed, bgColor.rgbGreen, bgColor.rgbBlue));
    }
}

void ofxThreadedGifDecoder::processFrame(FIBITMAP * bmp, int _frameNum){
    FITAG *tag;
    ofPixels pix;

    unsigned int   frameLeft, frameTop;
    float frameDuration;
    GifFrameDisposal disposal_method = GIF_DISPOSAL_BACKGROUND;
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameLeft", &tag)) {
        frameLeft = *(unsigned short *)FreeImage_GetTagValue(tag);
    }
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTop", &tag)) {
        frameTop = *(unsigned short *)FreeImage_GetTagValue(tag);
    }
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "FrameTime", &tag)) {
        long frameTime = *(long *)FreeImage_GetTagValue(tag);// centiseconds 1/100 sec
        frameDuration =(float)frameTime/1000.f;
    }
    
    if( FreeImage_GetMetadata(FIMD_ANIMATION, bmp, "DisposalMethod", &tag)) {
        disposal_method = (GifFrameDisposal) *(unsigned char *)FreeImage_GetTagValue(tag);
    }
    
    // we do this for drawing. eventually we should be able to draw 8 bits? at least to retain the data
//    if(FreeImage_GetBPP(bmp) == 8) {
//        // maybe we should only do this when asked for rendering?
//        bmp = FreeImage_ConvertTo24Bits(bmp);
//    }
    
    FIBITMAP* bmpConverted = NULL;
	if(FreeImage_GetColorType(bmp) == FIC_PALETTE || FreeImage_GetBPP(bmp) < 8) {
		if(FreeImage_IsTransparent(bmp)) {
			bmpConverted = FreeImage_ConvertTo32Bits(bmp);
		} else {
			bmpConverted = FreeImage_ConvertTo24Bits(bmp);
		}
		bmp = bmpConverted;
	}
    
	unsigned int width      = FreeImage_GetWidth(bmp);
	unsigned int height     = FreeImage_GetHeight(bmp);
	unsigned int bpp        = FreeImage_GetBPP(bmp);
    // changed this bc we're not using PixelType template anywhere else...
	unsigned int channels   = (bpp / sizeof( unsigned char )) / 8;
	unsigned int pitch      = FreeImage_GetPitch(bmp);
    
	// ofPixels are top left, FIBITMAP is bottom left
	FreeImage_FlipVertical(bmp);

	unsigned char * bmpBits = FreeImage_GetBits(bmp);
    
	if(bmpBits != NULL) {
        
		pix.setFromAlignedPixels(bmpBits, width, height, channels, pitch);

        #ifdef TARGET_LITTLE_ENDIAN
            pix.swapRgb();
        #endif
        
        gifFile.addFrame(pix, frameLeft, frameTop, disposal_method, frameDuration);
	} else {
		ofLogError() << "ofImage::putBmpIntoPixels() unable to set ofPixels from FIBITMAP";
	}
    
    FreeImage_Unload(bmp);
    pix.clear();
}

void ofxThreadedGifDecoder::reset(){
    gifFile.clear();
}

void ofxThreadedGifDecoder::clear(){
    gifFile.clear();
    for(int i=0; i<pxs.size(); i++){
        pxs[i]->clear();
        delete pxs[i];
    }
    pxs.clear();
    palette.clear();
    delete globalPalette;
}


// should I return a pointer of a copy?
ofxThreadedGifFile ofxThreadedGifDecoder::getFile() {
    return gifFile;
}
