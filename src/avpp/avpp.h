
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
#include "ofLog.h"
#include "graphics/ofPixels.h"



namespace avpp{

class Encoder;

class Frame {
public:
    Frame();
    Frame(enum AVPixelFormat pix_fmt, int width, int height);
    Frame(const Frame &other) = delete; // no copy
    Frame(Frame &&);        // move constructor
    ~Frame();

    //Frame& operator=(Frame&& other);

    bool setup(enum AVPixelFormat pix_fmt, int width, int height);

    int width() const;
    int height() const;
    int* linesize() const;
    void pts( int64_t timestamp);
    uint8_t** dataPtr();
    AVFrame* native();

    bool encode( const ofPixels &pix );

    friend Encoder& operator<<( Encoder& enc, Frame& f);

private:
    AVFrame *frame;
    struct SwsContext *sws_context;
};

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
    AVCodecID codec_id;
};

class AudioEncoderSettings {
public:
    bool hasGlobalHeader;
    AVCodecID codec_id;
};

class ContainerSettings {
public:
    std::vector<VideoEncoderSettings> videoStreams;
};



class Encoder {
public:
    Encoder();
    //Encoder(Encoder &&) = default; // Default move constructor
    Encoder(Encoder &&);  //Move constructor
    ~Encoder();

    bool setup(const EncoderSettings& settings);
    bool setup(const VideoEncoderSettings& settings);
    bool encode( Frame& f);
    bool encode( const ofPixels &pix );
    //bool encode( const std::vector<uint8_t> &pix );
    Encoder& operator<<( Frame& f);

    AVCodecContext* native();
    AVCodecContext* operator -> () {
        return enc;
    }

private:
    AVCodecContext *enc;
    Frame frame;
    //Packet pkt;
};




class Stream;
class VideoStream;

class Container {
public:
    Container();
    Container(const Container &other) = delete; // No copy
    Container(Container &&) = default; // Default move constructor
    ~Container();

    Container& operator=(Container&& other);

    bool fromShortName(std::string name);
    bool fromFilename(std::string name);

    bool isFileFormat();
    bool hasGlobalHeader();

    //void addStream(const EncoderSettings& settings);
    void addVideoStream(const VideoEncoderSettings& settings);
    void addAudioStream(const AudioEncoderSettings& settings);
    
    VideoStream& operator[](std::size_t idx);
    const VideoStream& operator[](std::size_t idx) const;

    bool startRecording();
    bool stopRecording();
    bool isRecordingActive() { return isRecording;}

    
private:
    std::vector<VideoEncoderSettings> videoStreamsSettings;
    bool isRecording;
    std::vector<VideoStream> streams;
    AVFormatContext *oc;
    std::string filename;
};

class Stream {
public:
    Stream();
    Stream(const Stream &other) = delete; // No copy
    Stream(Stream &&); // Default move constructor
    ~Stream();
    
    int index();

    bool setupEncoder( const EncoderSettings& settings );
    bool encode( const ofPixels &pix);
    
    friend bool Container::startRecording();

protected:
    Stream( AVFormatContext *fmtctx, const EncoderSettings& settings );
//private:
    AVStream *st;
    AVPacket* pkt;
    AVFormatContext *fmtctx;
    Encoder enc;
};

class VideoStream : protected Stream{
public:
    VideoStream();
    bool setupEncoder( const VideoEncoderSettings& settings );
    bool encode( const ofPixels &pix);

    friend bool Container::startRecording();
protected:
    VideoStream( AVFormatContext *fmtctx, const VideoEncoderSettings& settings );
};

}