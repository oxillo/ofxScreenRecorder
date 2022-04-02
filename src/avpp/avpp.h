
#ifdef __cplusplus
extern "C"
{
#endif

#include "libavcodec/avcodec.h"

#include "libavutil/frame.h"
#include "libavutil/imgutils.h"

#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#ifdef __cplusplus
}
#endif

namespace avpp{

class Encoder;

class Frame {
public:
    Frame();
    Frame(enum AVPixelFormat pix_fmt, int width, int height);
    Frame(const Frame &other) = delete; // no copy
    ~Frame();

    Frame& operator=(Frame&& other);

    int width() const;
    int height() const;
    int* linesize() const;
    void pts( int64_t timestamp);
    uint8_t** dataPtr();
    AVFrame* native();

    friend Encoder& operator<<( Encoder& enc, Frame& f);

    AVFrame *frame;
};

class Encoder {
public:
    Encoder();
    ~Encoder();
    Encoder& operator=(Encoder&& other);
    
    bool setup( int width, int height, int fps );
    bool encode( Frame& f);
    Encoder& operator<<( Frame& f);

    AVCodecContext* native();
    AVCodecContext* operator -> () {
        return enc;
    }
    AVCodecContext *enc;
};

}