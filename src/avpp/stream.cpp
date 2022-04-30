#include "stream.h"

namespace avpp{
Stream::Stream():st(nullptr),pkt(nullptr), fmtctx(nullptr){
}

VideoStream::VideoStream():Stream(){
}

Stream::Stream(Stream && other):enc(std::move(other.enc)){
    st = other.st;
    other.st = nullptr;
    pkt = other.pkt;
    other.pkt = nullptr;
    fmtctx = other.fmtctx;
    other.fmtctx = nullptr;
}

VideoStream::VideoStream( AVFormatContext *oc, const VideoEncoderSettings& settings, ContainerSettings *contSettings ){
    fmtctx = oc;
    st = avformat_new_stream(fmtctx, NULL);
    auto encoderSettings = settings;
    encoderSettings.hasGlobalHeader = fmtctx->oformat->flags & AVFMT_GLOBALHEADER;
    setupEncoder(&encoderSettings, contSettings );
    pkt = av_packet_alloc();
}

VideoStream::VideoStream( AVFormatContext *oc, const VideoEncoderSettings* settings, const ContainerSettings *containerSettings ){
    fmtctx = oc;
    st = avformat_new_stream(fmtctx, NULL);
    setupEncoder( settings, containerSettings );
    pkt = av_packet_alloc();
}

Stream::~Stream() {
    //if( pkt ) av_packet_free(&pkt);
}

int Stream::index(){
    if( st ) return st->index;
    return -1;
}


/*bool Stream::setupEncoder( const EncoderSettings& settings ){
    enc.setup( settings );
    st->time_base = enc->time_base;ContainerSettings contSettings;
    // copy the stream parameters to the muxer 
    avcodec_parameters_from_context(st->codecpar, enc.native());
    return true;
}*/

bool VideoStream::setupEncoder( const VideoEncoderSettings& settings ){
    ContainerSettings *contSettings = new ContainerSettings();
    contSettings->hasGlobalHeader = fmtctx->oformat->flags & AVFMT_GLOBALHEADER;
    
    enc.setup( &settings, contSettings );
    st->time_base = enc->time_base;
    /* copy the stream parameters to the muxer */
    avcodec_parameters_from_context(st->codecpar, enc.native());
    return true;
}


bool VideoStream::setupEncoder( const VideoEncoderSettings* settings, const ContainerSettings *contSettings ){
    enc.setup( settings, contSettings );
    st->time_base = enc->time_base;
    /* copy the stream parameters to the muxer */
    avcodec_parameters_from_context(st->codecpar, enc.native());
    return true;
}


bool Stream::writePacketsToFormat(){
    while (enc.getPacket( pkt )) {
        writePacket();
    }
    return true;
}

bool Stream::writePacket(){
    // rescale packet timestamp to stream timestamp
    av_packet_rescale_ts(pkt, enc->time_base, st->time_base);
    // set stream index
    pkt->stream_index = st->index;
    // Write packet to the output format (file or network)
    int ret = av_interleaved_write_frame(fmtctx, pkt);
    if( ret>= 0 ) return true; // Success
    return false; //Error
}

bool Stream::encode( const ofPixels &pix){
    if (!enc.encode(pix)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    return writePacketsToFormat();
}

bool VideoStream::encode( const ofPixels &pix){
    if (!enc.encode(pix)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    return writePacketsToFormat();
}




} // namespace avpp