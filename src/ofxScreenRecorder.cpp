/*
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat API example.
 *
 * @example output.c
 * Output a media file in any supported libavformat format. The default
 * codecs are used.
 */
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ofxLibav.h"

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

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;

    /* pts of the next frame that will be generated */
    int64_t next_pts;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    AVAudioResampleContext *avr;
} OutputStream;



/**************************************************************/
/* video output */

/* Add a video output stream. */
static void add_video_stream(OutputStream *ost, AVFormatContext *oc,
                             enum AVCodecID codec_id)
{
    AVCodecContext *c;
    AVCodec *codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        fprintf(stderr, "%d\n",__LINE__);
        exit(1);
    }

    ost->st = avformat_new_stream(oc, codec);
    if (!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = ost->st->codec;

    /* Put sample parameters. */
    c->bit_rate = 400000;
    /* Resolution must be a multiple of two. */
    c->width    = 1024;
    c->height   = 768;
    fprintf(stderr, "%d\n",__LINE__);

    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
    c->time_base       = ost->st->time_base;

    c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt       = STREAM_PIX_FMT;
    if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
        c->mb_decision = 2;
    }
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

static void open_video(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;

    c = ost->st->codec;

    /* open the codec */
    if (avcodec_open2(c, NULL, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        fprintf(stderr, "%d\n",__LINE__);
        exit(1);
    }

    fprintf(stderr, "%d : %d %d\n",__LINE__, c->width, c->height);

    /* Allocate the encoded raw picture. */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }
}

/* Prepare a dummy image. */
static void fill_yuv_image(AVFrame *pict, int frame_index,
                           int width, int height)
{
    int x, y, i, ret;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    ret = av_frame_make_writable(pict);
    if (ret < 0)
        exit(1);

    i = frame_index;

    /* Y */
    for (y = 0; y < height; y++)
        for (x = 0; x < width; x++)
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;

    /* Cb and Cr */
    for (y = 0; y < height / 2; y++) {
        for (x = 0; x < width / 2; x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

static AVFrame *get_video_frame(OutputStream *ost)
{
    AVCodecContext *c = ost->st->codec;

    /* check if we want to generate more frames */
    if (av_compare_ts(ost->next_pts, ost->st->codec->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return NULL;

    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        /* as we only generate a YUV420P picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_YUV420P,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SCALE_FLAGS, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Cannot initialize the conversion context\n");
                exit(1);
            }
        }
        fill_yuv_image(ost->tmp_frame, ost->next_pts, c->width, c->height);
        sws_scale(ost->sws_ctx, ost->tmp_frame->data, ost->tmp_frame->linesize,
                  0, c->height, ost->frame->data, ost->frame->linesize);
    } else {

        ofLogError() << "No scaling";
		fill_yuv_image(ost->frame, ost->next_pts, c->width, c->height);
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

/*
 * encode one video frame and send it to the muxer
 * return 1 when encoding is finished, 0 otherwise
 */
static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;
    AVCodecContext *c;
    AVFrame *frame;
    int got_packet = 0;

    c = ost->st->codec;

    frame = get_video_frame(ost);

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* a hack to avoid data copy with some raw video muxers */
        AVPacket pkt;
        av_init_packet(&pkt);

        if (!frame)
            return 1;

        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = ost->st->index;
        pkt.data          = (uint8_t *)frame;
        pkt.size          = sizeof(AVPicture);

        pkt.pts = pkt.dts = frame->pts;
        av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);

        ret = av_interleaved_write_frame(oc, &pkt);
    } else {
        AVPacket pkt = { 0 };
        av_init_packet(&pkt);

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
        if (ret < 0) {
            fprintf(stderr, "Error encoding a video frame\n");
            exit(1);
        }

        if (got_packet) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
        }
    }
    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }

    return (frame || got_packet) ? 0 : 1;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_close(ost->st->codec);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    avresample_free(&ost->avr);
}



AVFormatContext *oc;
int encode_video = 0;
int have_video = 0;
OutputStream video_st = { 0 };
AVOutputFormat *fmt;

void begin()
{
    std::string fn("movie.mp4");
    const char *filename;
    filename = fn.c_str();
    //AVOutputFormat *fmt;
    //AVFormatContext *oc;
    //int have_video = 0, have_audio = 0;
    //int encode_video = 0, encode_audio = 0;

    /* Initialize libavcodec, and register all codecs and formats. */
    av_register_all();


    /* Autodetect the output format from the name. default is MPEG. */
    fmt = av_guess_format(NULL, filename, NULL);
    fprintf(stderr, "%d\n",__LINE__);
    if (!fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        fmt = av_guess_format("mpeg", NULL, NULL);
    }
    fprintf(stderr, "%d\n",__LINE__);
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        return ;
    }

    /* Allocate the output media context. */
    fprintf(stderr, "%d\n",__LINE__);
    oc = avformat_alloc_context();
    fprintf(stderr, "%d\n",__LINE__);
    if (!oc) {
        fprintf(stderr, "Memory error\n");
        return ;
    }
    fprintf(stderr, "%d\n",__LINE__);
    oc->oformat = fmt;
    fprintf(stderr, "%d\n",__LINE__);
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

    /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */
     fprintf(stderr, "%d\n",__LINE__);
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        add_video_stream(&video_st, oc, fmt->video_codec);
        have_video = 1;
    }

    /* Now that all the parameters are set, we can open the audio and
     * video codecs and allocate the necessary encode buffers. */
    fprintf(stderr, "%d\n",__LINE__);
    if (have_video)
        open_video(oc, &video_st);
    fprintf(stderr, "%d\n",__LINE__);
    av_dump_format(oc, 0, filename, 1);
    fprintf(stderr, "%d\n",__LINE__);
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            return ;
        }
    }

    /* Write the stream header, if any. */
    avformat_write_header(oc, NULL);
}

void write_frames()
{
    int encode_video = 1;
    while (encode_video) {
        encode_video = !write_video_frame(oc, &video_st);
    }
}

void end()
{
    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(oc);

    /* Close each codec. */
    if (have_video)
        close_stream(oc, &video_st);

    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_close(oc->pb);

    /* free the stream */
    avformat_free_context(oc);
}

























namespace AV
{

Stream::Stream(AV::FormatContext *ctx, AVCodec *codec)
{
    index = ctx->avStreams.size();
    st = avformat_new_stream(ctx->oc, codec);
    if (st)
    {
        parent = ctx;

        /* Put sample parameters. */
        st->codec->bit_rate = 400000;
        /* Resolution must be a multiple of two. */
        st->codec->width    = 1440;
        st->codec->height   = 900;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };
        st->codec->time_base       = st->time_base;

        st->codec->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        st->codec->pix_fmt       = STREAM_PIX_FMT;
        if (st->codec->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B frames */
            st->codec->max_b_frames = 2;
        }
        if (st->codec->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            st->codec->mb_decision = 2;
        }
        /* Some formats want stream headers to be separate. */
        if (parent->oc->oformat->flags & AVFMT_GLOBALHEADER)
            st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

        if (avcodec_open2(st->codec, NULL, NULL) < 0) {
            fprintf(stderr, "could not open codec\n");
            fprintf(stderr, "%d\n",__LINE__);
        }
    }
}

Stream::~Stream()
{
    if (!st) return;
    DEBUG
    //close_stream(parent, &st);
}



}

bool AV::Packet::encodeFrame(AVCodecContext *avctx,AV::Frame &frame)
{
    int got_packet = 0;
    if (avcodec_encode_video2(avctx, &pkt, frame.picture, &got_packet) < 0)
    {
        fprintf(stderr, "Error encoding a video frame\n");
        return false;
    }
    return (got_packet!=0);
    /*if (got_packet)
    {
        av_packet_rescale_ts(&pkt, avctx->time_base, ost->st->time_base);
        //pkt.stream_index = ost->st->index;
    }*/
}

void AV::Frame::fill(int frame_index)
{
    int x, y, i;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    if (av_frame_make_writable(picture) < 0) return;

    i = frame_index;

	/*if (picture->format == AV_PIX_FMT_YUV420P) {
		// Y 
		for (y = 0; y < picture->height; y++)
			for (x = 0; x < picture->width; x++)
				picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;

		// Cb and Cr 
		for (y = 0; y < picture->height / 2; y++) {
			for (x = 0; x < picture->width / 2; x++) {
				picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
				picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
			}
		}
	}
	*/
	
	uint8_t *rgba32Data = new uint8_t[4*picture->width*picture->height];
	
	
	
	
    
    SwsContext * ctx = sws_getContext(picture->width, picture->height,
                                      AV_PIX_FMT_RGBA, picture->width, picture->height,
                                      AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
    
    
    /* encode 1 second of video */
        /* prepare a dummy image */
        /* Y */
        //        for (y = 0; y < c->height; y++) {
        //            for (x = 0; x < c->width; x++) {
        //                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        //            }
        //        }
        //
        //        /* Cb and Cr */
        //        for (y = 0; y < c->height/2; y++) {
        //            for (x = 0; x < c->width/2; x++) {
        //                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
        //                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
        //            }
        //        }
        
	uint8_t *pos = rgba32Data;
	for (y = 0; y < picture->height; y++)
	{
		for (x = 0; x < picture->width; x++)
		{
			pos[0] = i / (float)250 * 255;
			pos[1] = y / (float)(picture->height) * 255;
			pos[2] = x / (float)(picture->width) * 255;
			pos[3] = 255;
			pos += 4;
		}
	}

	uint8_t * inData[1] = { rgba32Data }; // RGBA32 have one plane
	//
	// NOTE: In a more general setting, the rows of your input image may
	//       be padded; that is, the bytes per row may not be 4 * width.
	//       In such cases, inLineSize should be set to that padded width.
	//
	int inLinesize[1] = { 4*picture->width }; // RGBA stride
	sws_scale(ctx, inData, inLinesize, 0, picture->height, picture->data, picture->linesize);
}

void AV::Frame::copy(void *src)
{
    int x, y;
    static int i = 0;


    if (src==NULL) return;
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    if (av_frame_make_writable(picture) < 0) return;

    /*for (y = 0; y < picture->height; y++)
        for (x = 0; x < picture->width; x++)
        {
            picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
            picture->data[1][y * picture->linesize[1] + x] = x + y + i * 3;
            picture->data[2][y * picture->linesize[2] + x] = x + y + i * 3;
            picture->data[3][y * picture->linesize[3] + x] = x + y + i * 3;
        }*/
    /* Cb and Cr */
    /*for (y = 0; y < picture->height / 2; y++) {
        for (x = 0; x < picture->width / 2; x++) {
            picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
            picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
        }
    }*/

    i++;

    ofLogNotice()<< ".";
    //memcpy(picture->data[0],src,picture->height*picture->width*4);
}



bool AV::Stream::write(AV::Packet &pkt)
{
    pkt.setStreamIndex(index);

    av_interleaved_write_frame(parent->oc, &pkt.pkt);
    return true;
}

bool AV::Stream::write(AV::Frame &frame)
{
    int got_packet = 0;
    AV::Packet pkt;
    //pkt = {0};
    //av_init_packet(&pkt);
	//ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    DEBUG
    if (avcodec_encode_video2(st->codec, &(pkt.pkt), frame.picture, &got_packet) < 0)
    {
        ofLogError() << "Error encoding a video frame";
        return false;
    }
    DEBUG
    av_packet_rescale_ts(&(pkt.pkt), st->codec->time_base, st->time_base);
    //pkt.stream_index = index;
    DEBUG
    pkt.setStreamIndex(index);
    DEBUG
    /* Write the compressed frame to the media file. */
    /*ret = */av_interleaved_write_frame(parent->oc, &(pkt.pkt));
    DEBUG
    return (got_packet!=0);
    /*if (got_packet)
    {
        av_packet_rescale_ts(&pkt, avctx->time_base, ost->st->time_base);
        //pkt.stream_index = ost->st->index;
    }*/
}

//Codec::codecregistered = false;