#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "Tile.h"
#include "TileCluster.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

namespace ix
{

Tile::Tile( TileCluster * clust, int index, Vec2i grid, Vec2i dim, float z, Vec3f col )
    : cluster( clust ),
      id( index ),
      dimension( dim ),
      topLeft( grid ),
      bottomRight( grid + dim ), 
      position( Vec3f( grid[0]*atomWidth, grid[1]*atomHeight, z ) ),
      box( Rectf( 0, 0, atomWidth*dim[0], atomHeight*dim[1] ) ),
      color( col ),
      velocity( Vec3f( 0.0f, 0.0f, 0.0f ) ),
      alpha( 0.0f ),
      alphaEase( 0.0f, 0.9f, 40 ),
      state( Entering )
{
    for ( int l = 0; l < 4; l++ ) {
        liberties[l] = -1;
    }
}

Vec2i Tile::relativeCorner( Vec2i dim, Vec2i orientation )
{
    Vec2i grid;
    for ( int i = 0; i < 2; i++ ) {
        if ( orientation[i] == 0 ) {
            int axisSpread = dim[i] - 1;
            int offset = Rand::randInt( axisSpread + dimension[i] ) - axisSpread;
            grid[i] = offset + topLeft[i];
        } else if ( orientation[i] < 0 ) {
            grid[i] = topLeft[i] - dim[i];
        } else {
            grid[i] = bottomRight[i];
        }
    }

    return grid;
}

bool Tile::collidesWith( ci::Vec2i tl, ci::Vec2i br )
{
    return !( br[0] <= topLeft[0] || 
              br[1] <= topLeft[1] || 
              tl[0] >= bottomRight[0] || 
              tl[1] >= bottomRight[1] );
}

void Tile::update()
{
    bool full = false;
    switch( state ) {
    case Entering:
        if ( !alphaEase.done() ) {
            alpha = alphaEase.out();
        } else {
            state = Blooming;
        }
        break;
    case Blooming:
        // if ( Rand::randFloat() < 0.02 ) {
        //     // full = branch();
        // }

        if ( full ) {
            state = Leaving;
            alphaEase = Ease( alpha, 0.0f, 40 );
        }
        break;
    case Leaving:
        if ( !alphaEase.done() ) {
            alpha = alphaEase.out();
        }
        break;
    }

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
