#include "ImageSequenceCursor.h"

using namespace std;

namespace ix 
{
    
ImageSequenceCursor::ImageSequenceCursor() : shift( 0, 0 ), dshift( 0, 0 ), complete( false ), goingOut( false )
{
    sequenceScale = 0.0f;
    anchor = ci::Vec2f( 320, 240 );
}
    
ci::Vec2f ImageSequenceCursor::augmentCenter( cv::Point _center )
{
    return ci::Vec2f( _center.x, _center.y );
    // ci::Vec2f core( 320, 240 );
    // return ( (ci::Vec2f( _center.x, _center.y ) - core ) * 3.5 + core );
}

void ImageSequenceCursor::in( const Hand & hand, cv::Point _center )
{
    // anchor = ci::Vec2f( _center.x, _center.y );
    // anchor = augmentCenter( _center );
    // center = anchor;
    center = augmentCenter( _center );
}
    
void ImageSequenceCursor::out( cv::Point _center )
{
    // center = ci::Vec2f( _center.x, _center.y );
    center = augmentCenter( _center );
    dshift = ci::Vec2f( 0, 0 );
}
    
void ImageSequenceCursor::close( cv::Point _center )
{
    move( _center );
    // center = ci::Vec2f( _center.x, _center.y );
}
    
void ImageSequenceCursor::open( cv::Point _center )
{
    move( _center );
    // center = ci::Vec2f( _center.x, _center.y );
}
    
void ImageSequenceCursor::move( cv::Point _center )
{
    // center = ci::Vec2f( _center.x, _center.y );
    center = augmentCenter( _center );
    ci::Vec2f dcenter = center - anchor;
        
    if ( dcenter.length() > 250 && dcenter.length() < 1100 ) 
    {
        ci::Vec2f shifting = (center - anchor) * shiftFactor;
        shifting[1] = -shifting[1];
        shift += shifting;
    }
        
    shift -= shift * 0.20;
}
    
void ImageSequenceCursor::drag( cv::Point _center )
{
    move( _center );
}
    
void ImageSequenceCursor::update()
{
    sequence.update();
    currentTexture = sequence.getCurrentTexture();
    if( currentTexture )
    {
        ci::Area area = currentTexture.getCleanBounds();
        sequenceOffset = Vec2i( area.getWidth() * 0.5f, area.getHeight() * 0.5f );
    }
    
    previousReflected = reflected;
    reflected.set( center );
    reflected[0] = 640 - reflected[0];
    
    // -- subtract from one to get the remainder of the distance / 100 ( max distance ) * .2 ( max size )
    float targetScale = ( 1.0f - ( reflected.distance( previousReflected ) / 10.0f ) ) * 0.15f;
    
    // -- tweens scale value
    sequenceScale += ( targetScale - sequenceScale ) * 0.1f;
    sequenceScale = min( 0.15f, max( sequenceScale, 0.0f ) );
}
    
void ImageSequenceCursor::draw()
{
    if( currentTexture )
    {
        gl::pushMatrices();
        glColor4f( 1, 1, 1, 1.0f - sequenceScale );
        // glBlendFunc( GL_ONE, GL_ONE );
        gl::enableAlphaBlending();
        gl::translate( reflected );
        // gl::scale( Vec3f( sequenceScale, sequenceScale, 1.0f ) );
        gl::scale( Vec3f( 0.15f, 0.15f, 1.0f ) );
        gl::translate( -sequenceOffset );
        gl::draw( currentTexture );
        gl::disableAlphaBlending();
        gl::popMatrices();
    }
}
    
} // namespace ix
