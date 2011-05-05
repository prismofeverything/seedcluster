#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <math.h>
// #include "cinder/Cinder.h"
// #include "cinder/Vector.h"
// #include "cinder/Rand.h"
// #include "cinder/gl/Texture.h"
// #include "cinder/gl/gl.h"
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
    cv::Point motion() const;
    cv::Point previousCenter( int offset=1 ) const;
    cv::Point smoothCenter( int reach=3 ) const;
    inline bool isOpen() { return fingertips.size() > 3; };
    inline bool isEntering() { return !isHand && isOpen(); };
    inline bool isOpening() { return isClosed && isOpen(); };
    inline bool isClosing() { return !isClosed && fingertips.size() < 1; };

    cv::Point center;
    bool isHand;
    bool isClosed;

    std::vector<cv::Point> path;
    int pathIndex;
    static const int maxHistory = 20;

    int id;
    double area;
    float radius;
    float hue;
    std::vector<cv::Point> fingertips;
    std::vector<cv::Point> contour;
    std::vector<cv::Point> approx;
};

class HandListener
{
 public:
    // one hand events
    virtual void handIn( const Hand & hand ) {};
    virtual void handOut( const Hand & hand ) {};
    virtual void handMove( const Hand & hand ) {};
    virtual void handClose( const Hand & hand ) {};
    virtual void handOpen( const Hand & hand ) {};
    virtual void handDrag( const Hand & hand ) {};

    // two hand events
    virtual void secondHandIn( const Hand & in, const Hand & other ) {};
    virtual void secondHandOut( const Hand & out, const Hand & other ) {};
    virtual void firstHandClose( const Hand & close, const Hand & other ) {};
    virtual void firstHandOpen( const Hand & open, const Hand & other ) {};
    virtual void secondHandClose( const Hand & close, const Hand & other ) {};
    virtual void secondHandOpen( const Hand & open, const Hand & other ) {};
    virtual void openHandsMove( const Hand & first, const Hand & second ) {};
    virtual void mixedHandsMove( const Hand & open, const Hand & close ) {};
    virtual void closedHandsMove( const Hand & first, const Hand & second ) {};
};

double angleBetween( const cv::Point & center, const cv::Point & a, const cv::Point & b );

struct PointDistance : public std::binary_function<cv::Point, cv::Point, float>
{
    inline float operator() ( const cv::Point & a, const cv::Point & b ) {
        return (float) pow( a.x - b.x, 2 ) + pow( a.y - b.y, 2 );
    }
};

class HandTracker
{
 public:
    HandTracker() {};
    void detectHands( cv::Mat z );
    void adaptThreshold( cv::Mat depth, int threshold );
    void detectHandsInSlice( cv::Mat z, int zMin, int zMax );
    void notifyListeners();
    void bridgeFrames();
    void registerListener( HandListener * listener );
    int numberOfHands();
    cv::Mat displayField( int lower, int upper );

    std::vector<HandListener *> listeners;

    ix::FrameBridge<cv::Point, PointDistance> bridge;
    cv::Mat depth;
    cv::Mat handmask;
    cv::MatND histogram;
    std::vector<Hand> before;
    std::vector<Hand> possibleHands;
    std::vector<Hand> hands;
    std::vector<std::vector<cv::Point> > contours;
    bool blankFrame;
    bool drawable;
};

} // namespace ix

