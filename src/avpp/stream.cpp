#include "avpp.h"

namespace avpp{
Stream::Stream():st(nullptr),pkt(nullptr), fmtctx(nullptr){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
}

VideoStream::VideoStream():Stream(){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
}


Stream::Stream(Stream && other):enc(std::move(other.enc)){
    st = other.st;
    other.st = nullptr;
    pkt = other.pkt;
    other.pkt = nullptr;
    fmtctx = other.fmtctx;
    other.fmtctx = nullptr;
}

Stream::Stream( AVFormatContext *oc, const EncoderSettings& settings ){
    fmtctx = oc;
    st = avformat_new_stream(fmtctx, NULL);
    auto encoderSettings = settings;
    encoderSettings.hasGlobalHeader = fmtctx->oformat->flags & AVFMT_GLOBALHEADER;
    setupEncoder(encoderSettings);
    pkt = av_packet_alloc();
}

VideoStream::VideoStream( AVFormatContext *oc, const VideoEncoderSettings& settings ){
    fmtctx = oc;
    st = avformat_new_stream(fmtctx, NULL);
    auto encoderSettings = settings;
    encoderSettings.hasGlobalHeader = fmtctx->oformat->flags & AVFMT_GLOBALHEADER;
    setupEncoder(encoderSettings);
    pkt = av_packet_alloc();
}

Stream::~Stream() {
    //if( pkt ) av_packet_free(&pkt);
}

int Stream::index(){
    if( st ) return st->index;
    return -1;
}


bool Stream::setupEncoder( const EncoderSettings& settings ){
    enc.setup( settings );
    st->time_base = enc->time_base;
    /* copy the stream parameters to the muxer */
    avcodec_parameters_from_context(st->codecpar, enc.native());
    return true;
}

bool VideoStream::setupEncoder( const VideoEncoderSettings& settings ){
    enc.setup( settings );
    st->time_base = enc->time_base;
    /* copy the stream parameters to the muxer */
    avcodec_parameters_from_context(st->codecpar, enc.native());
    return true;
}


bool Stream::encode( const ofPixels &pix){
    if (!enc.encode(pix)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc.native(), pkt);
        ofLogVerbose()<<__FILE__<<"@"<<__LINE__<<" - " << __PRETTY_FUNCTION__ ;
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
        } else if (ret >= 0) {
            av_packet_rescale_ts(pkt, enc->time_base, st->time_base);
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

bool VideoStream::encode( const ofPixels &pix){
    if (!enc.encode(pix)) {
        ofLogError()<<__FILE__<<"@"<<__LINE__;
        return false;
    }
    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc.native(), pkt);
        ofLogVerbose()<<__FILE__<<"@"<<__LINE__<<" - " << __PRETTY_FUNCTION__ ;
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
        } else if (ret >= 0) {
            av_packet_rescale_ts(pkt, enc->time_base, st->time_base);
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