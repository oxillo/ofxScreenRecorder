#include "avpp.h"
#include "ofLog.h"

namespace avpp{
    
//Encoder::Encoder(AVCodecID codec_id): enc(NULL){
Encoder::Encoder(): enc(NULL){
    /*AVCodecID codec_id = AV_CODEC_ID_H264;
    AVCodec *codec = avcodec_find_encoder( codec_id );
    //avcodec_find_encoder_by_name
    if( codec ){
        enc = avcodec_alloc_context3( codec );
        if( enc ) return;
    }
    ofLogError() << "setup_encoder : codec not found"; 
    //avcodec_find_encoder_by_name*/
}


/*Encoder& Encoder::operator=(Encoder&& other) {
    enc = other.enc;
    other.enc = NULL;
    return *this;
}*/


Encoder::~Encoder(){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    //if( enc ) avcodec_free_context(&enc);
    ofLogError()<<__FILE__<<"@"<<__LINE__;
}


bool Encoder::setup( int width, int height, int fps ){
    EncoderSettings settings;
    settings.codec_id = AV_CODEC_ID_H264;
    settings.width = width;
    settings.height = height;

    return setup(settings);

    AVCodec *codec = avcodec_find_encoder( settings.codec_id );
    //avcodec_find_encoder_by_name
    if( codec ){
        enc = avcodec_alloc_context3( codec );
        if( !enc ) return false;
    }
    ofLogError() << "setup_encoder : codec not found"; 
    //avcodec_find_encoder_by_name
    if( !enc ) return false;
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
    if( enc->codec_id == AV_CODEC_ID_H264 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }
    if( enc->codec_id == AV_CODEC_ID_H265 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }
    return (avcodec_open2(enc, NULL, NULL) == 0);
}

bool Encoder::setup(const EncoderSettings& settings){
    AVCodec *codec = avcodec_find_encoder( settings.codec_id );
    //avcodec_find_encoder_by_name
    if( !codec ) return false;
    enc = avcodec_alloc_context3( codec );
    if( !enc ) {
        ofLogError() << "setup_encoder : codec not found"; 
        return false;
    }

    /* Some formats want stream headers to be separate. */
    if( settings.hasGlobalHeader ) enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    
    /* Put sample parameters. */
    enc->bit_rate = 4000000;
    /* Resolution must be a multiple of two. */
    enc->width    = settings.width;
    enc->height   = settings.height;
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
    if( enc->codec_id == AV_CODEC_ID_H264 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }
    if( enc->codec_id == AV_CODEC_ID_H265 ){
        av_opt_set(enc->priv_data, "preset", "fast", 0);
    }
    return (avcodec_open2(enc, NULL, NULL) == 0);
    
    //avcodec_find_encoder_by_name
}

bool Encoder::encode(Frame& f){
    if( !enc ) return false;
    return (avcodec_send_frame(enc, f.native()) == 0);
}

Encoder& Encoder::operator<<( Frame& f){
    if( enc ) avcodec_send_frame(enc, f.native());
    return *this;
}

/*Packet& Encoder::getPacket(){
    int ret = avcodec_receive_packet(enc.native(), pkt.native());
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                fprintf(stderr, "Error encoding a video frame\n");
            } else if (ret >= 0) {
}*/



AVCodecContext* Encoder::native(){
    return enc;
}

} // namespace avpp