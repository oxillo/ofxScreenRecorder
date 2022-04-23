#include "ofxScreenRecorder.h"

#define DEBUG ofLogError() <<__FILE__ << ": Line " << __LINE__;

#define OFX_ADDON "ofxScreenRecorder"




const enum AVCodecID codec_id = AV_CODEC_ID_H264;
const int titleHeight = 72;
const int legendNbLines = 16;
const ofColor recorderBackgroundColor = ofColor(0, 26, 92);
const ofColor recorderForegroundColor = ofColor::white;

//===============================================================================
/* ScreenRecorder constructor */
ScreenRecorder::ScreenRecorder(){
    /* Initialize libavcodec, and register all codecs and formats. */
    //av_register_all();

    titleFont.load("arial.ttf",32);
    //titleFont.setLineHeight(28.0f);
    titleFont.setLetterSpacing(1.037);
    //titleHeight = ( titleFont.getAscenderHeight() - titleFont.getDescenderHeight() ) * 1.5;
    //titleAnchor = titleFont.getAscenderHeight() * 1.5;
    legendFont.load("mono.ttf",18);
    legendFont.setLineHeight(22.0f);
    legendFont.setLetterSpacing(1.037);
    //legendHeight = legendFont.getLineHeight();
    //legendHeight = ( legendFont.getAscenderHeight() - legendFont.getDescenderHeight() ) * 1.2;
    //legendAnchor = legendFont.getAscenderHeight() * 1.2;
    setupCompleted = false;
}

//===============================================================================
bool ScreenRecorder::setup( int width, int height ){

    /* Stop movie recording if active */

    stopRecordingMovie();

    /* Keep track of the expected size of the rendered frame */
    frameWidth = width;
    frameHeight = height;

    int legendHeight = int( legendNbLines * legendFont.getAscenderHeight() );

    /* Allocate the frame buffer that will be used to hold the recorded frame content
       It is the user content + the title at the top + the legend at the bottom 
       For the recording to work, we need frames that have a width that are a multiple of 16*/
    width = width + ( (16-width%16)%16 ); // Add the necessary pixels to have a 16 multiple
    compositingFbo.allocate( width, titleHeight + height + legendHeight , GL_RGB);
    compositingFboLeft = 0.5*( width - frameWidth );

    setupCompleted = true;
    return true;
}

//===============================================================================
void ScreenRecorder::addLogo(ofImage newLogo){
    float aspectRatio = newLogo.getWidth() / newLogo.getHeight();
    logo = newLogo;
    logo.resize( aspectRatio*titleHeight, titleHeight );

    logoLeft = compositingFbo.getWidth() - aspectRatio*titleHeight;
}


//===============================================================================
/* Add a video output stream. */
/*bool ScreenRecorder::add_video_stream(){
    
    st.st = avformat_new_stream(oc, NULL);
    if (!st.st) {
        ofLogError(OFX_ADDON) << "add_video_stream : could not allocate stream";
        return false;
    }

    st.st->time_base = enc->time_base;
    
    return true;
}*/

//===============================================================================

//===============================================================================
void ScreenRecorder::open_video(std::string filename){

    if( !fmt.fromFilename(filename) ) fmt.fromShortName("mp4");
    
    /* Setup the encoder */
    avpp::VideoEncoderSettings settings;
    settings.codec_id = AV_CODEC_ID_H264;
    settings.width = compositingFbo.getWidth();
    settings.height = compositingFbo.getHeight();
    settings.fps = 30;

    fmt.addVideoStream( settings );
    
    if( !fmt.startRecording() ){
        ofLogError() << "Recording can not be started";
    }
}

//===============================================================================
void ScreenRecorder::snapshot(std::string filename){
    static int snapshotNumber = 0;
    /* Build an automatic file name if needed */
    if( filename=="" ){
        std::stringstream autoFilename;
        autoFilename << "snapshot_" << setfill('0') << setw(4) << snapshotNumber << ".png";
        snapshotNumber++;
        filename = autoFilename.str();
    }
    ofSaveImage( pix, filename );
}

//===============================================================================
void ScreenRecorder::startRecordingMovie( std::string filename ){
    static int movieNumber = 0;
    /* Stop active recording if any */
    stopRecordingMovie();

    /* Build an automatic file name if needed */
    if( filename=="" ){
        std::stringstream autoFilename;
        autoFilename << "movie_" << setfill('0') << setw(4) << movieNumber << ".mp4";
        movieNumber++;
        filename = autoFilename.str();
    }
    open_video(filename);
    movieStartTimeMicros = ofGetSystemTimeMicros();
}

//===============================================================================
void ScreenRecorder::stopRecordingMovie(){
    fmt.stopRecording();
}

//===============================================================================
void ScreenRecorder::update_video_frame(){
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
void ScreenRecorder::draw( const ofFbo &fbo ){
    compositingFbo.begin();
        ofClear(recorderBackgroundColor);
        ofSetColor(recorderForegroundColor);
        titleFont.drawString(recorderTitle, 20,titleFont.getAscenderHeight()*1.2);
        legendFont.drawString(recorderLegend,20,titleHeight+frameHeight+legendFont.getAscenderHeight()*1.2 );
        ofSetColor(ofColor::white);
        if( logo.isAllocated() )
            logo.draw( logoLeft, 0, logo.getWidth(), logo.getHeight() );
        fbo.draw( compositingFboLeft, titleHeight, frameWidth, frameHeight );
    compositingFbo.end();
    compositingFbo.readToPixels( pix );
    
    if( fmt.isRecordingActive() ){
        fmt[0].encode(pix);
    }
}

ScreenRecorder::~ScreenRecorder() {
    stopRecordingMovie();
}













