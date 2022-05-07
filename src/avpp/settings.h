#pragma once

#include <string>
#include <map>
#include <optional>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}

/** 
* namespace for AV++ classes and functions
*/
namespace avpp{



/**
 * Abstract class for encoder settings. It must be specialized
 */
class EncoderSettings {
public:
    /** 
     * Get the coder ID. It is the same as libavcodec ID from 
     */
    virtual AVCodecID getCodecId() const {
        return _codec_id;
    };

    /** 
     * get the encoder name. 
     */
    virtual std::string getCodecName() const {
        return _codec_name;
    };

    /**
     * Check if the property is defined
     * param[in] prop : name of the property
     * returns true if the property is defined
     */
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
            ofLogError() << e.what() << ": Cannot parse int from " << prop;
            return std::nullopt;
        }
    }
    
    const std::map<std::string,std::string> &getPrivateData() const {return private_data;}

    bool hasGlobalHeader;

protected:
    /**
     * Define the encoder to use from its ID or name
     *
     * As multiple encoder may exist for the same format (for example when some hardware accelerated codec are available),
     * the name parameter is used to specify the encoder to use. 
     */
    EncoderSettings( AVCodecID codec, std::string name=""): _codec_id(codec), _codec_name(name){};
    
    AVCodecID _codec_id; /// requested encoder id 
    std::string _codec_name; /// requested encoder name - specify encoder when several exists for specified codec ID
    std::map<std::string,std::string> properties;
    std::map<std::string,std::string> private_data;
};

/**
 * The settings for a video encoder
 */
class VideoEncoderSettings : public EncoderSettings {
public:
    int width; /// the resulting width of the encoded movie
    int height; /// the resulting width of the encoded movie
    int fps; /// the requested framerate for the movie

    /**
     * returns the pixel format for the movie (RGB24,YUV)
     */
    virtual AVPixelFormat getPixelFormat() const {
        return _pixelFormat; 
    };

    

protected:
    AVPixelFormat _pixelFormat;  /// the requested pixel format
    

    /**
    ** Create settings with the specified codec If 'name' is specified, it will try to use the specified implementation of the encoder.
    ** This can be useful when there are multiple implementation x264, nvenc, h264
    */
    VideoEncoderSettings( AVCodecID codec, std::string name=""): EncoderSettings(codec,name) {
        properties["bit_rate"] = "4000000";
        properties["gop_size"] = "12";
        properties["max_b_frames"] = "1";
    }
};


/**
 * H264 encoder (YUV)
 */
class H264EncoderSettings : public VideoEncoderSettings {
public:
    H264EncoderSettings(std::size_t w, std::size_t h, float framerate=60.0):VideoEncoderSettings(AV_CODEC_ID_H264) {
        width = w;
        height = h;
        fps = framerate;
        _pixelFormat = AV_PIX_FMT_YUV444P;
        private_data["preset"]="fast";
    };
};


/**
 * RGB H264 encoder
 */
class H264RGBEncoderSettings : public VideoEncoderSettings {
public:
    H264RGBEncoderSettings(std::size_t w, std::size_t h, float framerate=60.0):VideoEncoderSettings(AV_CODEC_ID_H264,"libx264rgb") {
        width = w;
        height = h;
        fps = framerate;
        _pixelFormat = AV_PIX_FMT_RGB24;
        private_data["preset"]="fast";
    };
};


/**
 * H265 encoder (YUV)
 */
class H265EncoderSettings : public VideoEncoderSettings {
public:
    H265EncoderSettings(std::size_t w, std::size_t h, float framerate=60.0):VideoEncoderSettings(AV_CODEC_ID_H265) {
        width = w;
        height = h;
        fps = framerate;
        _pixelFormat = AV_PIX_FMT_YUV444P;
        private_data["preset"]="fast";
    };
};


/**
* Settings for audio encoder
*/
class AudioEncoderSettings : public EncoderSettings{
protected:
    
    /**
    ** Create settings with the specified codec If 'name' is specified, it will try to use the specified implementation of the encoder.
    ** This can be useful when there are multiple implementation x264, nvenc, h264
    */
    AudioEncoderSettings( AVCodecID codec, std::string name=""): EncoderSettings(codec,name) {
    }
};

/**
* Settings for subtitle encoder
*/
class SubtitleEncoderSettings : public EncoderSettings{
protected:
    
    /**
    ** Create settings with the specified codec If 'name' is specified, it will try to use the specified implementation of the encoder.
    ** This can be useful when there are multiple implementation x264, nvenc, h264
    */
    SubtitleEncoderSettings( AVCodecID codec, std::string name=""): EncoderSettings(codec,name) {
    }
};

/**
* Settings for the creation of a container. It describes the stream that will be used.
*/
class ContainerSettings {
public:
    /**
     *  Add a videostream using the videoEncoderSettings
     */
    template<typename T>
    auto& addVideoStream(std::size_t width, std::size_t height, float fps=60.0){
        videoStreamsSettings.emplace_back( std::make_shared<T>( width, height, fps ) );
        auto settings = std::static_pointer_cast<T>( videoStreamsSettings.back() );
        return *settings;
    }

    /**
     *  Add a audiostream using the audioEncoderSettings
     */
    template<typename T>
    auto& addAudioStream(){
        audioStreamsSettings.emplace_back( std::make_shared<T>( ) );
        auto settings = std::static_pointer_cast<T>( audioStreamsSettings.back() );
        return *settings;
    }

    /**
     *  Add a subtitle stream using SubtitleEncoderSettings
     */
    template<typename T>
    auto& addSubtitleStream(){
        subtitleStreamsSettings.emplace_back( std::make_shared<T>( ) );
        auto settings = std::static_pointer_cast<T>( subtitleStreamsSettings.back() );
        return *settings;
    }


    bool hasGlobalHeader;
    std::vector<std::shared_ptr<VideoEncoderSettings>> videoStreamsSettings;
    std::vector<std::shared_ptr<AudioEncoderSettings>> audioStreamsSettings;
    std::vector<std::shared_ptr<SubtitleEncoderSettings>> subtitleStreamsSettings;
    
};




}