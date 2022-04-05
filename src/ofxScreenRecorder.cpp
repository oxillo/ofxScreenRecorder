#include "ofxScreenRecorder.h"

#define DEBUG ofLogNotice() <<__FILE__ << ": Line " << __LINE__;

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
    int ret;

    

    /* Allocate the output context for the muxer */
    //oc = avformat_alloc_context();
    //fmt = avpp::Container();
    if( !fmt.fromFilename(filename) ) fmt.fromShortName("mp4");
    /*avformat_alloc_output_context2( &oc, NULL, filename.c_str(), NULL);
    if (!oc) {
        ofLogError(OFX_ADDON) << "Could not allocate output context";
        return;
    }*/

    /* Determine the output format of the muxer from the extension of the file name : using dummy.mp4 */
    /*oc->oformat = av_guess_format(NULL, "dummy.mp4", NULL);
    if (!oc->oformat) {
        ofLogError(OFX_ADDON) << "Could not find suitable output format";
        return;
    }*/
    /* Determine if the container is a file or a stream */
    //isFileFormat = !(oc->oformat->flags & AVFMT_NOFILE);
    isFileFormat = fmt.isFileFormat();

    

    //add_video_stream();
    //st = avpp::Stream(fmt,enc);

    /* Setup the encoder */
    int fps = 30;
    enc = avpp::Encoder();
    /* open the codec */
    avpp::EncoderSettings settings;
    settings.codec_id = AV_CODEC_ID_H264;
    settings.width = compositingFbo.getWidth();
    settings.height = compositingFbo.getHeight();
    settings.fps = fps;

    enc.setup( settings );
    
    /* Allocate the encoded raw picture. */
    frame = avpp::Frame(enc->pix_fmt, settings.width, settings.height);

    fmt.addStream( settings );
    
    av_dump_format(fmt.native(), 0, filename.c_str(), 1);

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
    
    static struct SwsContext *sws_context = NULL;
    static int64_t next_pts = 0;
    
    uint8_t *rgb;
    const int in_linesize[1] = { 3 * frame.width() };
    
    rgb = pix.getData();
    sws_context = sws_getCachedContext(sws_context,
            frame.width(), frame.height(), AV_PIX_FMT_RGB24,
            frame.width(), frame.height(), enc->pix_fmt,
            0, NULL, NULL, NULL);
    sws_scale(sws_context, (const uint8_t * const *) &rgb, in_linesize, 0,
            frame.height(), frame.dataPtr(), frame.linesize() );
    
    //frame->pts = next_pts++;
    frame.pts( ofGetSystemTimeMicros() - movieStartTimeMicros);

    return;
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
void ScreenRecorder::draw( const ofFbo &fbo ){
    int ret;

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
    enc= fmt[0].getEncoder();
    AVPacket* pkt = av_packet_alloc();
    if( fmt.isRecording ){
        update_video_frame();
        if (frame.native() == NULL) ofLogError() << "NULL Frame";
        /* encode the image */
        //enc << frame;
        
        //if( !enc.encode(frame) ) return; 
        fmt[0].encode(frame);
        return;
        /*ret = avcodec_send_frame(enc.native(), frame.native());
        if (ret < 0) {
            fprintf(stderr, "Error submitting a frame for encoding\n");
        }*/
        ret = 0;
        while (ret >= 0) {
            //AVPacket pkt = { 0 };

            //av_init_packet(&pkt);
            
            ret = avcodec_receive_packet(enc.native(), pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                fprintf(stderr, "Error encoding a video frame\n");
            } else if (ret >= 0) {
                av_packet_rescale_ts(pkt, enc->time_base, fmt[0].timebase());
                pkt->stream_index = fmt[0].index();
                /* Write the compressed frame to the media file. */
                ret = av_interleaved_write_frame(fmt.native(), pkt);
                if (ret < 0) {
                    fprintf(stderr, "Error while writing video frame\n");
                }
            }
        }
    }
}

ScreenRecorder::~ScreenRecorder() {
    DEBUG
    stopRecordingMovie();
    DEBUG
}













