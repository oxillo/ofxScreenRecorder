#include "avpp.h"

namespace avpp{
    
Container::Container() {
    isRecording = false;
}
Container::~Container(){
    stopRecording();
}

Container& Container::operator=(Container&& other){
    oc = other.oc;
    other.oc = NULL;
    return *this;
}

bool Container::fromShortName(std::string name){
    ofLogError()<<__FILE__<<"@"<<__LINE__;
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



void Container::addVideoStream(const VideoEncoderSettings& settings){
    videoStreamsSettings.push_back( settings );
}

/*void Container::addStream(const EncoderSettings& settings){
    streams.emplace_back( Stream{oc,settings} );
}*/


Stream& Container::operator[](std::size_t idx){
    return streams[idx];
}

const Stream& Container::operator[](std::size_t idx) const{
    ofLogError()<<__FILE__<<"@"<<__LINE__;
    return streams[idx];
}

bool Container::startRecording(){
    containerSettings.hasGlobalHeader = oc->oformat->flags & AVFMT_GLOBALHEADER;
    // Add video streams
    for (auto streamSettings: videoStreamsSettings) {
        streams.emplace_back( Stream{oc, &streamSettings, &containerSettings} );
    }
    av_dump_format(oc, 0, filename.c_str(), 1);
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
    streams.clear();
    if(oc) {
        avformat_free_context(oc);
        oc = nullptr;
    }
    isRecording = false;
    return true;
}

/*AVFormatContext* Container::native(){
    return oc;
}*/


} // namespace avpp