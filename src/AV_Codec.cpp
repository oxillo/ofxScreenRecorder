#include "AV_Codec.h"



namespace AV
{

bool registerAllCodecs()
{
   static bool registered = false;
   if (!registered)
   {
       avcodec_register_all();
       registered = true;
   }
   return registered;
}

Encoder::Encoder()
    :codec(NULL)
{
    registerAllCodecs();
}

Encoder::Encoder(int codec_id)
{
    find(codec_id);
}

Encoder::Encoder(const string codec_name)
{
    find(codec_name);
}

Encoder::~Encoder()
{
    codec = NULL;
}

bool Encoder::find(int codec_id)
{
    codec = avcodec_find_encoder((enum AVCodecID)codec_id);

    return (codec != NULL);
}

bool Encoder::find(const string codec_name)
{
    codec = avcodec_find_encoder_by_name(codec_name.c_str());

    return (codec != NULL);
}

} // Enf of namespace AV
