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



class Packet
{
public :
    Packet(AVCodecContext* ctx, Frame& frm);
    ~Packet();

    bool write(FILE* f);

private :
    AVPacket pkt;
    bool isValid;

};

}
