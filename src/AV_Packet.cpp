#include "AV_Packet.h"



namespace AV
{
Packet::Packet(AVCodecContext* ctx, Frame& frm) :
    isValid(false)
{
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    int got_pkt;
    /* encode the image */
    if (avcodec_encode_video2(ctx, &pkt, frm.getAVFrame(), &got_pkt) < 0)
    {
        ofLogError("AV_Packet") << "Error encoding frame";
        return;
    }
    isValid = (got_pkt > 0);
}

Packet::~Packet(){
    av_free_packet(&pkt);
}

bool Packet::write(FILE* f)
{
    if (isValid)
    {
        fwrite(pkt.data, 1, pkt.size, f);
        return true;
    }
    return false;
}

} // Enf of namespace AV
