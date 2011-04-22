#include <vector>
#include <algorithm>
#include <functional>
#include <math.h>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "FrameBridge.h"

namespace ix {

class Fingertip
{
 public:
    Fingertip();
    ~Fingertip() {};

    cv::Point center;
};

class Hand
{
 public:
    Hand();
    ~Hand() {};
    void sync( const Hand & other );
    void drawFingertips();

    cv::Point center;
    bool isHand;
    double area;
    float radius;
    float hue;
    std::vector<cv::Point> fingertips;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> approx;
    std::vector<int> hull;
};

struct PointDistance : public std::binary_function<cv::Point, cv::Point, float>
{
    inline float operator() ( const cv::Point & a, const cv::Point & b ) {
        return (float) pow( a.x - b.x, 2 ) + pow( a.y - b.y, 2 );
    }
};

template <class Listener>
class HandTracker
{
 public:
    HandTracker() : handsCount(0) { listener = NULL; };
    void detectHands( cv::Mat z, int zMin=100, int zMax=255 );
    void bridgeFrames();
    void drawField();
    void registerListener( Listener * _listener );
    int numberOfHands();

    Listener * listener;

    int handsCount;
    ix::FrameBridge<cv::Point, PointDistance> bridge;
    cv::Mat field;
    cv::Mat handmask;
    std::vector<Hand> before;
    std::vector<Hand> hands;
    std::vector<std::vector<cv::Point> > contours;
    ci::gl::Texture texture;
};

template <class Listener>
void HandTracker<Listener>::registerListener( Listener * _listener )
{
    listener = _listener;
}

template <class Listener>
void HandTracker<Listener>::detectHands( cv::Mat z, int zMin, int zMax ) 
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

            cv::Mat contourMat = cv::Mat( hand.contour );
            cv::Scalar center = mean( contourMat );
            hand.center = cv::Point( center.val[0], center.val[1] );
            hand.area = cv::contourArea( contourMat );

            if ( hand.area > 3000 )  { // possible hand
                cv::approxPolyDP( contourMat, hand.approx, 20, true );
                cv::convexHull( cv::Mat( hand.approx ), hand.hull );

                for ( int j=0; j<hand.hull.size(); j++ ) {
                    int idx = hand.hull[j]; 
                    int pdx = idx == 0 ? hand.approx.size() - 1 : idx - 1;
                    int sdx = idx == hand.approx.size() - 1 ? 0 : idx + 1;

                    cv::Point v1 = hand.approx[sdx] - hand.approx[idx];
                    cv::Point v2 = hand.approx[pdx] - hand.approx[idx];

                    float angle = acos( ( v1.x*v2.x + v1.y*v2.y ) / ( norm( v1 ) * norm( v2 ) ) );

                    if ( angle < 1 ) { 
                        hand.fingertips.push_back( hand.approx[idx] );
                    }
                }

                hands.push_back( hand );
            }
        }
    }

    bridgeFrames();

    // count hands first because the listener will need to know 
    // how many total there are.
    handsCount = 0;
    for ( std::vector<Hand>::iterator hand = hands.begin(); hand < hands.end(); hand++ ) {
        if ( hand->isHand || ( !hand->isHand && hand->fingertips.size() > 4 ) ) {
            handsCount++;
        }
    }

    // now notify the listeners that hands have departed or arrived.
    for ( std::vector<int>::iterator it = bridge.aUnmatched.begin();
          it < bridge.aUnmatched.end(); it++ ) {
        if ( before[ *it ].isHand ) {
            listener->handOut( before[ *it ] );
        }
    }

    for ( std::vector<Hand>::iterator hand = hands.begin(); hand < hands.end(); hand++ ) {
        if ( !hand->isHand && hand->fingertips.size() > 4 ) {
            hand->isHand = true;
            listener->handIn( *hand );
        }
    }
}

template <class Listener>
int HandTracker<Listener>::numberOfHands()
{
    return handsCount;
}

template <class Listener>
void HandTracker<Listener>::bridgeFrames()
{
    std::vector<cv::Point> preframe, postframe;
    for ( std::vector<Hand>::iterator it = before.begin(); it < before.end(); it++ ) {
        preframe.push_back( it->center );
    }
    for ( std::vector<Hand>::iterator it = hands.begin(); it < hands.end(); it++ ) {
        postframe.push_back( it->center );
    }

    std::vector<FrameLink> links = bridge.bridge( preframe, postframe, PointDistance() );
    for ( std::vector<FrameLink>::iterator it = links.begin(); it < links.end(); it++ ) {
        hands[ it->b ].sync( before[ it->a ] );
    }
}

template <class Listener>
void HandTracker<Listener>::drawField()
{
    cv::Mat amp = handmask * 254.0f;
    texture = ci::gl::Texture( ci::fromOcv( amp ) );
    ci::gl::draw( texture );
}

} // namespace ix
