#pragma once

#include <algorithm>

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
        std::swap( st, other.st );
        std::swap( pkt, other.pkt );
        std::swap( fmtctx, other.fmtctx );
    }
    
    ~Stream() {}
    
    int index() {
        if( st ) return st->index;
        return -1;
    }

    bool setupEncoder( const SettingType* settings, const ContainerSettings *contSettings ){
        enc.setup( settings, contSettings );
        return enc.copyParametersToStream(st);
    }

    template <typename T>
    bool encode( const T &data ){
        if (!enc.encode(data)) {
            return false;
        }
        return writePacketsToFormat();
    }

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
        av_packet_rescale_ts(pkt, enc.getTimeBase(), st->time_base);
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

