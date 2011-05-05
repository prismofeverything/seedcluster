HandCursor::HandCursor()
{

}

void HandCursor::update()
{
    if ( !radiusEase.done() ) {
        radius = radiusEase.out();
    }
    if ( !radiusEase.done() ) {
        radius = radiusEase.out();
    }
    if ( !radiusEase.done() ) {
        radius = radiusEase.out();
    }

}

void HandCursor::draw()
{
    gl::pushModelView();
    gl::enableAlphaBlending();
    glColor4f( 1, 1, 1, alpha );
    gl::drawSolidCircle( center, radius * 0.2 );
    setColor( Vec3f( 0.364, 1, 0.6 ), alpha );
    gl::drawSolidCircle( center, radius * 0.9 );
    glColor4f( 1, 1, 1, alpha );
    gl::translate( Vec3f( 0, 0, -1 ) );
    gl::drawSolidCircle( center, radius );
    gl::disableAlphaBlending();
    gl::popModelView();
}

