
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

namespace avpp{

class Encoder;

class Frame {
public:
    Frame();
    Frame(enum AVPixelFormat pix_fmt, int width, int height);
    Frame(const Frame &other) = delete; // no copy
    ~Frame();

    Frame& operator=(Frame&& other);

    int width() const;
    int height() const;
    int* linesize() const;
    void pts( int64_t timestamp);
    uint8_t** dataPtr();
    AVFrame* native();

    friend Encoder& operator<<( Encoder& enc, Frame& f);

    AVFrame *frame;
};

class EncoderSettings {
public:
    int width;
    int height;
    int fps;
    AVCodecID codec_id;
};



class Encoder {
public:
    Encoder();
    ~Encoder();
    Encoder& operator=(Encoder&& other);
    
    bool setup( int width, int height, int fps );
    bool setup(const EncoderSettings& settings);
    bool encode( Frame& f);
    Encoder& operator<<( Frame& f);

    AVCodecContext* native();
    AVCodecContext* operator -> () {
        return enc;
    }

    //Packet& Encoder::getPacket();

private:
    AVCodecContext *enc;
    //Packet pkt;
};



class Packet {
public:
    Packet() {
        pkt = av_packet_alloc();
    }
    ~Packet(){
        av_packet_free(&pkt);
    }

    /*int receiveFrom(Encoder enc){
        ret = avcodec_receive_packet(enc.native(), pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                fprintf(stderr, "Error encoding a video frame\n");
            } else if (ret >= 0) {
    }*/

    AVPacket* native(){
        return pkt;
    }

    AVPacket* pkt;
};

class Stream;

class Container {
public:
    Container();
    ~Container();

    Container& operator=(Container&& other);

    bool fromShortName(std::string name);
    bool fromFilename(std::string name);

    bool isFileFormat();
    bool hasGlobalHeader();

    void addStream(Encoder& enc);
    //void addStream(const EncoderSettings& settings);
    
    Stream& operator[](std::size_t idx);
    const Stream& operator[](std::size_t idx) const;

    bool startRecording();
    bool stopRecording();

    
    AVFormatContext* native();

    bool isRecording;
    std::vector<Stream> streams;
    AVFormatContext *oc;
    std::string filename;
};

class Stream {
public:
    Stream();
    Stream( AVStream* stream);
    ~Stream();
    
    int index();

    bool setupEncoder( int width, int height, int fps );
    bool encode( Frame& f);
    //Stream& operator<<( Frame& f);

    AVRational timebase();

    AVStream *st;
    AVPacket* pkt;
    AVCodecContext *enc;
};

}