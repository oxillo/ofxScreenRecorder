
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
#include "./codec.h"



namespace avpp{


class ContainerSettings {
public:
    std::vector<VideoEncoderSettings> videoStreams;
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

    //bool setupEncoder( const EncoderSettings& settings );
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
    bool setupEncoder( const VideoEncoderSettings* settings );
    bool encode( const ofPixels &pix);

    friend bool Container::startRecording();
protected:
    VideoStream( AVFormatContext *fmtctx, const VideoEncoderSettings& settings );
    VideoStream( AVFormatContext *fmtctx, const VideoEncoderSettings* settings );
};

}