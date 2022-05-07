
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

    VideoStream& video(std::size_t idx);
    const VideoStream& video(std::size_t idx) const;
    AudioStream& audio(std::size_t idx);
    const AudioStream& audio(std::size_t idx) const;
    SubtitleStream& subtitle(std::size_t idx);
    const SubtitleStream& subtitle(std::size_t idx) const;

    bool startRecording();
    bool stopRecording();
    bool isRecordingActive() { return isRecording;}
    ContainerSettings& settings() {return containerSettings;}
    const ContainerSettings& settings() const {return containerSettings;}

    
private:
    ContainerSettings containerSettings;
    bool isRecording;
    std::vector<VideoStream> videoStreams;
    std::vector<AudioStream> audioStreams;
    std::vector<SubtitleStream> subtitleStreams;
    AVFormatContext *oc;
    std::string filename;
};

}