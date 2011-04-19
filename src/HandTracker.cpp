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

std::vector<cv::Point2i> HandTracker::detectFingertips( cv::Mat z, int zMin, int zMax )
{ 
    handmask = z < zMax & z > zMin;
    fingertips.clear();
    contours.clear();
    curves.clear();
    hulls.clear();
    field = z < -1;

    findContours( handmask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE ); 
    if ( contours.size() ) {
        for ( int i=0; i<contours.size(); i++ ) {
            vector<Point> contour = contours[i];
            Mat contourMat = Mat( contour );
            double area = cv::contourArea( contourMat );

            if ( area > 1000 )  { // possible hand
                std::vector<Point> approxCurve;
                std::vector<int> hull;
                cv::approxPolyDP( contourMat, approxCurve, 20, true );
                cv::convexHull( Mat( approxCurve ), hull );
                curves.push_back( approxCurve );
                hulls.push_back( hull );

                int upper = 640, lower = 0;
                for ( int j=0; j<hull.size(); j++ ) {
                    int idx = hull[j]; // corner index
                    if ( approxCurve[idx].y < upper ) upper = approxCurve[idx].y;
                    if ( approxCurve[idx].y > lower ) lower = approxCurve[idx].y;
                }

                cutoff = lower - ( lower - upper ) * 0.1f;

                // find interior angles of hull corners
                for ( int j=0; j<hull.size(); j++ ) {
                    int idx = hull[j]; // corner index
                    int pdx = idx == 0 ? approxCurve.size() - 1 : idx - 1; //  predecessor of idx
                    int sdx = idx == approxCurve.size() - 1 ? 0 : idx + 1; // successor of idx

                    Point v1 = approxCurve[sdx] - approxCurve[idx];
                    Point v2 = approxCurve[pdx] - approxCurve[idx];

                    float angle = acos( ( v1.x*v2.x + v1.y*v2.y ) / ( norm( v1 ) * norm( v2 ) ) );

                    if ( angle < 1 && approxCurve[idx].y < cutoff ) {
                        fingertips.push_back( approxCurve[idx] );
                    }
                }
            }
        }
    }

    initialized = true;
    return fingertips;
}

std::vector<Hand> HandTracker::detectHands( cv::Mat z, int zMin, int zMax ) 
{ 
    handmask = z < zMax & z > zMin;
    field = z < -1;
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

                // find interior angles of hull corners
                for ( int j=0; j<hand.hull.size(); j++ ) {
                    int idx = hand.hull[j]; // corner index
                    int pdx = idx == 0 ? hand.approx.size() - 1 : idx - 1; // predecessor of idx
                    int sdx = idx == hand.approx.size() - 1 ? 0 : idx + 1; // successor of idx

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

void HandTracker::drawFingertips()
{
    for( vector<Point2i>::iterator it = fingertips.begin(); it != fingertips.end(); it++ ) {
        gl::drawSolidCircle( ci::Vec2f( it->x, it->y ), 10.0f );
    }
}

void HandTracker::drawField()
{
    cv::Mat amp = handmask * 254.0f;
    texture = gl::Texture( fromOcv( amp ) );
    gl::draw( texture );
}


} // namespace ix

