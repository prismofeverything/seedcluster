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

cv::Point Hand::previousCenter() const 
{
    return path[(pathIndex - 1) % maxHistory];
}

cv::Point Hand::motion() const 
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

void HandTracker::registerListener( HandListener * listener )
{
    listeners.push_back( listener );
}

void HandTracker::detectHands( cv::Mat z )
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

void HandTracker::adaptThreshold( cv::Mat z, int threshold )
{
    // const int channels[] = { 0 };
    // const int histSize[] = { 20 };
    // float range[] = { 0, 255 };
    // const float * ranges[] = { range };
    // cv::calcHist( &z, 1, channels, cv::Mat(), histogram, 1, histSize, ranges, true, false );

    // // double doublemax = 300.0;
    // // cv::threshold( z, z, threshold, threshold, CV_THRESH_TOZERO_INV );
    // // cv::minMaxLoc( z, NULL, &doublemax );
    // // int max = ceil( doublemax );
    // // detectHandsInSlice( z, max - 20, max );
    // // return max;
}

void HandTracker::detectHandsInSlice( cv::Mat z, int zMin, int zMax )
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
        if ( hand->isHand || hand->isEntering() ) {
            hands.push_back( *hand );
            hand->isHand = true;
            if ( hand->isClosing() ) {
                hand->isClosed = true;
            } else if ( hand->isOpening() ) {
                hand->isClosed = false;
            }
        }
    }
}

void HandTracker::notifyListeners()
{
    for ( std::vector<int>::iterator it = bridge.aUnmatched.begin();
          it < bridge.aUnmatched.end(); it++ ) {
        if ( before[ *it ].isHand ) {
            if ( hands.size() == 0 ) {
                for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                      listener != listeners.end(); listener++ ) {
                    (*listener)->handOut( before[ *it ] );
                }
            } else if ( hands.size() == 1 ) {
                for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                      listener != listeners.end(); listener++ ) {
                    (*listener)->secondHandOut( before[ *it ], hands.front() );
                }
            }
        }
    }

    if ( hands.size() == 1 ) {
        Hand & hand = hands.front();

        if ( hand.isEntering() ) {
            hand.isHand = true;
            for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                  listener != listeners.end(); listener++ ) {
                (*listener)->handIn( hand );
            }
        } else if ( hand.isClosed ) {
            if ( hand.isOpening() ) {
                hand.isClosed = false;
                for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                      listener != listeners.end(); listener++ ) {
                    (*listener)->handOpen( hand );
                }
            } else {
                for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                      listener != listeners.end(); listener++ ) {
                    (*listener)->handDrag( hand );
                }
            }
        } else if ( hand.isClosing() ) {
            hand.isClosed = true;
            for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                  listener != listeners.end(); listener++ ) {
                (*listener)->handClose( hand );
            }
        } else {
            for ( std::vector<HandListener *>::iterator listener = listeners.begin(); 
                  listener != listeners.end(); listener++ ) {
                (*listener)->handMove( hand );
            }
        }
    } else if ( hands.size() == 2 ) {
        Hand & first = hands.front();
        Hand & second = hands.back();

        if ( first.isEntering() ) {
            // second hand in
            first.isHand = true;
            for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                  listener != listeners.end(); listener++ ) {
                (*listener)->secondHandIn( first, second );
            }
        } else if ( second.isEntering() ) {
            // second hand in
            second.isHand = true;
            for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                  listener != listeners.end(); listener++ ) {
                (*listener)->secondHandIn( second, first );
            }
        } else if ( first.isClosed ) {
            if ( first.isOpening() ) {
                first.isClosed = false;
                if ( second.isClosed ) {
                    // secondHandOpen
                    for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                          listener != listeners.end(); listener++ ) {
                        (*listener)->secondHandOpen( first, second );
                    }
                } else {
                    // firstHandOpen
                    for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                          listener != listeners.end(); listener++ ) {
                        (*listener)->firstHandOpen( first, second );
                    }
                }
            } else if ( second.isClosed ) {
                if ( second.isOpening() ) {
                    // secondHandOpen
                    second.isClosed = false;
                    for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                          listener != listeners.end(); listener++ ) {
                        (*listener)->secondHandOpen( second, first );
                    }
                } else {
                    // closedHandsMove
                    for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                          listener != listeners.end(); listener++ ) {
                        (*listener)->closedHandsMove( first, second );
                    }
                }
            } else if ( second.isClosing() ) {
                // secondHandClose
                second.isClosed = true;
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->secondHandClose( second, first );
                }
            } else {
                // mixedHandsMove
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->mixedHandsMove( second, first );
                }
            }
        } else if ( second.isClosed ) {
            if ( second.isOpening() ) {
                // firstHandOpen
                second.isClosed = false;
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->firstHandOpen( second, first );
                }
            } else {
                // mixedHandsMove
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->mixedHandsMove( first, second );
                }
            }
        } else if ( first.isClosing() ) {
            first.isClosed = true;
            if ( second.isClosed ) {
                // secondHandClose
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->secondHandClose( first, second );
                }
            } else {
                // firstHandClose
                for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                      listener != listeners.end(); listener++ ) {
                    (*listener)->firstHandClose( first, second );
                }
            }
        } else if ( second.isClosing() ) {
            // firstHandClose
            second.isClosed = true;
            for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                  listener != listeners.end(); listener++ ) {
                (*listener)->firstHandClose( second, first );
            }
        } else {
            // openHandsMove
            for ( std::vector<HandListener *>::iterator listener = listeners.begin();
                  listener != listeners.end(); listener++ ) {
                (*listener)->openHandsMove( first, second );
            }
        }
    }
}

int HandTracker::numberOfHands()
{
    return hands.size();
}

void HandTracker::bridgeFrames()
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

void HandTracker::drawField( int lower, int upper )
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

        // for ( int i = 0; i < edges.size(); i++ ) {
        //     std::vector<cv::Point> approx;
        //     cv::Mat contourMat = cv::Mat( edges[i] );
        //     cv::approxPolyDP( contourMat, approx, 10, true );

        //     if ( approx.size() > 6 ) {
        //         for ( int j = 0; j < approx.size(); j++ ) {
        //             cv::line( depth, approx[j], approx[(j-1) % approx.size()], cv::Scalar( 150.0 ) );
        //         }
        //     }
        // }

        texture = ci::gl::Texture( ci::fromOcv( depth ) );

        ci::gl::draw( texture );
    }
}

} // namespace ix

