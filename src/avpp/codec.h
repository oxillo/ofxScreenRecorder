#include "./frame.h"
#include "./settings.h"
extern "C" {
#include "libavutil/opt.h"
}


namespace avpp{




class Encoder {
public:
    Encoder();
    Encoder(const Encoder &) = delete; // No copy
    //Encoder(Encoder &&) = default; // Default move constructor
    Encoder(Encoder &&);  //Move constructor
    ~Encoder();

    template <typename Settings>
    bool setup( const Settings* settings, const ContainerSettings* containersettings );
    //bool encode( Frame& f);
    template <typename Data>
    bool encode( const Data &d );
    
    AVCodecContext* native();
    AVCodecContext* operator -> () {
        return enc;
    }

protected:
    AVCodecContext *enc;
    Frame frame;
};

template<> bool Encoder::setup(const VideoEncoderSettings* settings, const ContainerSettings* containersettings);
template<> bool Encoder::encode( const ofPixels &pix );


}