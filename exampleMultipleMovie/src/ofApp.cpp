#include "ofLog.h"
#include "ofApp.h"


const int fps = 30;

const uint64_t movieMaxTimeMicros = 20 * 60 * 1000000; //2 minutes


//--------------------------------------------------------------
void ofApp::setup(){
    fbo.allocate( ofGetWidth(), ofGetHeight(), GL_RGB);

    nbFramesPerMovie = 2 * 60 * fps; // 2 min

    recorder.setup( 693, 500 );
    recorder.addTitle("ofxSnapshotRecorder");
    recorder.addLogo( ofImage("of.png") );

    ofSetFrameRate(fps);

    ofSetVerticalSync(false);
    //ofLogToFile("record.log");
}

//--------------------------------------------------------------
void ofApp::update(){
    /* select the stream to encode */
    
    fbo.begin();
        ofSetColor(255*ofRandom(0.,1.), 255*ofRandom(0.,1.), 255*ofRandom(0.,1.), 255*ofRandom(0.,1.));
        ofDrawRectangle(ofRandomWidth(), ofRandomHeight(), 80, 80);
    fbo.end();


    uint64_t movieElapsedTimeMicros = ofGetSystemTimeMicros() - movieStartTimeMicros;
    // Restart the movie when the number of frames is reached
    if( movieElapsedTimeMicros >= movieMaxTimeMicros ){
        std::stringstream filename;
        std::string timestamp(ofGetTimestampString());
        filename <<  "movie_"  << timestamp << ".mp4";
        ofLogNotice()<< "before startRecordingMovie " << ofGetSystemTimeMicros();
        recorder.startRecordingMovie( filename.str() );
        ofLogNotice()<< "after startRecordingMovie " << ofGetSystemTimeMicros();
        movieStartTimeMicros = ofGetSystemTimeMicros();
        ofLogNotice() << "starting at " << movieStartTimeMicros / 1000000; 
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    fbo.draw(0 , 0, ofGetWidth(), ofGetHeight() );
    recorder.addLegend("It is "+ofGetTimestampString() + "\n\n" + "Recorded with ofxSnapshotRecorder");
    recorder.draw( fbo );
    // Take a snapshot every 2 seconds
    /*if( ofGetFrameNum()%(2*30)==0 ){
        recorder.snapshot("");
    }*/
}

//--------------------------------------------------------------
void ofApp::exit(){
    recorder.stopRecordingMovie();
}



