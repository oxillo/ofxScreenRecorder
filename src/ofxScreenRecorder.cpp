#include "ofxScreenRecorder.h"

#define DEBUG ofLogNotice() << "Line " << __LINE__;

#define OFX_ADDON "ofxScreenRecorder"



static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

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
    isRecording = false;
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
bool ScreenRecorder::add_video_stream(){
    
    st = avformat_new_stream(oc, NULL);
    if (!st) {
        ofLogError(OFX_ADDON) << "add_video_stream : could not allocate stream";
        return false;
    }

    st->time_base = enc->time_base;
    
    return true;
}

//===============================================================================
bool ScreenRecorder::setup_encoder( int width, int height, int fps){
    AVCodec *codec;
    
    
    /* find the video encoder */
    codec = avcodec_find_encoder( codec_id );
    if( !codec ){
        ofLogError(OFX_ADDON) << "setup_encoder : codec not found";
        return false;
    }

    enc = avcodec_alloc_context3( codec );
    if( !enc ){
        ofLogError(OFX_ADDON) << "setup_encoder : could not alloc an encoding context";
        return false;
    }
    
    /* Put sample parameters. */
    enc->bit_rate = 4000000;
    /* Resolution must be a multiple of two. */
    enc->width    = width;
    enc->height   = height;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    //enc->time_base       = (AVRational){ 1, fps };
    enc->time_base = (AVRational){ 1, 1000000}; // microseconds

    enc->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    enc->pix_fmt       = AV_PIX_FMT_YUV444P;
    //enc->pix_fmt       = AV_PIX_FMT_RGB24;

    enc->max_b_frames = 1;
    
    if( enc->codec_id == AV_CODEC_ID_MPEG2VIDEO ){
        /* just for testing, we also add B-frames */
        enc->max_b_frames = 2;
    }
    if( enc->codec_id == AV_CODEC_ID_MPEG1VIDEO ){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
        enc->mb_decision = 2;
    }
    if( codec_id == AV_CODEC_ID_H264 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }
    if( codec_id == AV_CODEC_ID_H265 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }

    return true;
}

//===============================================================================
void ScreenRecorder::open_video(std::string filename){
    int ret;

    /* Setup the encoder */
    int fps = 30;
    if( !setup_encoder( compositingFbo.getWidth(), compositingFbo.getHeight(), fps) ){
        ofLogError(OFX_ADDON) << "setup : could not setup encoder";
        return;
    }

    /* Allocate the output context for the muxer */
    oc = avformat_alloc_context();
    if (!oc) {
        ofLogError(OFX_ADDON) << "Could not allocate output context";
        return;
    }

    /* Determine the output format of the muxer from the extension of the file name : using dummy.mp4 */
    oc->oformat = av_guess_format(NULL, "dummy.mp4", NULL);
    if (!oc->oformat) {
        ofLogError(OFX_ADDON) << "Could not find suitable output format";
        return;
    }
    /* Determine if the container is a file or a stream */
    isFileFormat = !(oc->oformat->flags & AVFMT_NOFILE);

       /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    add_video_stream();


    /* open the codec */
    if (avcodec_open2(enc, NULL, NULL) < 0) {
        ofLogError() << "could not open codec";
    }

    /* Allocate the encoded raw picture. */
    frame = alloc_picture(enc->pix_fmt, enc->width, enc->height);
    if (!frame) {
        ofLogError() << "Could not allocate picture";
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(st->codecpar, enc);
    if (ret < 0) {
        ofLogError() << "Could not copy the stream parameters";
    }

    av_dump_format(oc, 0, filename.c_str(), 1);

    if (isFileFormat) {
        if (avio_open(&oc->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0) {
            ofLogError() << "Could not open '" << filename <<"'";
        }
    }

    /* Write the stream header, if any. */
    avformat_write_header(oc, NULL);
    isRecording = true;
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
    if( isRecording ) stopRecordingMovie();

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
    /* Write the trailer and close the file if needed */
    if( isRecording ){
        av_write_trailer(oc);
    
        if( isFileFormat ){
            avio_close(oc->pb);
        }
        avcodec_free_context(&enc);
        av_frame_free(&frame);
        avformat_free_context(oc);
    }
    isRecording = false;
}

//===============================================================================
void ScreenRecorder::update_video_frame(){
    
    static struct SwsContext *sws_context = NULL;
    static int64_t next_pts = 0;
    
    uint8_t *rgb;
    const int in_linesize[1] = { 3 * frame->width };
    
    rgb = pix.getData();
    sws_context = sws_getCachedContext(sws_context,
            frame->width, frame->height, AV_PIX_FMT_RGB24,
            frame->width, frame->height, enc->pix_fmt,
            0, NULL, NULL, NULL);
    sws_scale(sws_context, (const uint8_t * const *) &rgb, in_linesize, 0,
            frame->height, frame->data, frame->linesize);
    
    //frame->pts = next_pts++;
    frame->pts = ofGetSystemTimeMicros() - movieStartTimeMicros;

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
    
    if( isRecording ){
        update_video_frame();
        if (frame == NULL) ofLogError() << "NULL Frame";
        /* encode the image */
        ret = avcodec_send_frame(enc, frame);
        if (ret < 0) {
            fprintf(stderr, "Error submitting a frame for encoding\n");
        }
        while (ret >= 0) {
            AVPacket pkt = { 0 };

            av_init_packet(&pkt);

            ret = avcodec_receive_packet(enc, &pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                fprintf(stderr, "Error encoding a video frame\n");
            } else if (ret >= 0) {
                av_packet_rescale_ts(&pkt, enc->time_base, st->time_base);
                pkt.stream_index = st->index;

                /* Write the compressed frame to the media file. */
                ret = av_interleaved_write_frame(oc, &pkt);
                if (ret < 0) {
                    fprintf(stderr, "Error while writing video frame\n");
                }
            }
        }
    }
}

ScreenRecorder::~ScreenRecorder() {
    stopRecordingMovie();
}












