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

    //bool setupEncoder( const EncoderSettings& settings );
    bool encode( const ofPixels &pix);
    bool writePacketsToFormat();
    bool writePacket();

    
    //friend bool Container::startRecording();
    friend class Container;
    bool fromEncoder( const Encoder &encoder );

protected:
    Stream( AVFormatContext *fmtctx, const EncoderSettings& settings );
//private:
    AVStream *st;
    AVRational encoderTimeBase;
    AVPacket* pkt;
    AVFormatContext *fmtctx;
    Encoder enc;
};

class VideoStream : protected Stream{
public:
    VideoStream();
    bool setupEncoder( const VideoEncoderSettings& settings );
    bool setupEncoder( const VideoEncoderSettings* settings, const ContainerSettings *contSettings );
    bool encode( const ofPixels &pix);

    //friend bool Container::startRecording();
    friend class Container;
protected:
    VideoStream( AVFormatContext *fmtctx, const VideoEncoderSettings& settings, ContainerSettings *contSettings );
    VideoStream( AVFormatContext *fmtctx, const VideoEncoderSettings* settings, const ContainerSettings *contSettings );
};

}