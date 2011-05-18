#include <vector>
#include <iostream>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/ip/Fill.h"
#include "Tile.h"
#include "TileCluster.h"

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

namespace ix
{

Tile::Tile( TileCluster * clust, int index, Vec2i grid, Vec2i dim, float z, Vec3f col, MovieInfo movie )
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
      state( Entering ),
      movieinfo( movie )
{
    ci::Vec2i posterdim( atomWidth * dim[0], atomHeight * dim[1] * 0.8f );
    gl::Texture::Format format;
    format.enableMipmapping( true );
    format.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
    ci::Surface fullsize = loadImage( "/Users/rspangler/Desktop/movie_posters/" + movie.image );
    ci::Vec2i difference = (fullsize.getSize() - posterdim) / 2;
    ci::Surface field = fullsize.clone( ci::Area( difference, difference + posterdim ) );
    poster = gl::Texture( field, format );

    layout.clear( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    layout.setFont( segoebold );
    layout.setColor( Color( 0, 0.8f, 0 ) );
    layout.addLine( movie.title );
    layout.setFont( segoesemibold );
    layout.setColor( Color( 0, 0, 0 ) );
    layout.addLine( movie.year + ",  " );
    layout.setFont( segoe );
    layout.append( movie.genre );

    ci::Vec2i infodim( atomWidth*dim[0], atomHeight*dim[1]*0.2f );
    Surface info( infodim[0], infodim[1], true );
    ci::ip::fill( &info, ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    Surface tag = layout.render( true, false );
    info.copyFrom( tag, ci::Area( ci::Vec2i( 0, 0 ), tag.getSize() ), ci::Vec2i( 10, 10 ) );
    posterinfo = gl::Texture( info, format );
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

void Tile::drawPoster()
{
    glColor4f( 1.0f, 1.0f, 1.0f, alpha );
    gl::pushModelView();
    gl::translate( position );

    gl::draw( poster );
    gl::translate( ci::Vec3f( 0.0f, atomHeight * dimension[1] * 0.8, 0.0f ) );
    gl::draw( posterinfo );
    // gl::draw( poster, box );
    gl::popModelView();
}

ci::Font Tile::segoe = Font( "Segoe", 18 );
ci::Font Tile::segoebold = Font( "Segoe Bold", 24 );
ci::Font Tile::segoesemibold = Font( "Segoe Bold", 18 );

};
