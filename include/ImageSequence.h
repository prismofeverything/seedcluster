#pragma once;

#include <string>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;

class ImageSequence
{
public:
    ImageSequence();
    ~ImageSequence();
    
    void createFromDir( string path );
    
    void update();
    void play();
    void pause();
    void stop();
    
    // -- getters/ setters
    void setPlayheadFrameInc( int frames ) { playheadFrameInc = frames; }
    int getPlayheadFrameInc() { return playheadFrameInc; }
    void setPlayheadPosition( int newPosition );
    int getPlayheadPosition() { return playheadPosition; }
    void setLooping( bool doLoop ) { looping = doLoop; }
    Texture getCurrentTexture() { return textures[ playheadPosition ]; }
    bool isPlaying() { return playing; }
    bool isPaused() { return paused; }
    
protected:
    int playheadPosition;
    int playheadFrameInc;
    vector<Texture> textures;
    
    int totalFrames;
    bool looping;
    bool paused;
    bool playing;
    bool complete;
};