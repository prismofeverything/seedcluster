#include <iostream>
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
    cv::Point motion();

    cv::Point center;
    bool isHand;
    bool isClosed;

    std::vector<cv::Point> path;
    int pathIndex;
    static const int maxHistory = 12;

    double area;
    float radius;
    float hue;
    std::vector<cv::Point> fingertips;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> approx;
};

double angleBetween( const cv::Point & center, const cv::Point & a, const cv::Point & b );

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
    HandTracker() { listener = NULL; };
    void detectHands( cv::Mat z );
    int adaptThreshold( cv::Mat depth, int threshold );
    void detectHandsInSlice( cv::Mat z, int zMin, int zMax );
    void notifyListeners();
    void bridgeFrames();
    void drawField( int lower, int upper );
    void registerListener( Listener * _listener );
    int numberOfHands();

    Listener * listener;

    ix::FrameBridge<cv::Point, PointDistance> bridge;
    cv::Mat depth;
    cv::Mat handmask;
    std::vector<Hand> before;
    std::vector<Hand> possibleHands;
    std::vector<Hand> hands;
    std::vector<std::vector<cv::Point> > contours;
    ci::gl::Texture texture;
    bool blankFrame;
    bool drawable;
};

template <class Listener>
void HandTracker<Listener>::registerListener( Listener * _listener )
{
    listener = _listener;
    blankFrame = false;
}

template <class Listener>
void HandTracker<Listener>::detectHands( cv::Mat z )
{
    // cv::Mat depth = z.clone();
    // cv::erode( depth, depth, cv::Mat() );
    // std::vector<Hand> previousPossible = possibleHands;
    // std::vector<Hand> previousHands = hands;
    // int max = adaptThreshold( depth, 250 );

    // if ( !blankFrame && hands.size() == 0 && previousHands.size() > 0 ) {
    //     hands = previousHands;
    //     possibleHands = previousPossible;
    //     before = previousPossible;
    //     blankFrame = true;
    // } else {
    //     blankFrame = false;
    // }

    depth = z.clone();
    adaptThreshold( z, 0 );
    detectHandsInSlice( z, 150, 255 );

    notifyListeners();
}

template <class Listener>
int HandTracker<Listener>::adaptThreshold( cv::Mat z, int threshold )
{
    const int channels[] = { 0 };
    const int histSize[] = { 20 };
    float range[] = { 0, 255 };
    const float * ranges[] = { range };
    cv::MatND hist;
    cv::calcHist( &z, 1, channels, cv::Mat(), hist, 1, histSize, ranges, true, false );
    return 0;

    // double doublemax = 300.0;
    // cv::threshold( z, z, threshold, threshold, CV_THRESH_TOZERO_INV );
    // cv::minMaxLoc( z, NULL, &doublemax );
    // int max = ceil( doublemax );
    // detectHandsInSlice( z, max - 20, max );
    // return max;
}

template <class Listener>
void HandTracker<Listener>::detectHandsInSlice( cv::Mat z, int zMin, int zMax )
{ 
    handmask = z < zMax & z > zMin;
    cv::morphologyEx( handmask, handmask, cv::MORPH_CLOSE, cv::Mat(), cv::Point( -1, -1 ), 2 );
    // cv::dilate( handmask, handmask, cv::Mat(), cv::Point( -1, -1 ), 3 );
    before = possibleHands;
    possibleHands.clear();

    findContours( handmask, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );

    for ( int i = 0; i < contours.size(); i++ ) {
        Hand hand;
        hand.contour = contours[i];

        cv::Mat contourMat = cv::Mat( hand.contour );
        cv::Scalar center = mean( contourMat );
        hand.center = cv::Point( center.val[0], center.val[1] );
        hand.area = cv::contourArea( contourMat );

        if ( hand.area > 1000 ) { // possible hand
            cv::approxPolyDP( contourMat, hand.approx, 20, true );

            for ( int hx = 0; hx < hand.approx.size(); hx++ ) {
                int px = hx == 0 ? hand.approx.size() - 1 : hx - 1;
                int sx = hx == hand.approx.size() - 1 ? 0 : hx + 1;
                double angle = angleBetween( hand.approx[hx], hand.approx[sx], hand.approx[px] );

                if ( angle > 0.0 && angle < 1.0 ) { 
                    hand.fingertips.push_back( hand.approx[hx] );
                }
            }

            possibleHands.push_back( hand );
        }
    }

    bridgeFrames();

    // collect the real hands
    hands.clear();
    for ( std::vector<Hand>::iterator hand = possibleHands.begin(); hand < possibleHands.end(); hand++ ) {
        if ( hand->isHand || hand->fingertips.size() > 3 ) {
            hands.push_back( *hand );
        }
    }
}

template <class Listener>
void HandTracker<Listener>::notifyListeners()
{
    // notify the listeners that hands have departed or arrived, now that we have a count
    for ( std::vector<int>::iterator it = bridge.aUnmatched.begin();
          it < bridge.aUnmatched.end(); it++ ) {
        if ( before[ *it ].isHand ) {
            listener->handOut( before[ *it ] );
        }
    }

    for ( std::vector<Hand>::iterator hand = possibleHands.begin(); hand < possibleHands.end(); hand++ ) {
        if ( !hand->isHand && hand->fingertips.size() > 3 ) {
            hand->isHand = true;
            listener->handIn( *hand );
        } else if ( hand->isHand ) {
            if ( hand->isClosed ) {
                if ( hand->fingertips.size() > 2 ) {
                    hand->isClosed = false;
                    listener->handOpen( *hand );
                } else {
                    listener->handDrag( *hand );
                }
            } else {
                if ( !hand->isClosed && hand->fingertips.size() < 1 ) {
                    hand->isClosed = true;
                    listener->handClose( *hand );
                } else {
                    listener->handMove( *hand );
                }
            }
        }
    }
}

template <class Listener>
int HandTracker<Listener>::numberOfHands()
{
    return hands.size();
}

template <class Listener>
void HandTracker<Listener>::bridgeFrames()
{
    std::vector<cv::Point> preframe, postframe;
    for ( std::vector<Hand>::iterator it = before.begin(); it < before.end(); it++ ) {
        preframe.push_back( it->center );
    }
    for ( std::vector<Hand>::iterator it = possibleHands.begin(); it < possibleHands.end(); it++ ) {
        postframe.push_back( it->center );
    }

    std::vector<FrameLink> links = bridge.bridge( preframe, postframe, PointDistance() );
    for ( std::vector<FrameLink>::iterator it = links.begin(); it < links.end(); it++ ) {
        possibleHands[ it->b ].sync( before[ it->a ] );
    }
}

template <class Listener>
void HandTracker<Listener>::drawField( int lower, int upper )
{
    if ( !depth.rows == 0 ) {
        // cv::Mat amp = handmask * 254.0f;
        // texture = ci::gl::Texture( ci::fromOcv( amp ) );
        // texture = ci::gl::Texture( ci::fromOcv( handmask ) );

        cv::Mat canny = depth.clone();
        std::vector<std::vector<cv::Point> > edges;
        cv::Canny( depth, canny, 10, 20 );
        cv::findContours( canny, edges, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
        cv::drawContours( depth, edges, -1, cv::Scalar( 250.0 ) );

        for ( int i = 0; i < edges.size(); i++ ) {
            std::vector<cv::Point> approx;
            cv::Mat contourMat = cv::Mat( edges[i] );
            cv::approxPolyDP( contourMat, approx, 10, true );

            if ( approx.size() > 6 ) {
                for ( int j = 0; j < approx.size(); j++ ) {
                    cv::line( depth, approx[j], approx[(j-1) % approx.size()], cv::Scalar( 150.0 ) );
                }
            }
        }

        texture = ci::gl::Texture( ci::fromOcv( depth ) );

        ci::gl::draw( texture );
    }
}

} // namespace ix

