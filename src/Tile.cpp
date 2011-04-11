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

Tile::Tile( TileCluster & clust, Vec2i grid, int row, int column, float z, Vec3f col )
    : cluster( clust ),
      corner( grid ),
      rows( row ),
      columns( column ),
      position( Vec3f( grid[0]*atomWidth, grid[1]*atomHeight, z ) ),
      box( Rectf( 0, 0, atomWidth*column, atomHeight*row ) ),
      color( col ),
      velocity( Vec3f( 0.0f, 0.0f, 0.0f ) ),
      alpha( 0.0f )
{
    for ( int l = 0; l < 4; l++ ) {
        liberties[l] = -1;
    }

    state = boost::shared_ptr<TileState>( new EnterTileState() );
}

Tile::Tile( const Tile & rhs ) 
    : cluster( rhs.cluster ),
      corner( rhs.corner ),
      rows( rhs.rows ),
      columns( rhs.columns ),
      position( rhs.position ),
      box( rhs.box ),
      color( rhs.color ),
      velocity( rhs.velocity ),
      alpha( rhs.alpha )
{
    for ( int l = 0; l < 4; l++ ) {
        liberties[l] = rhs.liberties[l];
    }

    state = rhs.state;
}

Tile & Tile::operator=( const Tile & rhs )
{
    corner = rhs.corner;
    rows = rhs.rows;
    columns = rhs.columns;
    position = rhs.position;
    box = rhs.box;
    color = rhs.color;
    velocity = rhs.velocity;
    alpha = rhs.alpha;

    for ( int l = 0; l < 4; l++ ) {
        liberties[l] = rhs.liberties[l];
    }

    state = rhs.state;

    return *this;
}

void Tile::addAlpha( float variance )
{
    alpha += variance;
    if ( alpha < 0.0f ) alpha = 0.0f;
    if ( alpha > 1.0f ) alpha = 1.0f;
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
