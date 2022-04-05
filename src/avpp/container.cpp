#include "avpp.h"

namespace avpp{
    
Container::Container() {
    isRecording = false;
    //oc = avformat_alloc_context()
}
Container::~Container(){
    stopRecording();
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    streams.clear();
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    avformat_free_context(oc);
    ofLogError()<<__FILE__<<"@"<<__LINE__;
}

Container& Container::operator=(Container&& other){
    oc = other.oc;
    other.oc = NULL;
    return *this;
}

bool Container::fromShortName(std::string name){
    return (avformat_alloc_output_context2( &oc, NULL, name.c_str(), NULL) >= 0);
}

bool Container::fromFilename(std::string name){
    filename=name;
    return (avformat_alloc_output_context2( &oc, NULL, NULL, name.c_str()) >= 0);
}

bool Container::isFileFormat(){
    return !(oc->oformat->flags & AVFMT_NOFILE);
}

bool Container::hasGlobalHeader(){
    return (oc->oformat->flags & AVFMT_GLOBALHEADER);
}

void Container::addStream(Encoder& enc){
    if( AVStream* st = avformat_new_stream(oc, NULL)){
        st->time_base = enc->time_base;
        /* Some formats want stream headers to be separate. */
        if( hasGlobalHeader() ) enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        /* copy the stream parameters to the muxer */
        avcodec_parameters_from_context(st->codecpar, enc.native());
        Stream s(st);
        streams.emplace_back(s);
    }
}


void Container::addStream(const EncoderSettings& settings){
    Stream s(oc,settings);
    streams.emplace_back(s);
}


Stream& Container::operator[](std::size_t idx){
    return streams[idx];
}

const Stream& Container::operator[](std::size_t idx) const{
    return streams[idx];
}

bool Container::startRecording(){
    if( isFileFormat()  
        && avio_open(&oc->pb, filename.c_str(), AVIO_FLAG_WRITE) >= 0 ){
        /* Write the stream header, if any. */
        avformat_write_header(oc, NULL);
        isRecording = true;
        return true;
    }
    return false;
}

bool Container::stopRecording(){
    if( isRecording ){
        av_write_trailer(oc);
        if( isFileFormat() ){
            avio_close(oc->pb);
        }
    }
    isRecording = false;
    return true;
}

AVFormatContext* Container::native(){
    return oc;
}


} // namespace avpp