#pragma once
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "Ease.h"
#include <vector>

class Seed {
 public:
    Seed( ci::Vec2i _center,
          ci::Vec3f _color );

    void update();
    void draw();

    float radius;
    float z;
    ci::Vec2i center;
    ci::Vec3f color;
    float alpha;
    Ease radiusEase;
};
