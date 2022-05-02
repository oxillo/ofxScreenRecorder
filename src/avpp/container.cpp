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


VideoStream& Container::video(std::size_t idx){
    return videoStreams[idx];
}

const VideoStream& Container::video(std::size_t idx) const{
    return videoStreams[idx];
}

AudioStream& Container::audio(std::size_t idx){
    return audioStreams[idx];
}

const AudioStream& Container::audio(std::size_t idx) const{
    return audioStreams[idx];
}

bool Container::startRecording(){
    containerSettings.hasGlobalHeader = oc->oformat->flags & AVFMT_GLOBALHEADER;
    // Add video streams
    for (auto streamSettings: settings().videoStreamsSettings2) {
        ofLogError(__FILE__)<<__LINE__ <<" width:"<<streamSettings->width;;
        videoStreams.emplace_back( VideoStream{oc, streamSettings.get(), &containerSettings} );
    }
    // Add audio streams
    for (auto streamSettings: settings().audioStreamsSettings) {
        audioStreams.emplace_back( AudioStream{oc, &streamSettings, &containerSettings} );
    }
    // Add subtitle streams
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
    videoStreams.clear();
    audioStreams.clear();
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


void ContainerSettings::addVideoStream(const VideoEncoderSettings& settings){
    videoStreamsSettings.push_back( settings );
}

void ContainerSettings::addAudioStream(const AudioEncoderSettings& settings){
    audioStreamsSettings.push_back( settings );
}

} // namespace avpp