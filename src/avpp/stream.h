#pragma once

extern "C" {
#include "libavformat/avformat.h"
}
#include "ofLog.h"
#include "graphics/ofPixels.h"
#include "./codec.h"



namespace avpp{

class Container;

class Stream {
public:
    Stream();
    Stream(const Stream &other) = delete; // No copy
    Stream(Stream &&); // Default move constructor
    ~Stream();
    
    int index();

    template <typename T>
    bool setupEncoder( const T* settings, const ContainerSettings *contSettings );
    /*template <typename T>
    bool encode( const T &d );*/
    bool encode( const ofPixels &pix);
    bool writePacketsToFormat();
    bool writePacket();

    
    //friend bool Container::startRecording();
    friend class Container;
    bool fromEncoder( const Encoder &encoder );

protected:
    template <typename T>
    Stream( AVFormatContext *fmtctx, const T* settings, const ContainerSettings *contSettings );
//private:
    AVStream *st;
    AVPacket* pkt;
    AVFormatContext *fmtctx;
    Encoder enc;
};

template<>
Stream::Stream( AVFormatContext *oc, const VideoEncoderSettings* settings, const ContainerSettings *containerSettings );

template<>
bool Stream::setupEncoder( const VideoEncoderSettings* settings, const ContainerSettings *contSettings );

}