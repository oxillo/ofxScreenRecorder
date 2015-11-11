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

void begin();

void write_frames();

void end();


#define DEBUG ofLogError(__FILE__) << "Line : " << __LINE__;

namespace AV
{

class FormatContext;
class Frame;
class Stream;

class Packet
{
public:
    Packet()
    {
        pkt = {0};
        av_init_packet(&pkt);
    }

    void setFlags (int flags)
    {
        pkt.flags = flags;
    }

    int getFlags()
    {
        return pkt.flags;
    }

    void setStreamIndex(int index)
    {
        pkt.stream_index = index;
    }

    int getStreamIndex()
    {
        return pkt.stream_index;
    }


    void rescaleTimeStamp(AVRational src, AVRational dest)
    {
        av_packet_rescale_ts(&pkt, src, dest);
    }

    void setPresentationTimeStamp(int64_t pts)
    {
        pkt.pts = pts;
    }

    int64_t getPresentationTimeStamp()
    {
        return pkt.pts;
    }

    void setDecompressionTimeStamp(int64_t dts)
    {
        pkt.dts = dts;
    }

    int64_t getDecompressionTimeStamp()
    {
        return pkt.dts;
    }


    bool encodeFrame(AVCodecContext *avctx,AV::Frame &frame);





    //void setData(uint)
    //pkt.data          = (uint8_t *)frame;

    //getData()

    //setSize()
    // pkt.size          = sizeof(AVPicture);





private:
    AVPacket pkt;

friend class AV::Stream;
};

class Stream
{
public:
    Stream(AV::FormatContext *ctx, AVCodec *codec);

    ~Stream();

    inline bool isValid() { return (st!=NULL); }

    bool write(AV::Packet &pkt);

    bool write(AV::Frame &picture);

    int getStreamIndex() {return index;};



private :
    int index;
    AVStream *st;
    AV::FormatContext *parent;


};


class FormatContext
{
public:
    FormatContext() : oc(NULL)
    {
        DEBUG
        oc = avformat_alloc_context();
        if (!oc)
        {
            ofLogError() << "Cannot allocate a format context";
        }
    }


    void setFilename(const std::string &filename)
    {
        AVOutputFormat *fmt;


        fmt = av_guess_format(NULL, filename.c_str(), NULL);
        if (!fmt)
        {
            ofLogWarning() << "Could not deduce output format from file extension: using MPEG.";
            fmt = av_guess_format("mpeg", NULL, NULL);
        }
        if (!fmt)
        {
            ofLogError() << "Could not find suitable output format";
        }

        oc->oformat = fmt;
        snprintf(oc->filename, sizeof(oc->filename), "%s", filename.c_str());
    }

    std::string getFilename()
    {
        return std::string(oc->filename);
    }


    // return the index on the created stream
    // return -1 in case of error
    int addVideoStream()
    {
        AVCodec *codec;
        /* find the video encoder */
        //codec = avcodec_find_encoder(oc->oformat->video_codec);
		AVDictionary *options = NULL;
		//av_dict_set(&options, "video_size", "640x480", 0);
		//av_dict_set(&options, "pixel_format", "rgb24", 0);
		//av_dict_set(&options, "tune", "zerolatency", 0);
		//codec = avcodec_find_encoder_by_name("x264");
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		

        if (!codec)
        {
            fprintf(stderr, "codec not found\n");
            return -1;
        }
		
		codec_ctx = avcodec_alloc_context3(codec);
		
		/* put sample parameters */
		codec_ctx->bit_rate = 400000;
		/* resolution must be a multiple of two */
		codec_ctx->width = 1440;
		codec_ctx->height = 900;
		/* frames per second */
		codec_ctx->time_base = (AVRational){1,25};
		/* emit one intra frame every ten frames
		 * check frame pict_type before passing frame
		 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
		 * then gop_size is ignored and the output of encoder
		 * will always be I frame irrespective to gop_size
		 */
		codec_ctx->gop_size = 10;
		codec_ctx->max_b_frames = 1;
		codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

        av_opt_set(codec_ctx->priv_data, "preset", "ultrafast", 0);
		
		
		if (avcodec_open2(codec_ctx, codec, &options) < 0)
			exit(1);

        AV::Stream new_stream(this,codec);
        if (new_stream.isValid())
        {
            avStreams.push_back(new_stream);
            return avStreams.back().getStreamIndex();
        }

        //avcodec_close(codec);
        return -1;

    }

    // Write the stream header, if any.
    void writeHeader()
    {
        if (!oc) return;
        avformat_write_header(oc, NULL);
    }

    // Write the stream trailer
    void writeTrailer()
    {
        if (!oc) return;
        DEBUG
        av_write_trailer(oc);
        DEBUG
    }

    void open()
    {
        DEBUG
        /* open the output file, if needed */
        if (!(oc->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&oc->pb, oc->filename, AVIO_FLAG_WRITE) < 0) {
                DEBUG
                fprintf(stderr, "Could not open '%s'\n", oc->filename);
            }
        }
        DEBUG
        writeHeader();
        DEBUG
        inUse = true;
    }
    // write the trailer
    // Close all the streams
    // Close the file (if necessary)
    void close()
    {
        if (!inUse) return;
        DEBUG
        writeTrailer();
        DEBUG
        if (!oc) return;
        DEBUG
        avStreams.clear();
        DEBUG

        if (!(oc->oformat->flags & AVFMT_NOFILE))
        {
            /* Close the output file. */
            avio_close(oc->pb);
			avcodec_free_context(&codec_ctx);
        }
        inUse = false;



    }

    ~FormatContext()
    {
        DEBUG
        close();
        if (oc)
        {
            // writeTrailer();
            DEBUG
            avformat_free_context(oc);
        }
    }

    AV::Stream & getStream(int index)
    {
        return avStreams[index];
    }



private:
    AVFormatContext *oc;
	AVCodecContext *codec_ctx;
    std::vector<AV::Stream> avStreams;
    bool inUse;

friend class AV::Stream;
};


class Frame
{
public :
    Frame(enum AVPixelFormat pix_fmt, int width, int height)
    {
        picture = av_frame_alloc();
        frameValidity = (picture != NULL);
        if (!frameValidity) return;
        picture->format = pix_fmt;
        picture->width = width;
        picture->height = height;

        /* the image can be allocated by any means and av_image_alloc() is
         * just the most convenient way if av_malloc() is to be used */
        int ret = av_image_alloc(picture->data, picture->linesize, width, height,pix_fmt, 32);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw picture buffer\n");
        }

        dataBufferIsInitialized = (av_frame_get_buffer(picture, 32) >= 0);
    }

    bool scale()
    {
       // sws_scale(sws_ctx, inData, inLinesize, 0, ctx->height, frame->data, frame->linesize);

    }

    ~Frame()
    {
        if (!frameValidity) return;
        av_frame_free(&picture);
    }

    inline bool isValid() {return frameValidity; }

    void fill(int frame_index);
    void copy(void *src);

private :
    bool frameValidity;
    bool dataBufferIsInitialized;
    AVFrame *picture;
    SwsContext * sws_ctx;

friend class AV::Packet;
friend class AV::Stream;
};


/*
AVFrame *picture;
    int ret;


    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;*/

}


class ofxScreenRecorder
{
	//static AllCodecs allcodecs;

public:
    ofxScreenRecorder() : video_codec_id(-1), isCodecOpened(false), isOpened(false)
    {
        registerAllCodecs();
    }


    ofxScreenRecorder(int codec_id) : ofxScreenRecorder()
	{
        setupCodec(codec_id);
	}


    void setupCodec(int codec_id)
    {
        // Release the currently allocated codec context and/or file
        if (ctx) {
            cleanup();
        }

        // Look for the speciifed codec
        codec = avcodec_find_encoder((enum AVCodecID)codec_id);
        if (!codec) {
            ofLogWarning("ofxScreenRecorder") << "Codec not found";
            video_codec_id = -1;
            return;
        }
        video_codec_id = codec_id;

        // Allocate a new codec context
        ctx = avcodec_alloc_context3(codec);
        if (!ctx) {
            ofLogWarning("ofxScreenRecorder") << "Could not allocate video codec context";
        }
    }


    void setup(int width, int height, int fps=30)
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

		ctx->pix_fmt = AV_PIX_FMT_YUV420P;
		//ctx->pix_fmt = AV_PIX_FMT_RGB ;
		
        if (video_codec_id == AV_CODEC_ID_H264) {
            av_opt_set(ctx->priv_data, "preset", "ultrafast", 0);
            av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
            av_opt_set(ctx->priv_data, "qp", "18", 0);
        }
		
		/* open it */
		if (avcodec_open2(ctx, codec, NULL) < 0) {
			fprintf(stderr, "Could not open codec\n");
            return;
		}
        isCodecOpened = true;
		
		
        //frame = Frame(ctx->pix_fmt,ctx->width,ctx->height);
        frame = av_frame_alloc();
		if (!frame) {
			fprintf(stderr, "Could not allocate video frame\n");
            return ;
		}
		frame->format = ctx->pix_fmt;
		frame->width  = ctx->width;
        frame->height = ctx->height;
		
		/* the image can be allocated by any means and av_image_alloc() is
		 * just the most convenient way if av_malloc() is to be used */
		int ret = av_image_alloc(frame->data, frame->linesize, ctx->width, ctx->height,
							 ctx->pix_fmt, 32);
		if (ret < 0) {
			fprintf(stderr, "Could not allocate raw picture buffer\n");
            return ;
		}
		sws_ctx = sws_getContext(ctx->width, ctx->height,
                                      AV_PIX_FMT_RGBA, ctx->width, ctx->height,
                                      AV_PIX_FMT_YUV420P , 0, 0, 0, 0);
		
		rgba32Data = new uint8_t[4*ctx->width*ctx->height];
	
        return ;
	}

    void setup(const ofFbo &fbo, int fps=30)
    {
        // Remember the FBO we are working with. We'll refer to it when writing frames
        recordedFbo = std::make_shared<ofFbo>(fbo);


        setup(fbo.getWidth(),fbo.getHeight(),fps);
    }
	
	bool open(const char *filename)
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
	
	bool writeframe(int i)
	{
		if (!isOpened) return false;
		AVPacket pkt;
		av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        
        
        //fflush(stdout);
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
        for (int y = 0; y < ctx->height; y++)
        {
            for (int x = 0; x < ctx->width; x++)
            {
                pos[0] = i / (float)250 * 255;
                pos[1] = 0;
                pos[2] = x / (float)(ctx->width) * 255;
				pos[2] = y / (float)(ctx->height) * 255;
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
        int inLinesize[1] = { 4*ctx->width }; // RGBA stride
        sws_scale(sws_ctx, inData, inLinesize, 0, ctx->height, frame->data, frame->linesize);
        
        frame->pts = i;
        
		int got_output;
        /* encode the image */
        int ret = avcodec_encode_video2(ctx, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(7);
        }
        
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
		return true;
	}



    bool writeframe()
    {
        if (!isOpened || !recordedFbo) return false;

        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;

        int width = recordedFbo->getWidth();
        int height = recordedFbo->getHeight();




        ofPixels pixels;
        recordedFbo->readToPixels(pixels);
        uint8_t * inData[1] = { pixels.getData() }; // RGBA32 have one plane
        //
        // NOTE: In a more general setting, the rows of your input image may
        //       be padded; that is, the bytes per row may not be 4 * width.
        //       In such cases, inLineSize should be set to that padded width.
        //
        int inLinesize[1] = { 4*width }; // RGBA stride
        sws_scale(sws_ctx, inData, inLinesize, 0, height, frame->data, frame->linesize);
        frame->pts = frame->pts + 1;

        int got_output;
        /* encode the image */
        int ret = avcodec_encode_video2(ctx, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(7);
        }
        if (got_output) {
            //printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
        return true;
    }
	
	


    void close()
    {
        if (f && isOpened) {
            uint8_t endcode[] = { 0, 0, 1, 0xb7 };
            fwrite(endcode, 1, sizeof(endcode), f);
            fclose(f);
            isOpened = false;
        }
    }
	
    ~ofxScreenRecorder()
	{
        cleanup();
		av_free(ctx);
	}
	
private :
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

    /**
    Close the file and the codec
    */
    void cleanup()
    {
        if (isOpened)
        {
            close();
            avcodec_close(ctx);
        }
    }




    AVCodec *codec;
    int video_codec_id;
	AVCodecContext *ctx;
    bool isCodecOpened;
	SwsContext * sws_ctx;
    AVFrame *frame;
	bool isOpened;
	FILE *f;
    std::shared_ptr<ofFbo> recordedFbo;
	uint8_t *rgba32Data;
};

