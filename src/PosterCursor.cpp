#include <vector>
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "PosterCursor.h"

using namespace cinder;

namespace ix {

PosterCursor::PosterCursor()
    : shift( 0, 0 ), 
      dshift( 0, 0 ), 
      complete( false ),
      goingOut( false )
{

}

void PosterCursor::in( const Hand & hand, cv::Point _center )
{
    anchor = ci::Vec2f( _center.x, _center.y );
    center = anchor;
}

void PosterCursor::out( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
    dshift = ci::Vec2f( 0, 0 );
}

void PosterCursor::close( cv::Point _center )
{
    move( _center );
    // center = ci::Vec2f( _center.x, _center.y );
}

void PosterCursor::open( cv::Point _center )
{
    move( _center );
    // center = ci::Vec2f( _center.x, _center.y );
}

void PosterCursor::move( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
    ci::Vec2f dcenter = center - anchor;

    if ( dcenter.length() > 30 && dcenter.length() < 500 ) {
        ci::Vec2f shifting = (center - anchor) * 0.034;
        shifting[1] = -shifting[1];
        shift += shifting;
    }

    shift -= shift * 0.20;
    // shift += dshift;
    // dshift -= dshift * 0.1;

    // shift += shifting;

    // shift = (center - anchor);
    // shift[1] = -shift[1];
}

void PosterCursor::drag( cv::Point _center )
{
    move( _center );
}

void PosterCursor::update()
{

}

void PosterCursor::draw()
{
    float radius = 30.0f;
    float alpha = 0.6f;

    ci::Vec2f reflected( center );
    reflected[0] = 640 - reflected[0];

    gl::pushModelView();
    gl::enableAlphaBlending();
    glColor4f( 1, 1, 1, alpha );
    gl::drawSolidCircle( reflected, radius * 0.2 );
    gl::popModelView();
    
    gl::pushModelView();
    Color hsv( CM_HSV, Vec3f( 0.364, 1, 0.6 ) );
    glColor4f( hsv.r, hsv.g, hsv.b, alpha );
    gl::drawSolidCircle( reflected, radius * 0.9 );
    gl::popModelView();
    
    gl::pushModelView();
    glColor4f( 1, 1, 1, alpha );
    gl::translate( Vec3f( 0, 0, -1 ) );
    gl::drawSolidCircle( reflected, radius );
    gl::disableAlphaBlending();
    gl::popModelView();
}

} // namespace ix
