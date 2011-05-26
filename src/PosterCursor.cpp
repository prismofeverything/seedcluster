#include <vector>
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "PosterCursor.h"

using namespace cinder;

namespace ix {

PosterCursor::PosterCursor()
    : shift( 0, 0 ), 
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
}

void PosterCursor::close( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
}

void PosterCursor::open( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
}

void PosterCursor::move( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
    shift = (center - anchor);
    shift[0] = -shift[0];
    // shift -= shift * 0.001f;
    // anchor = anchor + shift;
}

void PosterCursor::drag( cv::Point _center )
{
    center = ci::Vec2f( _center.x, _center.y );
    shift = (center - anchor);
    shift[0] = -shift[0];
}

void PosterCursor::update()
{

}

void PosterCursor::draw()
{
    float radius = 50.0f;
    float alpha = 0.5f;

    gl::pushModelView();
    gl::enableAlphaBlending();
    glColor4f( 1, 1, 1, alpha );
    gl::drawSolidCircle( center, radius * 0.2 );
    gl::popModelView();
    
    gl::pushModelView();
    Color hsv( CM_HSV, Vec3f( 0.364, 1, 0.6 ) );
    glColor4f( hsv.r, hsv.g, hsv.b, alpha );
    gl::drawSolidCircle( center, radius * 0.9 );
    gl::popModelView();
    
    gl::pushModelView();
    glColor4f( 1, 1, 1, alpha );
    gl::translate( Vec3f( 0, 0, -1 ) );
    gl::drawSolidCircle( center, radius );
    gl::disableAlphaBlending();
    gl::popModelView();
}

} // namespace ix
