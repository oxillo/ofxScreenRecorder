#pragma once

#include "ofMain.h"
#include "ofAppGLFWWindow.h"

#include "ofxScreenRecorder.h"


class ofApp : public ofBaseApp{

	public:
        void setup();
		void update();
        void draw();
        void exit();

    private: 
        int i;
        
        ScreenRecorder recorder;

        uint64_t movieStartTimeMicros;

        int nbFramesPerMovie; //Specify the number a frame in a movie. It will automatically restart 

        
        ofFbo fbo;  // frame buffer for screen content
};

