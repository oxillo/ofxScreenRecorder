extern "C" {
#include "libavcodec/avcodec.h"
}


namespace avpp{



/**
 * 
 */
class EncoderSettings {
public:
    int width;
    int height;
    int fps;
    bool hasGlobalHeader;
    AVCodecID codec_id;
};

class VideoEncoderSettings {
public:
    int width;
    int height;
    int fps;
    bool hasGlobalHeader;

    static VideoEncoderSettings H264() {
        return VideoEncoderSettings( AV_CODEC_ID_H264 );
    }

    static VideoEncoderSettings H265() {
        return VideoEncoderSettings( AV_CODEC_ID_H265 );
    }

    virtual AVCodecID getCodecId() const {
        ofLogError()<< __FILE__ << "@" << __LINE__;
        return codec_id; };
private:
    AVCodecID codec_id;
    VideoEncoderSettings( AVCodecID codec ){
        codec_id = codec;
    }
};

/*class H264EncoderSettings : public VideoEncoderSettings {
public:
    H264EncoderSettings();
    AVCodecID getCodecId() const override 
    {
        ofLogError()<< __FILE__ << "@" << __LINE__;
        return AV_CODEC_ID_H264;};
};

class H265EncoderSettings : public VideoEncoderSettings {
public:
    H265EncoderSettings();
    AVCodecID getCodecId() const override {
        ofLogError()<< __FILE__ << "@" << __LINE__;
        return AV_CODEC_ID_H265;};
};*/
//typedef ofPixels_<unsigned char> ofPixels;

class AudioEncoderSettings {
public:
    bool hasGlobalHeader;
    AVCodecID codec_id;
};

class ContainerSettings {
public:
    bool hasGlobalHeader;
    std::vector<VideoEncoderSettings> videoStreams;
};




}