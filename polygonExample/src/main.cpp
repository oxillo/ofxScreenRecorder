#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    //av_log_set_level(AV_LOG_DEBUG);
    //av_register_all();
    //avformat_network_init();

    ofSetupOpenGL(1920,1080, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp());

    //avformat_network_deinit();
}
