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

} // namespace ix
