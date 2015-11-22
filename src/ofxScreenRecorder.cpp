
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ofxScreenRecorder.h"
#include "AV_Packet.h"

/*#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/channel_layout.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavformat/avformat.h"
#include "libavresample/avresample.h"
#include "libswscale/swscale.h"

#ifdef __cplusplus
}
#endif
*/


/* 5 seconds stream duration */
#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
//#define STREAM_PIX_FMT    AV_PIX_FMT_RGBA /* default pix_fmt */

#define SCALE_FLAGS SWS_BICUBIC



static bool registerAllCodecs()
{
   static bool registered = false;
   if (!registered)
   {
       avcodec_register_all();
       registered = true;
   }
   return registered;
}

ofxScreenRecorder::ofxScreenRecorder() : video_codec_id(-1), isCodecOpened(false), isOpened(false)
{
    registerAllCodecs();
}


ofxScreenRecorder::ofxScreenRecorder(int codec_id) : ofxScreenRecorder()
{
    setupCodec(codec_id);
}


ofxScreenRecorder::~ofxScreenRecorder()
{
    cleanup();
    av_free(ctx);
}


void ofxScreenRecorder::setupCodec(int codec_id)
{
    // Release the currently allocated codec context and/or file
    cleanup();

    // Look for the speciifed codec
    codec = avcodec_find_encoder((enum AVCodecID)codec_id);

    if (!codec) {
        ofLogWarning("ofxScreenRecorder") << "Codec not found";
        return;
    }

    allocateCodecContext();
}

void ofxScreenRecorder::setupCodec(string codec_name)
{
    // Release the currently allocated codec context and/or file
    if (ctx) {
        cleanup();
    }

    // Look for the speciifed codec
    codec = avcodec_find_encoder_by_name(codec_name.c_str());

    if (!codec) {
        ofLogWarning("ofxScreenRecorder") << "Codec not found";
        video_codec_id = -1;
        return;
    }

    allocateCodecContext();
}


void ofxScreenRecorder::setup(int width, int height, int fps)
{
    if (!ctx) {
        ofLogError("ofxScreenRecorder") << "Cannot setup. The codec is unavailable or wasn't succesfully allocated";
        return;
    }
    /* put sample parameters */
    //ctx->bit_rate = 4000000;
    /* resolution must be a multiple of two */
    //TODO : Check if resolution are multiples of 2
    ctx->width = width;
    ctx->height = height;

    /* frames per second */
    //TODO : Check if fps > 0
    ctx->time_base = (AVRational){1,fps};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    ctx->gop_size = 10;
    ctx->max_b_frames = 1;

    //ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->pix_fmt = AV_PIX_FMT_RGB24 ;

    if (video_codec_id == AV_CODEC_ID_H264) {
        ofLogWarning() << "H264 codec";
        av_opt_set(ctx->priv_data, "preset", "fast", 0);
        av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
        av_opt_set(ctx->priv_data, "qp", "18", 0);
    }

    /* open it */
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return;
    }
    isCodecOpened = true;


    avframe.setup(ctx);
    return ;
}

void ofxScreenRecorder::setup(const ofFbo &fbo, int fps)
{
    // Remember the FBO we are working with. We'll refer to it when writing frames
    recordedFbo = std::make_shared<ofFbo>(fbo);


    setup(fbo.getWidth(),fbo.getHeight(),fps);
}

bool ofxScreenRecorder::open(const char *filename)
{
    close();

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        return false;
    }
    isOpened = true;
    return true;
}


bool ofxScreenRecorder::writeframe()
{
    if (!isOpened || !recordedFbo) return false;

    /*AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;*/


    ofPixels pixels;
    recordedFbo->readToPixels(pixels);

    avframe << pixels;
    // Can also be written as
    //   avframe.scale(pixels);

    AV::Packet pack(ctx,avframe);
    pack.write(f);



    /*int got_output;
    int ret = avcodec_encode_video2(ctx, &pkt, avframe.getAVFrame(), &got_output);
    if (ret < 0) {
        fprintf(stderr, "Error encoding frame\n");
        exit(7);
    }
    if (got_output) {
        //printf("Write frame %3d (size=%5d)\n", i, pkt.size);
        fwrite(pkt.data, 1, pkt.size, f);
        av_free_packet(&pkt);
    }*/
    return true;
}




void ofxScreenRecorder::close()
{
    if (f && isOpened) {
        uint8_t endcode[] = { 0, 0, 1, 0xb7 };
        fwrite(endcode, 1, sizeof(endcode), f);
        fclose(f);
        isOpened = false;
    }
}



void ofxScreenRecorder::allocateCodecContext()
{
    video_codec_id = codec->id;
    // Allocate a new codec context
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        ofLogWarning("ofxScreenRecorder") << "Could not allocate video codec context";
    }
}

void ofxScreenRecorder::cleanup()
{
    if(ctx && isOpened)
    {
        close();
        avcodec_close(ctx);
        video_codec_id = AV_CODEC_ID_NONE;
    }
}
