#include "app/ofAppRunner.h"
#include "app/ofAppGLFWWindow.h"
#include "utils/ofConstants.h"

#include "ofApp.h"

//========================================================================
int main( ){

    ofGLFWWindowSettings settings;
    settings.setGLVersion(3,3); //Use OpenGL 3.3
    settings.decorated = true; // remove borders
    settings.resizable = true;
#if OF_VERSION_MINOR >= 10
    settings.setSize( 768, 512 );
#else
    settings.width = 768;
    settings.height = 512;
#endif
    //settings.setPosition(ofVec2f(0,0));
    shared_ptr<ofAppBaseWindow> win = ofCreateWindow(settings);

    shared_ptr<ofApp> app(new ofApp);

    ofRunApp(win, app);
    ofRunMainLoop();

}
