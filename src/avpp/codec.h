#pragma once
/**
* @file codec.h
* Encoder object definition
*/

#include "./frame.h"
#include "./settings.h"
extern "C" {
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
}


namespace avpp{


//class Stream;

/**
* 
*/
class Encoder {
public:
    Encoder();
    Encoder(const Encoder &) = delete; // No copy
    Encoder(Encoder &&);  //Move constructor
    ~Encoder();

    /** 
     * Setup encoder from settings 
     */
    template <typename Settings>
    bool setup( const Settings* settings, const ContainerSettings* containersettings );
    
    /**
     * Encode the provided data
     */
    template <typename Data>
    bool encode( const Data &d );

    /**
     * Retrieve encoded data
     */
    bool getPacket(AVPacket* pkt);

    /**
     * Retrieve the time base for this encoder
     */
    AVRational getTimeBase(){
        if( enc ) return enc->time_base;
        // Default to microsecond timebase.
        return (AVRational){ 1, 1000000};
    }

    /**
     * Copy codec parameter into stream. 
     * Resize time stamp to match stream time-base
     */
    bool copyParametersToStream(AVStream* st){
        if( enc ){
            st->time_base = enc->time_base;
            // copy the stream parameters to the muxer
            avcodec_parameters_from_context(st->codecpar, enc);;
            return true;
        }
        return false;
    }
    
    AVCodecContext* native();
    /*AVCodecContext* operator -> () {
        return enc;
    }*/

protected:
    AVCodecContext *enc;    ///< The libav encoder
    Frame frame;            ///< The frame (audio or video) associated to this encode
};

template<> bool Encoder::setup(const VideoEncoderSettings* settings, const ContainerSettings* containersettings);
template<> bool Encoder::setup(const AudioEncoderSettings* settings, const ContainerSettings* containersettings);

template<> bool Encoder::encode( const ofPixels &pix );


}