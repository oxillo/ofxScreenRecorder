
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
#include "./stream.h"


namespace avpp{

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
    
    VideoStream& video(std::size_t idx);
    const VideoStream& video(std::size_t idx) const;
    AudioStream& audio(std::size_t idx);
    const AudioStream& audio(std::size_t idx) const;

    bool startRecording();
    bool stopRecording();
    bool isRecordingActive() { return isRecording;}

    
private:
    std::vector<VideoEncoderSettings> videoStreamsSettings;
    std::vector<AudioEncoderSettings> audioStreamsSettings;
    ContainerSettings containerSettings;
    bool isRecording;
    std::vector<VideoStream> videoStreams;
    std::vector<AudioStream> audioStreams;
    AVFormatContext *oc;
    std::string filename;
};

}