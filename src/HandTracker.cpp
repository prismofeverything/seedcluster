#include <iostream>
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
}

void Hand::drawFingertips()
{
    for( vector<Point2i>::iterator it = fingertips.begin(); it != fingertips.end(); it++ ) {
        gl::drawSolidCircle( ci::Vec2f( it->x, it->y ), 10.0f );
        gl::drawLine( ci::Vec2f( center.x, center.y ), ci::Vec2f( it->x, it->y ) );
    }
}

HandTracker::HandTracker()
{
    initialized = false;
}

std::vector<Hand> HandTracker::detectHands( cv::Mat z, int zMin, int zMax ) 
{ 
    handmask = z < zMax & z > zMin;
    field = z < -1;
    before = hands;
    hands.clear();

    findContours( handmask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE ); 
    if ( contours.size() ) {
        for ( int i=0; i<contours.size(); i++ ) {
            Hand hand;
            hand.contour = contours[i];

            Mat contourMat = Mat( hand.contour );
            Scalar center = mean( contourMat );
            hand.center = Point( center.val[0], center.val[1] );
            hand.area = cv::contourArea( contourMat );

            if ( hand.area > 3000 )  { // possible hand
                cv::approxPolyDP( contourMat, hand.approx, 20, true );
                cv::convexHull( Mat( hand.approx ), hand.hull );

                for ( int j=0; j<hand.hull.size(); j++ ) {
                    int idx = hand.hull[j]; 
                    int pdx = idx == 0 ? hand.approx.size() - 1 : idx - 1;
                    int sdx = idx == hand.approx.size() - 1 ? 0 : idx + 1;

                    Point v1 = hand.approx[sdx] - hand.approx[idx];
                    Point v2 = hand.approx[pdx] - hand.approx[idx];

                    float angle = acos( ( v1.x*v2.x + v1.y*v2.y ) / ( norm( v1 ) * norm( v2 ) ) );

                    if ( angle < 1 ) { 
                        hand.fingertips.push_back( hand.approx[idx] );
                    }
                }

                hands.push_back( hand );
            }
        }
    }

    initialized = true;
    return hands;
}

void HandTracker::drawField()
{
    cv::Mat amp = handmask * 254.0f;
    texture = gl::Texture( fromOcv( amp ) );
    gl::draw( texture );
}


} // namespace ix

