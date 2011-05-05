#include <map>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "HandTracker.h"
#include "Ease.h"

class HandCursor {
 public:
    HandCursor();

    void in( ci::Vec2f _center );
    void out( ci::Vec2f _center );
    void close( ci::Vec2f _center );
    void open( ci::Vec2f _center );
    void move( ci::Vec2f _center );
    void drag( ci::Vec2f _center );
    
    void draw();

    ci::Vec2f center;
    float radius;
    float alpha;

    Ease radiusEase;
    Ease alphaEase;
    Ease brightnessEase;
};

class HandMap {
 public:
    HandMap() {};
    HandCursor get( const Hand & hand );

    void in( const Hand & hand );
    void out( const Hand & hand );
    void close( const Hand & hand );
    void open( const Hand & hand );
    void move( const Hand & hand );
    void drag( const Hand & hand );

    std::map<float, HandCursor> handmap;
};
