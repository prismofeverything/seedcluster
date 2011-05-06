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

    void in( const Hand & hand, cv::Point _center );
    void out( cv::Point _center );
    void close( cv::Point _center );
    void open( cv::Point _center );
    void move( cv::Point _center );
    void drag( cv::Point _center );
    
    void update();
    void drawCircle( ci::Vec2f _center, float _radius, float _alpha );
    void draw();

    inline bool isOut() { return complete; };

    ci::Vec2f center;
    ci::Vec3f color;
    float radius;
    float alpha;
    float inertia;
    float fingerStretch;

    bool goingOut;
    bool complete;

    float fullRadius, closeRadius;

    Ease radiusEase;
    Ease alphaEase;
    Ease brightnessEase;
    Ease stretchEase;

    std::vector<ci::Vec2f> fingertips;
};

class HandMap {
 public:
    HandMap() {};
    HandCursor & get( const Hand & hand );
    void update( const HandTracker & tracker );
    void draw();
    void flush( const std::vector<Hand> & hands );

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
