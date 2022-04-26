#include "./codec.h"
#include "ofLog.h"

namespace avpp{
    
//-------------------------------------------------------------------------------------
Encoder::Encoder(): enc(nullptr){}

//-------------------------------------------------------------------------------------
Encoder::Encoder( Encoder &&other ) : frame( std::move(other.frame) ){
    enc = other.enc;
    other.enc = nullptr; 
    
}

Encoder::~Encoder(){
    // release libav encoder
    if( enc ) avcodec_free_context(&enc);
    enc = nullptr;
}


bool Encoder::setup(const VideoEncoderSettings* settings, const ContainerSettings* containerSettings){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    
    AVCodec *codec = avcodec_find_encoder( settings->getCodecId() );
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    //avcodec_find_encoder_by_name
    if( !codec ) return false;
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    enc = avcodec_alloc_context3( codec );
    if( !enc ) {
        ofLogError() << "setup_encoder : codec not found"; 
        return false;
    }
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    /* Some formats want stream headers to be separate. */
    if( containerSettings->hasGlobalHeader ) enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    /* Put sample parameters. */
    enc->bit_rate = 4000000;
    /* Resolution must be a multiple of two. */
    enc->width    = settings->width;
    enc->height   = settings->height;
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
    

    int ret = avcodec_open2(enc, NULL, NULL);
    frame.setup(enc->pix_fmt, enc->width, enc->height);
    return (ret == 0);
    //avcodec_find_encoder_by_name
}

template<> bool Encoder::setup(const VideoEncoderSettings& settings){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    
    AVCodec *codec = avcodec_find_encoder( settings.getCodecId() );
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    //avcodec_find_encoder_by_name
    if( !codec ) return false;
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    enc = avcodec_alloc_context3( codec );
    if( !enc ) {
        ofLogError() << "setup_encoder : codec not found"; 
        return false;
    }
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    /* Some formats want stream headers to be separate. */
    if( settings.hasGlobalHeader ) enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ofLogError()<<__FILE__<<"@"<<__LINE__;
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
    

    int ret = avcodec_open2(enc, NULL, NULL);
    frame.setup(enc->pix_fmt, enc->width, enc->height);
    return (ret == 0);
    //avcodec_find_encoder_by_name
}


bool Encoder::encode( const ofPixels &pix ){
    if( !enc ) return false;
    if( !frame.encode(pix) ) return false;
    return (avcodec_send_frame(enc, frame.native()) == 0);
}




AVCodecContext* Encoder::native(){
    return enc;
}

/*H265EncoderSettings::H265EncoderSettings(){
};*/

} // namespace avpp