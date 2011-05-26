#pragma once
#include <map>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "HandTracker.h"
#include "Ease.h"

namespace ix {

class PosterCursor {
 public:
    PosterCursor();

    void in( const Hand & hand, cv::Point _center );
    void out( cv::Point _center );
    void close( cv::Point _center );
    void open( cv::Point _center );
    void move( cv::Point _center );
    void drag( cv::Point _center );
    
    inline bool isOut() { return complete; };

    void update();
    void draw();

    bool complete;
    bool goingOut;

    ci::Vec2f anchor;
    ci::Vec2f center;
    ci::Vec2f shift;
};

} // namespace ix
