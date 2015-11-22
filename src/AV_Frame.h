#pragma once
#include "ofMain.h"



#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libswscale/swscale.h"
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif


#define DEBUG ofLogError(__FILE__) << "Line : " << __LINE__;

namespace AV
{



class Frame
{
public :
    Frame();
    Frame(int width, int height, enum AVPixelFormat pix_fmt);
    Frame(const AVCodecContext *ctx);
    ~Frame();

    bool setup(const AVCodecContext *ctx);
    bool scale(ofPixels& pixels);
    bool operator<<(ofPixels& pixels);

    bool isValid() {return frameValidity; }

    void fill(int frame_index);
    void copy(void *src);

    //Frame& operator++ ();
    //Frame operator++ (int);
    AVFrame* getAVFrame() {return frm;}

private :
    bool frameValidity;
    bool dataBufferIsInitialized;
    AVFrame *frm;
    SwsContext * sws_ctx;

};

}
