#include "./frame.h"
#include "ofLog.h"

namespace avpp{
    
Frame::Frame(){
    frame = nullptr;
    sws_context = nullptr;
}

Frame::Frame(enum AVPixelFormat pix_fmt, int width, int height) {
    ofLogError()<<__FILE__<<"@"<<__LINE__<< " " << this;
    frame = av_frame_alloc();
    if( frame ){
        frame->format = pix_fmt;
        frame->width  = width;
        frame->height = height;
        /* allocate the buffers for the frame data */
        int ret = av_frame_get_buffer(frame, 32);
        if (ret >= 0) return;
    }
    ofLogError() << "Could not allocate frame data.";
}

Frame::Frame( Frame &&other ){
    frame = other.frame;
    other.frame = nullptr;
    sws_context = other.sws_context;
    other.sws_context = nullptr;
}


/*Frame& Frame::operator=(Frame&& other) {
    frame = other.frame;
    other.frame = NULL;
    return *this;
}*/

bool Frame::setup(enum AVPixelFormat pix_fmt, int width, int height){
    frame = av_frame_alloc();
    if( !frame ) return false;
    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;
    /* allocate the buffers for the frame data */
    int ret = av_frame_get_buffer(frame, 32);
    return (ret >=0 );
}



Frame::~Frame(){
    sws_freeContext(sws_context);
    if( frame ) av_frame_free(&frame);
}

/**
Rescale an input image to the width, height, color format of the frame so it can be fed to the encoder.
**/
bool Frame::encode( const ofPixels &pix ){
    if( !frame ) return false;
    ofLogVerbose()<<__FILE__<<"@"<<__LINE__<<" frame&:"<<this<<" size:" <<frame->width << "x" << frame->height;
    // Retrieve or create a scaling context 
    sws_context = sws_getCachedContext(sws_context,
            pix.getWidth(), pix.getHeight(), AV_PIX_FMT_RGB24,
            frame->width, frame->height, (AVPixelFormat)frame->format,
            0, NULL, NULL, NULL);
    // Rescale the input image
    const uint8_t *rgb = pix.getData();
    const int in_linesize[1] = { 3 * int(pix.getWidth()) };
    sws_scale(sws_context, &rgb, in_linesize, 
            0, pix.getHeight(), 
            frame->data, frame->linesize );
    frame->pts = ofGetSystemTimeMicros();
    return true;
}

int Frame::width() const{
    if( frame ) return frame->width;
    return 0;
}

int Frame::height() const{
    if( frame ) return frame->height;
    return 0;
}

int* Frame::linesize() const{
    if( frame ) return frame->linesize;
    return 0;
}

void Frame::pts( int64_t timestamp){
    if( frame ) frame->pts = timestamp;
}

uint8_t** Frame::dataPtr(){
    if( frame ) return frame->data;
    return NULL;
}

/*bool Frame::encode( const std::vector<uint8_t> &pix ){
    return true;
}*/

AVFrame* Frame::native(){
    return frame;
}

} // namespace avpp