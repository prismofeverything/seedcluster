#pragma once
#include <map>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "HandTracker.h"
#include "Ease.h"

namespace ix {

class HandCursor {
 public:
    HandCursor();

    void in( cv::Point _center );
    void out( cv::Point _center );
    void close( cv::Point _center );
    void open( cv::Point _center );
    void move( cv::Point _center );
    void drag( cv::Point _center );
    
    void update();
    void draw();

    inline bool isOut() { return complete; };

    ci::Vec2f center;
    ci::Vec3f color;
    float radius;
    float alpha;

    bool goingOut;
    bool complete;

    Ease radiusEase;
    Ease alphaEase;
    Ease brightnessEase;
};

class HandMap {
 public:
    HandMap() {};
    HandCursor & get( const Hand & hand );
    void update();
    void draw();

    void in( const Hand & hand );
    void out( const Hand & hand );
    void close( const Hand & hand );
    void open( const Hand & hand );
    void move( const Hand & hand );
    void drag( const Hand & hand );

    const static int smoothing = 10;
    std::map<float, HandCursor> handmap;
};

} // namespace ix
