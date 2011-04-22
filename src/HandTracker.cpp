#include <algorithm>
#include <functional>
#include "cminpack.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCv.h"
#include "HandTracker.h"

using namespace cv;
using namespace std;
using namespace cinder;

#define TAU 6.2831853071795862f

namespace ix {

Hand::Hand()
{
    hue = Rand::randFloat();
    isHand = false;
}

void Hand::sync( const Hand & other )
{
    hue = other.hue;
    isHand = other.isHand;
}

void Hand::drawFingertips()
{
    for( vector<Point2i>::iterator it = fingertips.begin(); it != fingertips.end(); it++ ) {
        gl::drawSolidCircle( ci::Vec2f( it->x, it->y ), 10.0f );
        gl::drawLine( ci::Vec2f( center.x, center.y ), ci::Vec2f( it->x, it->y ) );
    }
}

} // namespace ix

