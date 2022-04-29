#pragma once

#include <string>
#include <map>
#include <optional>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
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

/**
 *
 */
class VideoEncoderSettings {
public:
    int width;
    int height;
    int fps;
    //enc->pix_fmt       = AV_PIX_FMT_YUV444P;
    //enc->pix_fmt       = AV_PIX_FMT_RGB24;
    bool hasGlobalHeader;

    virtual AVCodecID getCodecId() const {
        return _codec_id;
    };

    virtual std::string getCodecName() const {
        return _codec_name;
    };

    virtual AVPixelFormat getPixelFormat() const {
        return _pixelFormat; 
    };

    bool hasProperty( const std::string &prop ) const{
        return (properties.count( prop ) != 0);
    }
    
    /**
    *  Return property as an int if it is defined
    */
    std::optional<int> getIntProperty( const std::string &prop ) const {
        if( ! hasProperty(prop) ) return std::nullopt;
        try {
            return stoi( properties.at(prop) );
        } catch(std::invalid_argument &e) {
            return std::nullopt;
        }
    }
    
    const std::map<std::string,std::string> &getPrivateData() const {return private_data;}

protected:
    AVCodecID _codec_id;
    std::string _codec_name;
    AVPixelFormat _pixelFormat;
    std::map<std::string,std::string> properties;
    std::map<std::string,std::string> private_data;

    /**
    ** Create settings with the specified codec If 'name' is specified, it will try to use the specified implementation of the encoder.
    ** This can be useful when there are multiple implementation x264, nvenc, h264
    */
    VideoEncoderSettings( AVCodecID codec, std::string name=""):_codec_id(codec), _codec_name(name){
        properties["bit_rate"] = "4000000";
        properties["gop_size"] = "12";
        properties["max_b_frames"] = "1";
    }
};


/**
 * H264 encoder
 */
class H264EncoderSettings : public VideoEncoderSettings {
public:
    H264EncoderSettings():VideoEncoderSettings(AV_CODEC_ID_H264) {
        _pixelFormat = AV_PIX_FMT_YUV444P;
        private_data["preset"]="fast";
    };
};

class H264RGBEncoderSettings : public VideoEncoderSettings {
public:
    H264RGBEncoderSettings():VideoEncoderSettings(AV_CODEC_ID_H264,"libx264rgb") {
        _pixelFormat = AV_PIX_FMT_RGB24;
        private_data["preset"]="fast";
    };
};


/**
 * H265 encoder
 */
class H265EncoderSettings : public VideoEncoderSettings {
public:
    H265EncoderSettings():VideoEncoderSettings(AV_CODEC_ID_H265) {
        _pixelFormat = AV_PIX_FMT_YUV444P;
        private_data["preset"]="fast";
    };
};

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