#include "avpp.h"

namespace avpp{
Stream::Stream():st(nullptr),pkt(nullptr), fmtctx(nullptr){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
}


Stream::Stream(Stream && other):enc2(std::move(other.enc2)){
    st = other.st;
    other.st = nullptr;
    pkt = other.pkt;
    other.pkt = nullptr;
    fmtctx = other.fmtctx;
    other.fmtctx = nullptr;
}
/*Stream::Stream(const Stream &other){
    
}*/

Stream::Stream( AVFormatContext *oc, const EncoderSettings& settings ){
    fmtctx = oc;
    st = avformat_new_stream(fmtctx, NULL);
    auto encoderSettings = settings;
    encoderSettings.hasGlobalHeader = fmtctx->oformat->flags & AVFMT_GLOBALHEADER;
    setupEncoder(encoderSettings);
    pkt = av_packet_alloc();
}

Stream::Stream( AVStream* stream){
    pkt = av_packet_alloc();
    st = stream;
}

Stream::~Stream() {
    //if( pkt ) av_packet_free(&pkt);
}

int Stream::index(){
    if( st ) return st->index;
    return -1;
}

AVRational Stream::timebase(){
    if( st ) return st->time_base;
    return { 1, 1 };
}


bool Stream::setupEncoder( const EncoderSettings& settings ){
    enc2.setup( settings );
    st->time_base = enc2->time_base;
    /* copy the stream parameters to the muxer */
    avcodec_parameters_from_context(st->codecpar, enc2.native());
    return true;
}

const Encoder& Stream::getEncoder(){
    return enc2;
}

bool Stream::encode( Frame& f){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    if (!enc2.encode(f)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc2.native(), pkt);
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
        } else if (ret >= 0) {
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            av_packet_rescale_ts(pkt, enc2->time_base, st->time_base);
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            pkt->stream_index = st->index;
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            /* Write the compressed frame to the media file. */
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            ret = av_interleaved_write_frame(fmtctx, pkt);
            ofLogError()<<__FILE__<<"@"<<__LINE__;
            if (ret < 0) {
                fprintf(stderr, "Error while writing video frame\n");
            }
        }
    }
    return true;
    /*if( !enc ) return false;
    return (avcodec_send_frame(enc, f.native()) == 0);*/
}

bool Stream::encode( const ofPixels &pix){
    if (!enc2.encode(pix)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc2.native(), pkt);
        ofLogVerbose()<<__FILE__<<"@"<<__LINE__<<" - " << __PRETTY_FUNCTION__ ;
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
        } else if (ret >= 0) {
            av_packet_rescale_ts(pkt, enc2->time_base, st->time_base);
            pkt->stream_index = st->index;
            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(fmtctx, pkt);
            if (ret < 0) {
                fprintf(stderr, "Error while writing video frame\n");
            }
        }
    }
    return true;
    /*if( !enc ) return false;
    return (avcodec_send_frame(enc, f.native()) == 0);*/
}



} // namespace avpp