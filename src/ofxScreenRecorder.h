#pragma once
#include "ofMain.h"
#include <stdio.h>
#include "avpp/avpp.h"




class ScreenRecorder {
public:
    ScreenRecorder();
    ~ScreenRecorder();

    /**
     * \brief Setup a video recorder
     * 
     * The \p width and \p height indicates the size of the frames in the 
     * target video. It is not necessary the same as the input frames
     * but should it is recommended to keep the aspect ratio.
     * The size of the video will be the specified width and will have
     * an increased height to integrate title and legend
     * \a width will be automatically to a multiple of 16 for performance reason. 
     * 
     * \param width int
     * \param height int
     */
    bool setup( int width, int height );

    /** \brief
     *  Takes a snapshot of current video recorder frame content.
     * 
     * Record of the frame as an image with filename \p filename.
     * If \p filename is "", the name will "snapshot_XXXX.png" where XXXX
     * is a number that starts at 0000 and is automatically incremented.
     * 
     * \param filename std::string
     *
     */
    void snapshot( std::string filename = "" );
    
    /** \brief
     *  Starts recording a movie.
     * 
     * Starts the record of a movie using \p filename as filename.
     * If \p filename is "", the name will "movie_XXXX.mp4" where XXXX
     * is a number that starts at 0000 and is automatically incremented.
     * Starting a new movie when an other one is recording automatically calls
     * stopRecordingMovie()
     *
     * \param filename std::string
     *
     */
    void startRecordingMovie( std::string filename = "" );

    /** \brief
     *  Stops recording.
     * 
     */
    void stopRecordingMovie();

    /** \brief
     *  Returns if recording is active.
     * 
     * \return bool
     *
     */
    bool isRecordingActive(){
        return isRecording; 
    }

    /** \brief
     *  Define the title string included at the top of the video
     *
     * 
     * \param title std::string
     *
     */
	void addTitle(std::string newTitle){
        recorderTitle = newTitle; 
    }
    
    /** \brief
     *  Define the legend string included at the bottom of the video
     *
     * \param legend std::string
     *
     */
	void addLegend(std::string newLegend){
        recorderLegend = newLegend;
    }

    /** \brief
     *  Define the logo to be displayed in the upper-right corner
     *
     * The logo is resized to fit the height of the title bar
     * 
     * \param newLogo ofImage
     *
     */
	void addLogo(ofImage newLogo);
	
    /** \brief
     *  Draw the content of \param fbo inside the movie
     *
     * The content of \p fbo is rescaled to fit inside the size defined in 
     * setup()
     * 
     * \param fbo ofFbo
     *
     */
	void draw( const ofFbo &fbo );
    

private :
    bool isFileFormat;      //< Indicates that the recording is going to a file, not to a network stream
    bool setupCompleted;    //< Indicates that setup() has been called and everything is OK
    bool isRecording;       //< Indicates that a recording is active
    
    ofFbo compositingFbo;   //< Intermediate frame buffer for compositing user frame, title and legend.
    float compositingFboLeft; //< X position of compositingFbo

    int frameWidth, frameHeight;    //< Size of the frame inside the video
    ofPixels pix;           //< The pixels from compositingFbo

    ofImage logo;           //< The logo to add to title bar
    float logoLeft;         //< X position of logo


    avpp::Container fmt;
    uint64_t movieStartTimeMicros;
    
	std::string recorderTitle;
    ofTrueTypeFont titleFont;
    std::string recorderLegend;
    ofTrueTypeFont legendFont;
	//float titleHeight;  /// The height reserved for the title at the top of the recorded image
    //float legendHeight; /// The height of a legend line. Will reserve space for 6 legend lines at the bottom
    float titleAnchor;  /// Where to start the drawing of the title (from top)
    float legendAnchor; /// Where to start the drawing of the legend (from bottom of FBO)
    

    void update_video_frame();
    bool add_video_stream();
    void open_video(std::string filename = "");
    bool setup_encoder(int width, int height, int fps=30);

};
