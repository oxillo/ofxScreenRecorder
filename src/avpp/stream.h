#pragma once

extern "C" {
#include "libavformat/avformat.h"
}
#include "ofLog.h"
#include "graphics/ofPixels.h"
#include "./codec.h"



namespace avpp{

class Container;


template<typename SettingType>
class Stream {
public:
    Stream():st(nullptr),pkt(nullptr), fmtctx(nullptr){}
    Stream(const Stream &other) = delete; // No copy

    /** Move constructor
    */
    Stream(Stream && other):enc(std::move(other.enc)){
        st = other.st;
        other.st = nullptr;
        pkt = other.pkt;
        other.pkt = nullptr;
        fmtctx = other.fmtctx;
        other.fmtctx = nullptr;
    }
    
    ~Stream() {}
    
    int index() {
        if( st ) return st->index;
        return -1;
    }

    bool setupEncoder( const SettingType* settings, const ContainerSettings *contSettings ){
        enc.setup( settings, contSettings );
        st->time_base = enc->time_base;
        /* copy the stream parameters to the muxer */
        avcodec_parameters_from_context(st->codecpar, enc.native());
        return true;
    }
    /*template <typename T>
    bool encode( const T &d );*/
    bool encode( const ofPixels &pix){
        if (!enc.encode(pix)) {
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            return false;
        }
        return writePacketsToFormat();
    }
    

    
    //friend bool Container::startRecording();
    friend class Container;
    bool fromEncoder( const Encoder &encoder );

protected:
    Stream( AVFormatContext *oc, const SettingType* settings, const ContainerSettings *contSettings ){
        fmtctx = oc;
        st = avformat_new_stream(fmtctx, NULL);
        setupEncoder( settings, contSettings );
        pkt = av_packet_alloc();
    }

private:
    AVStream *st;
    AVPacket* pkt;
    AVFormatContext *fmtctx;
    Encoder enc;

    bool writePacketsToFormat(){
        while (enc.getPacket( pkt )) {
            writePacket();
        }
        return true;
    }

    bool writePacket(){
        // rescale packet timestamp to stream timestamp
        av_packet_rescale_ts(pkt, enc->time_base, st->time_base);
        // set stream index
        pkt->stream_index = st->index;
        // Write packet to the output format (file or network)
        int ret = av_interleaved_write_frame(fmtctx, pkt);
        if( ret>= 0 ) return true; // Success
        return false; //Error
    }
};



typedef Stream<VideoEncoderSettings> VideoStream;
typedef Stream<AudioEncoderSettings> AudioStream;



}

