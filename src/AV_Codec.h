#pragma once
#include "AV_Frame.h"



#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
}
#endif


namespace AV
{



class Encoder
{
public :
    Encoder();
    Encoder(int codec_id);
    Encoder(const string codec_name);
    ~Encoder();

    bool find(int codec_id);
    bool find(const string codec_name);

    bool isValid() {return (codec != NULL);}

private :
    AVCodec *codec;
};

}
