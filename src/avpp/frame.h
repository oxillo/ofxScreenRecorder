#pragma once

extern "C"
{

#include "libavcodec/avcodec.h"

#include "libavutil/frame.h"
#include "libswscale/swscale.h"

} // extern "C"

#include "graphics/ofPixels.h"



namespace avpp{

class Frame {
public:
    Frame();
    Frame(enum AVPixelFormat pix_fmt, int width, int height);
    Frame(const Frame &other) = delete; // no copy
    Frame(Frame &&);        // move constructor
    ~Frame();

    bool setup(enum AVPixelFormat pix_fmt, int width, int height);

    int width() const;
    int height() const;
    int* linesize() const;
    void pts( int64_t timestamp);
    uint8_t** dataPtr();
    AVFrame* native();

    bool encode( const ofPixels &pix );

private:
    AVFrame *frame;
    struct SwsContext *sws_context;
};

} // namespace avpp