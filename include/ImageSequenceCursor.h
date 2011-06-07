#pragma once
#include <map>
#include <vector>

#include "cinder/app/AppBasic.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "HandTracker.h"
#include "Ease.h"
#include "CinderOpenCv.h"
#include "ImageSequence.h"

using namespace cinder;
using namespace ci;
using namespace ci::app;

namespace ix 
{
    
class ImageSequenceCursor 
{
public:
    ImageSequenceCursor();
        
    void in( const Hand & hand, cv::Point _center );
    void out( cv::Point _center );
    void close( cv::Point _center );
    void open( cv::Point _center );
    void move( cv::Point _center );
    void drag( cv::Point _center );
        
    inline bool isOut() { return complete; };
        
    void setup();
    void update();
    void draw();
        
    bool complete;
    bool goingOut;
        
    ci::Vec2f anchor;
    ci::Vec2f center;
    ci::Vec2f shift;
    ci::Vec2f dshift;
    ci::Vec2f reflected;
    ci::Vec2f projected;
    ci::Vec2f previousReflected;
    ci::Vec2f sequenceOffset;
    
    ix::ImageSequence sequence;
    gl::Texture currentTexture;
    float sequenceScale;
};
    
} // namespace ix