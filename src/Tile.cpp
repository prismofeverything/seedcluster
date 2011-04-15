#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "TileState.h"
#include "Tile.h"
#include "TileCluster.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f
static const bool PREMULT = false;


Tile::Tile( TileCluster & clust, int index, Vec2i grid, int row, int column, float z, Vec3f col )
    : cluster( clust ),
      id( index ),
      corner( grid ),
      rows( row ),
      columns( column ),
      position( Vec3f( grid[0]*atomWidth, grid[1]*atomHeight, z ) ),
      box( Rectf( 0, 0, atomWidth*column, atomHeight*row ) ),
      color( col ),
      velocity( Vec3f( 0.0f, 30.0f, 1.0f ) ),
      alpha( 0.0f ),
	  rotate(TAU*0.5f)

{
	
	cubeSize = Vec3f(Rand::randFloat(500) + 500,Rand::randFloat(1000) + 500,Rand::randFloat(20) + 40);

	std::string boldFont( "Arial Bold" );
	
	// this does a complicated layout
	TextLayout layout;
	
	layout.setFont( Font( boldFont, 1204 ) );
	layout.setColor( Color( 0, 0, 0 ) );
	layout.addLine( std::string( "JAY-Z, I Made It!" ) );
	
	for ( int l = 0; l < 4; l++ ) {
        liberties[l] = -1;
    }

    state = boost::shared_ptr<TileState>( new EnterTileState() );
	
}

Tile::Tile( const Tile & rhs ) 
    : cluster( rhs.cluster ),
      id( rhs.id ),
      corner( rhs.corner ),
      rows( rhs.rows ),
      columns( rhs.columns ),
      position( rhs.position ),
      box( rhs.box ),
      color( rhs.color ),
      velocity( rhs.velocity ),
      alpha( rhs.alpha ),
	  rotate( rhs.rotate ),
	  cubeSize( rhs.cubeSize )
{
    
	for ( int l = 0; l < 4; l++ ) {
        liberties[l] = rhs.liberties[l];
    }
    state = rhs.state;
}

Tile & Tile::operator=( const Tile & rhs )
{
    id = rhs.id;
    corner = rhs.corner;
    rows = rhs.rows;
    columns = rhs.columns;
    position = rhs.position;
    box = rhs.box;
    color = rhs.color;
    velocity = rhs.velocity;
    alpha = rhs.alpha;
	rotate = rhs.rotate;
	cubeSize = rhs.cubeSize;
	
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

void Tile::addRotation( float variance )
{
    rotate += variance;
}

void Tile::setAlpha( float newAlpha )
{
    alpha = newAlpha;
}

void Tile::setRotation( float newRotation )
{
    rotate = newRotation;
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

        liberties[l] = cluster.tiles.size();
        cluster.addTile( grid, row, column, Rand::randFloat() * 20 - 10, newColor, id, (2 + l) % 4 );
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
    state = state->update( *this );
    position += velocity;
}

void Tile::draw()
{
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r, colorcolor.g, colorcolor.b, alpha );
    gl::pushModelView();    
	gl::translate( position );
	gl::rotate( rotate * (360.0f/TAU) );
	gl::translate( Vec3f( columns*atomWidth*-0.5f, rows*atomHeight*-0.5f, 0.0f ) );
	gl::translate( position );
	gl::drawCube( position, cubeSize );
//	gl::drawSolidRect( box);
	gl::color( Color::white() );
	
	gl::popModelView();
	
}
