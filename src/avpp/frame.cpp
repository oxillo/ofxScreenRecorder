#include "avpp.h"
#include "ofLog.h"

namespace avpp{
    
Frame::Frame(){}

Frame::Frame(enum AVPixelFormat pix_fmt, int width, int height) {
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

Frame& Frame::operator=(Frame&& other) {
    frame = other.frame;
    other.frame = NULL;
    return *this;
}



Frame::~Frame(){
    if( frame ) av_frame_free(&frame);
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

AVFrame* Frame::native(){
    return frame;
}

} // namespace avpp