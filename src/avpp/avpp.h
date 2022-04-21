
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


#define TRACE_VERBOSE ofLogVerbose()<<__FILE__<<"@"<<__LINE__<<" - " << __PRETTY_FUNCTION__ ;

namespace avpp{

class Encoder;

class Frame {
public:
    Frame();
    Frame(enum AVPixelFormat pix_fmt, int width, int height);
    Frame(const Frame &other) = delete; // no copy
    //Frame(Frame &&) = default; // Default move constructor
    Frame(Frame &&);
    ~Frame();

    Frame& operator=(Frame&& other);

    bool setup(enum AVPixelFormat pix_fmt, int width, int height);

    int width() const;
    int height() const;
    int* linesize() const;
    void pts( int64_t timestamp);
    uint8_t** dataPtr();
    AVFrame* native();

    bool encode( const ofPixels &pix );

    friend Encoder& operator<<( Encoder& enc, Frame& f);

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



class Encoder {
public:
    Encoder();
    //Encoder(Encoder &&) = default; // Default move constructor
    Encoder(Encoder &&);
    //Encoder& operator=(Encoder&& other) = default;
    ~Encoder();

    bool setup( int width, int height, int fps );
    bool setup(const EncoderSettings& settings);
    bool encode( Frame& f);
    bool encode( const ofPixels &pix );
    //bool encode( const std::vector<uint8_t> &pix );
    Encoder& operator<<( Frame& f);

    AVCodecContext* native();
    AVCodecContext* operator -> () {
        return enc;
    }

    //Packet& Encoder::getPacket();

private:
    AVCodecContext *enc;
    Frame frame;
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
    Container(const Container &other) = delete; // No copy
    Container(Container &&) = default; // Default move constructor
    ~Container();

    Container& operator=(Container&& other);

    bool fromShortName(std::string name);
    bool fromFilename(std::string name);

    bool isFileFormat();
    bool hasGlobalHeader();

    //void addStream(Encoder& enc);
    void addStream(const EncoderSettings& settings);
    
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
    Stream(const Stream &other) = delete; // No copy
    //Stream(Stream &&) = default; // Default move constructor
    Stream(Stream &&); // Default move constructor
    Stream( AVFormatContext *fmtctx, const EncoderSettings& settings );
    Stream( AVStream* stream);
    ~Stream();
    
    int index();

    bool setupEncoder( const EncoderSettings& settings );
    bool encode( Frame& f);
    bool encode( const ofPixels &pix);
    const Encoder& getEncoder();

    AVPacket* getPacket() { return pkt;}
    //Stream& operator<<( Frame& f);

    AVRational timebase();

    AVStream *st;
    AVPacket* pkt;
    AVFormatContext *fmtctx;
    Encoder enc2;
};

}