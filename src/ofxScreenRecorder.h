#pragma once
#include "ofMain.h"
#include "AV_Frame.h"




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


class ofxScreenRecorder
{

public:
    ofxScreenRecorder();
    ofxScreenRecorder(int codec_id);
    ofxScreenRecorder(string codec_name);

    ~ofxScreenRecorder();


    void setupCodec(int codec_id);
    void setupCodec(string codec_name);


    void setup(int width, int height, int fps=30);
    void setup(const ofFbo &fbo, int fps=30);
	
    bool open(const char *filename);
    void close();
	
    bool writeframe();
	
private :


    /**
    Close the file and the codec
    */
    void cleanup();
    void allocateCodecContext();




    AVCodec *codec;
    int video_codec_id;
	AVCodecContext *ctx;
    bool isCodecOpened;
    //AVFrame *frame;
    AV::Frame avframe;
	bool isOpened;
	FILE *f;
    std::shared_ptr<ofFbo> recordedFbo;
	uint8_t *rgba32Data;
};

