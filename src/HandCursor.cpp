#include <vector>
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "HandCursor.h"

namespace ix {

HandCursor::HandCursor()
    : radius( 1.0f ),
      alpha( 0.8f ),
      inertia( 0.6f ),
      fingerStretch( 1.1f ),
      color( 0.364, 1, 0.6 ),
      goingOut( false ),
      complete( false ),
      fullRadius( 30.0f ),
      closeRadius( 20.0f )
{
    radiusEase = Ease( radius, fullRadius, 30 );
}

void HandCursor::in( const Hand & hand, cv::Point _center )
{
    stretchEase = Ease( fingerStretch, -5, 20 );
    center = ci::Vec2f( _center.x, _center.y );
    for ( std::vector<cv::Point>::const_iterator fingertip = hand.fingertips.begin(); fingertip != hand.fingertips.end(); fingertip++ ) {
        fingertips.push_back( ci::Vec2f( _center.x - fingertip->x, _center.y - fingertip->y ) );
    }
}

void HandCursor::out( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );

    radiusEase = Ease( radius, -1.0f, 30 );
    brightnessEase = Ease( color[2], 0.6f, 30 );

    goingOut = true;
}

void HandCursor::close( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
    radiusEase = Ease( radius, closeRadius, 30 );
    brightnessEase = Ease( color[2], 0.9f, 30 );
}

void HandCursor::open( cv::Point _center )
{
    fingerStretch = 0.8f;
    stretchEase = Ease( fingerStretch, -5, 20 );
    center = ci::Vec2f( _center.x, _center.y );
    radiusEase = Ease( radius, fullRadius, 30 );
    brightnessEase = Ease( color[2], 0.6f, 30 );
}

void HandCursor::move( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
}

void HandCursor::drag( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
}

void HandCursor::update()
{
    bool transitionsComplete = true;

    if ( !radiusEase.done() ) {
        transitionsComplete = false;
        radius = radiusEase.out();
    }
    if ( !alphaEase.done() ) {
        transitionsComplete = false;
        alpha = alphaEase.out();
    }
    if ( !brightnessEase.done() ) {
        transitionsComplete = false;
        color[2] = brightnessEase.out();
    }
    if ( !stretchEase.done() ) {
        transitionsComplete = false;
        fingerStretch = stretchEase.in();
    }

    if ( goingOut && transitionsComplete ) {
        complete = true;
    }
}

void HandCursor::drawCircle( ci::Vec2f _center, float _radius, float _alpha )
{
    ci::gl::pushModelView();
    ci::gl::enableAlphaBlending();

    glColor4f( 1, 1, 1, _alpha );
    ci::gl::drawSolidCircle( _center, _radius * 0.2 );

    ci::Color hsv( ci::CM_HSV, color );
    glColor4f( hsv.r, hsv.g, hsv.b, _alpha );
    ci::gl::drawSolidCircle( _center, _radius * 0.9 );

    glColor4f( 1, 1, 1, _alpha );
    ci::gl::translate( ci::Vec3f( 0, 0, -1 ) );
    ci::gl::drawSolidCircle( _center, _radius );

    ci::gl::disableAlphaBlending();
    ci::gl::popModelView();
}

void HandCursor::draw()
{
    drawCircle( center, radius, alpha );
    for ( std::vector<ci::Vec2f>::iterator fingertip = fingertips.begin(); fingertip != fingertips.end(); fingertip++ ) {
        drawCircle( center - ((*fingertip) * fingerStretch), radius*0.3f * fingerStretch, alpha * fingerStretch );
    }
}

HandCursor & HandMap::get( const Hand & hand )
{
    return handmap[ hand.hue ];
}

void HandMap::update( const HandTracker & tracker )
{
    std::vector<float> outs;

    for ( std::map<float, HandCursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        cursor->second.update();
        if ( cursor->second.isOut() ) {
            outs.push_back( cursor->first );
        }
    }

    for ( std::vector<float>::iterator oo = outs.begin(); oo != outs.end(); oo++ ) {
        handmap.erase( *oo );
    }

    flush( tracker.hands );
}

void HandMap::draw()
{
    for ( std::map<float, HandCursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        cursor->second.draw();
    }
}

void HandMap::flush( const std::vector<Hand> & hands )
{
    std::vector<float> outs;
    for ( std::map<float, HandCursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        outs.push_back( cursor->first );
    }

    for ( std::vector<Hand>::const_iterator hand = hands.begin(); hand != hands.end(); hand++ ) {
        std::vector<float>::iterator found = std::find( outs.begin(), outs.end(), hand->hue );
        if ( found != outs.end() ) {
            outs.erase( found );
        }
    }

    for ( std::vector<float>::iterator gone = outs.begin(); gone < outs.end(); gone++ ) {
        if ( !handmap[ *gone ].goingOut ) {
            handmap.erase( *gone );
        }
    }
}

void HandMap::in( const Hand & hand )
{
    HandCursor cursor;
    cursor.in( hand, hand.center );
    handmap[ hand.hue ] = cursor;
}

void HandMap::out( const Hand & hand )
{
    handmap[ hand.hue ].out( hand.smoothCenter( smoothing ) );
    // handmap.erase( hand.hue );
}

void HandMap::close( const Hand & hand )
{
    handmap[ hand.hue ].close( hand.smoothCenter( smoothing ) );
}

void HandMap::open( const Hand & hand )
{
    handmap[ hand.hue ].open( hand.smoothCenter( smoothing ) );
}

void HandMap::move( const Hand & hand )
{
    handmap[ hand.hue ].move( hand.smoothCenter( smoothing ) );
}

void HandMap::drag( const Hand & hand )
{
    handmap[ hand.hue ].drag( hand.smoothCenter( smoothing ) );
}

} // namespace ix
