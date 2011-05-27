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
#define INFOHEIGHT 162.0f

namespace ix
{

Tile::Tile( TileCluster * clust, int index, Vec2i grid, TileDimension dim, float z, Vec3f col, MovieInfo movie )
    : cluster( clust ),
      id( index ),
      dimension( dim ),
      topLeft( grid ),
      bottomRight( grid + dim.first ), 
      position( Vec3f( grid[0]*atomWidth, grid[1]*atomHeight, z ) ),
      box( Rectf( 0, 0, atomWidth*dim.first[0], atomHeight*dim.first[1] ) ),
      color( col ),
      velocity( Vec3f( 0.0f, 0.0f, 0.0f ) ),
      alpha( 0.0f ),
      alphaEase( 0.0f, 0.9f, 40 ),
      state( Entering ),
      movieinfo( movie )
{
    gl::Texture::Format format;
    format.enableMipmapping( true );
    format.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
    format.setMagFilter( GL_LINEAR_MIPMAP_LINEAR );

    ci::Vec2f posterdim( atomWidth * dim.first[0], atomHeight * dim.first[1] - INFOHEIGHT );
    float posterratio = posterdim[0] / posterdim[1];

    ci::Vec2f moviedim = movie.image.getSize();
    float movieratio = moviedim[0] / moviedim[1];

    ci::Vec2i offset;
    float movieposterratio;
    if ( movieratio < posterratio ) {
        float clipheight = moviedim[0] / posterratio;
        movieposterratio = moviedim[0] / posterdim[0];
        offset = Vec2i( 0, (moviedim[1] - clipheight) * 0.5 );
    } else {
        float clipwidth = moviedim[1] * posterratio;
        movieposterratio = moviedim[1] / posterdim[1];
        offset = Vec2i( (moviedim[0] - clipwidth) * 0.5, 0 );
    }

    field = ci::Area( offset, offset + ( Vec2f( posterdim[0], posterdim[1] ) * movieposterratio ) );

    layout.clear( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    layout.setFont( segoebold );
    layout.setColor( Color( 0, 0.8f, 0 ) );
    layout.addLine( movie.title );
    layout.setFont( segoesemibold );
    layout.setColor( Color( 0, 0, 0 ) );
    layout.addLine( movie.year + ",  " );
    layout.setFont( segoe );
    layout.append( movie.genre );

    ci::Vec2i infodim( atomWidth*dim.first[0], INFOHEIGHT );
    Surface info( infodim[0], infodim[1], true );
    ci::ip::fill( &info, ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    Surface tag = layout.render( true, false );
    info.copyFrom( tag, ci::Area( ci::Vec2i( 0, 0 ), tag.getSize() ), ci::Vec2i( 25, 20 ) );
    posterinfo = gl::Texture( info, format );
}

Vec2i Tile::relativeCorner( Vec2i dim, Vec2i orientation )
{
    Vec2i grid;
    for ( int i = 0; i < 2; i++ ) {
        if ( orientation[i] == 0 ) {
            int axisSpread = dim[i] - 1;
            int offset = Rand::randInt( axisSpread + dimension.first[i] ) - axisSpread;
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
    gl::pushMatrices();
    gl::translate( position );
    gl::drawSolidRect( box );

    drawShadow();

    gl::popMatrices();
}

void Tile::drawShadow()
{
    gl::pushMatrices();
    glColor4f( 0.0f, 0.0f, 0.0f, alpha );
    gl::disableDepthWrite();

    // glEnable( GL_BLEND );
    // glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    // glBlendFunc( GL_DST_COLOR, GL_SRC_ALPHA );
    // glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    // glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_DST_COLOR, GL_SRC_ALPHA);

    // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    // glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);   //Modulate RGB with RGB
    // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    // glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
    // glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
    // glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);  //Modulate ALPHA with ALPHA
    // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    // glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    // glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
    // glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

    // gl::translate( position );

    gl::translate( ci::Vec3f( -50.0f, -50.0f, 0.0f ) );
    gl::draw( dimension.second );
    gl::popMatrices();
    gl::enableDepthWrite();
}

void Tile::drawPoster()
{
    glColor4f( 1.0f, 1.0f, 1.0f, alpha );

    ci::Vec2i posterdim( atomWidth * dimension.first[0], atomHeight * dimension.first[1] - INFOHEIGHT );
    gl::pushMatrices();
    gl::translate( position );
    gl::draw( movieinfo.image, field, ci::Rectf( Vec2i( 0, 0 ), posterdim ) );

    gl::pushMatrices();
    gl::translate( ci::Vec3f( 0.0f, atomHeight * dimension.first[1] - INFOHEIGHT, 0.0f ) );
    gl::draw( posterinfo );
    gl::popMatrices();

    drawShadow();

    gl::popMatrices();
}

ci::Font Tile::segoe = Font( "Segoe", 24 );
ci::Font Tile::segoebold = Font( "Segoe Bold", 32 );
ci::Font Tile::segoesemibold = Font( "Segoe Bold", 24 );

};
