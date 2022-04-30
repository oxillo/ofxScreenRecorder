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

AVCodec* findCodec( AVCodecID id, std::string name ){
    // Try first to retrieve the codec by its name
    AVCodec *codec = nullptr;
    if( name.length()>0 ){
        codec = avcodec_find_encoder_by_name( name.c_str() );
        if( codec ){
            ofLogNotice()<<"Using "<< name << " codec.";
            return codec;
        }
    }
    // If that fails (or because no name was provided), try to find it by id
    codec = avcodec_find_encoder( id );
    if( codec ){
        ofLogNotice()<<"Using "<< codec->name << " codec from ID:" << codec->id;
        return codec;
    }
    // Cannot find a codec, return false for an error
    ofLogError()<<"Could not find a matching codec";
    return nullptr;
}

template<>
bool Encoder::setup(const VideoEncoderSettings* settings, const ContainerSettings* containerSettings){
    auto codec = findCodec( settings->getCodecId(), settings->getCodecName());
    if( !codec ) return false;
    
    // Allocate the encoder from the codec
    enc = avcodec_alloc_context3( codec );
    if( !enc ) {
        ofLogError() << "setup_encoder : codec not found"; 
        return false;
    }

    // Some formats want stream headers to be separate.
    if( containerSettings->hasGlobalHeader ) enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    
    // Put sample parameters. 
    auto bit_rate = settings->getIntProperty("bit_rate");
    if( bit_rate ) enc->bit_rate = *bit_rate;
    /* Resolution must be a multiple of two. */
    enc->width    = settings->width;
    enc->height   = settings->height;
    // pixel format ()
    enc->pix_fmt       = settings->getPixelFormat();
    /* timebase: This is the fundamental unit of time (in seconds) in terms
    * of which frame timestamps are represented. For fixed-fps content,
    * timebase should be 1/framerate and timestamp increments should be
    * identical to 1. */
    //enc->time_base       = (AVRational){ 1, fps };
    enc->time_base = (AVRational){ 1, 1000000}; // microseconds

    // Group of picture
    auto gop_size = settings->getIntProperty("gop_size");
    if( gop_size ) enc->gop_size = *gop_size;

    // Maximum consecutive B(idirectional) frames (4-5 typical 16 max)
    auto max_b_frames = settings->getIntProperty("max_b_frames");
    if( max_b_frames ){
        enc->max_b_frames = *max_b_frames;
    }else{
        enc->max_b_frames = 4;
    }
    
    if( enc->codec_id == AV_CODEC_ID_MPEG1VIDEO ){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
        * This does not happen with normal video, it just happens here as
        * the motion of the chroma plane does not match the luma plane. */
        enc->mb_decision = 2;
    }
    
    for (const auto& [key, value] : settings->getPrivateData()) {
        av_opt_set(enc->priv_data, key.c_str(), value.c_str(), 0);
    }

    // Open the encoder
    int ret = avcodec_open2(enc, NULL, NULL);

    frame.setup(enc->pix_fmt, enc->width, enc->height);
    return (ret == 0);
}

template<> 
bool Encoder::setup(const AudioEncoderSettings* settings, const ContainerSettings* containersettings){
    return true;
}

bool Encoder::getPacket(AVPacket* pkt){
    auto ret = avcodec_receive_packet(enc, pkt);
    if( ret>=0 ) return true;
    if( ret==AVERROR(EAGAIN) ) return false;
    if( ret==AVERROR_EOF ) return false;
    ofLogError()<<"Error encoding frame";
    return false;
}


template<>
bool Encoder::encode( const ofPixels &pix ){
    if( !enc ) return false;
    if( !frame.encode(pix) ) return false;
    return (avcodec_send_frame(enc, frame.native()) == 0);
}




AVCodecContext* Encoder::native(){
    return enc;
}

} // namespace avpp