#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "TileState.h"
#include "Tile.h"
#include "TileCluster.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

namespace ix
{

Tile::Tile( TileCluster * clust, int index, Vec2i grid, int row, int column, float z, Vec3f col )
    : cluster( clust ),
      id( index ),
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
}

bool Tile::branch()
{
    int l = Rand::randInt( 4 );

    if ( liberties[l] < 0 ) {
        Vec2i grid;
        Vec3f newColor = color;
        newColor[2] = Rand::randFloat();
        int row = Rand::randInt( 5 ) + 1;
        int column = Rand::randInt( 5 ) + 1;

        switch( l )
            {
            case 0:
                grid[0] = corner[0] + columns;
                grid[1] = Rand::randInt( 5 ) - 2 + corner[1];
                break;
            case 1:
                grid[0] = Rand::randInt( 5 ) - 2 + corner[0];
                grid[1] = corner[1] - row;
                break;
            case 2:
                grid[0] = corner[0] - column;
                grid[1] = Rand::randInt( 5 ) - 2 + corner[1];
                break;
            case 3:
                grid[0] = Rand::randInt( 5 ) - 2 + corner[0];
                grid[1] = corner[1] + rows;
                break;
            }

        liberties[l] = cluster->tiles.size();
        cluster->addTile( grid, row, column, Rand::randFloat() * 20 - 10, newColor, id, (2 + l) % 4 );
    }

    bool full = true;
    for ( l = 0 ; l < 4 ; l++ ) {
        if ( liberties[l] < 0 ) {
            full = false;
        }
    }
    return full;
}

void Tile::update()
{
    state.updateTile( *this );
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

};
