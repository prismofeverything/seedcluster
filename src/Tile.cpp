#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "TileState.h"
#include "Tile.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

Tile::Tile()
{
    
}

Tile::Tile( Vec2i grid, int row, int column, float z, Vec3f col )
{
    corner = grid;
    rows = row;
    columns = column;
    box = Rectf( 0, 0, atomWidth*columns, atomHeight*rows );
    position = Vec3f( grid[0]*atomWidth, grid[1]*atomHeight, z );
    color = col;
    velocity = Vec3f( 0.0f, 0.0f, 0.0f );
    alpha = 0.0f;
    for ( int l = 0; l < 4; l++ ) {
        liberties[l] = -1;
    }

    state = boost::shared_ptr<TileState>(new EnterTileState());
}

void Tile::addAlpha( float variance )
{
    alpha += variance;
}

void Tile::update()
{
    state = state->update( *this );
    position += velocity;
}

void Tile::draw()
{
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r, colorcolor.g, colorcolor.b, alpha );
    gl::pushModelView();
    gl::translate( position );
    gl::drawSolidRect( box );
    gl::popModelView();
}
