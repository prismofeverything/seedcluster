#include <iostream>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "CinderOpenCv.h"
#include "Fingertips.h"

using namespace cv;
using namespace std;
using namespace cinder;

namespace ix {

Fingertips::Fingertips()
{
    initialized = false;
}

void Fingertips::unproject( unsigned short* depth, float* x, float* y, float* z ) 
{
    int u,v;
    const float f = 500.0f;
    const float u0 = 320.0f;
    const float v0 = 240.0f;
    float zCurrent;

    // TODO calibration

    for ( int i=0; i<640*480; i++ ) {
        u = i % 640;
        v = i / 640;
        zCurrent = 1.0f / ( -0.00307110156374373f * depth[i] + 3.33094951605675f );
        if ( z != NULL ) {
            z[i] = zCurrent;
        }
        if ( x != NULL ) {
            x[i] = ( u - u0 ) * zCurrent / f;
        }
        if ( y != NULL ) {
            y[i] = ( v - v0 ) * zCurrent / f;
        }
    }
}

std::vector<cv::Point2i> Fingertips::detectFingertips( cv::Mat z, int zMin, int zMax ) 
{ 
    handmask = z < zMax & z > zMin;
    fingertips.clear();
    contours.clear();
    curves.clear();
    hulls.clear();
    field = z < -10000;

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

                // find upper and lower bounds of the hand and define cutoff threshold (don't consider lower vertices as fingers)
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

                    // low interior angle + within upper 90% of region -> we got a finger
                    if ( angle < 1 && approxCurve[idx].y < cutoff ) {
                        int u = approxCurve[idx].x;
                        int v = approxCurve[idx].y;

                        fingertips.push_back( Point2i( u,v ) );
                    }
                }
            }
        }
    }

    initialized = true;
    return fingertips;
}

void Fingertips::drawContours()
{
    if ( initialized ) {
        for ( int i=0; i<curves.size(); i++ ) {
            Scalar center = mean( Mat( contours[i] ) );
            Point centerPoint = Point( center.val[0], center.val[1] );
            vector<Point> contour = contours[i];
            vector<Point> curve = curves[i];
            vector<int> hull = hulls[i];

            // draw cutoff threshold
            cv::line( field, Point( center.val[0]-100, cutoff ), Point( center.val[0]+100, cutoff ), Scalar( 254.0f ) );

            int last = contour.size();
            for ( int j=0; j<last; j++ ) {
                cv::circle( field, contour[j], 10, Scalar( 254.0f ) );
                cv::line( field, contour[j], contour[(j-1)%last], Scalar( 254.0f ) );

                ci::Vec2f jnow( contour[j].x, contour[j].y );
                ci::Vec2f jprev( contour[(j-1) % last].x, contour[(j-1) % last].y );

                gl::drawSolidCircle( jnow, 10.0f );
                gl::drawLine( jnow, jprev );
            }

            last = curve.size();
            for ( int j=0; j<last; j++ ) {
                cv::circle( field, curve[j], 10, Scalar( 254.0f ) );
                cv::line( field, curve[j], curve[j-1], Scalar( 254.0f ) );

                ci::Vec2f jnow( curve[j].x, curve[j].y );
                ci::Vec2f jprev( curve[(j-1) % last].x, curve[(j-1) % last].y );

                gl::drawSolidCircle( jnow, 10.0f );
                gl::drawLine( jnow, jprev );
            }

            // int last = hull.size();
            // for ( int j=0; j<last; j++ ) {
            //     // cv::circle( field, hull[j], 10, Scalar( 254.0f ) );
            //     ci::Vec2f jnow( curve[hull[j]].x, curve[hull[j]].y );
            //     ci::Vec2f jprev( curve[hull[(j-1) % last]].x, curve[hull[(j-1) % last]].y );

            //     gl::drawSolidCircle( jnow, 10.0f );
            //     gl::drawLine( jnow, jprev );
            //     // cv::line( field, hull[j], hull[j-1], Scalar( 254.0f ) );
            // }

            // draw curve hull
            // for ( int j=0; j<hull.size(); j++ ) {
            //     cv::circle( field, curve[hull[j]], 10, Scalar( 254.0f ), 3 );
            //     if( j == 0 ) {
            //         cv::line( field, curve[hull[j]], curve[hull[hull.size()-1]], Scalar( 254.0f ) );
            //     } else {
            //         cv::line( field, curve[hull[j]], curve[hull[j-1]], Scalar( 254.0f ) );
            //     }
            // }
        }
    }
}

void Fingertips::drawFingertips()
{
    for( vector<Point2i>::iterator it = fingertips.begin(); it != fingertips.end(); it++ ) {
        gl::drawSolidCircle( ci::Vec2f( it->x, it->y ), 10.0f );
        cv::circle( field, (*it), 10, Scalar( 254.0f ), -1 );
        fieldMean = mean( field );
    }                   
}

void Fingertips::drawField()
{
    gl::Texture fieldTexture( fromOcv( field ) );
    gl::draw( fieldTexture );
}


} // namespace ix

