#pragma once
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "Ease.h"
#include <vector>

namespace ix {

class Seed {
 public:
    Seed( ci::Vec2f _center,
          ci::Vec3f _color );

    void update();
    void draw();

    void hover();
    void unhover();
    void choose();
    void release();

    void seek( ci::Vec2f towards );
    void zoom( float factor );

    float radius;
    float baseRadius;
    float hoverRadius;
    float z;

    ci::Vec2f center;
    ci::Vec3f color;
    ci::Vec3f originalColor;

    float alpha;
    float zoomFactor;
    Ease radiusEase;
    Ease brightnessEase;
    Ease xEase;
    Ease yEase;
};

struct SeedContains
{
    SeedContains( ci::Vec2i _point ) : point( _point ) {};
    inline bool operator() ( const Seed & seed ) {
        return point.distance( seed.center ) < seed.radius;
    };

    ci::Vec2i point;
};

};
