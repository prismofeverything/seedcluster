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
    isClosed = false;
    pathIndex = 0;
}

void Hand::sync( const Hand & other )
{
    hue = other.hue;
    isHand = other.isHand;
    isClosed = other.isClosed;
    pathIndex = other.pathIndex;

    path = other.path;
    if ( path.size() < maxHistory ) {
        pathIndex = path.size();
        path.push_back( center );
    } else {
        pathIndex++;
        pathIndex = pathIndex % maxHistory;
        path[pathIndex] = center;
    }
}

cv::Point Hand::motion()
{
    return path[pathIndex] - path[(pathIndex - 1) % maxHistory];
}

void Hand::drawFingertips()
{
    for( vector<Point2i>::iterator it = fingertips.begin(); it != fingertips.end(); it++ ) {
        gl::drawSolidCircle( ci::Vec2f( it->x, it->y ), 10.0f );
        gl::drawLine( ci::Vec2f( center.x, center.y ), ci::Vec2f( it->x, it->y ) );
    }
}

double angleBetween( const cv::Point & center, const cv::Point & a, const cv::Point & b )
{
    double aa = atan2( a.y - center.y, a.x - center.x );
    double bb = atan2( b.y - center.y, b.x - center.x );
    return aa - bb;
}

} // namespace ix

