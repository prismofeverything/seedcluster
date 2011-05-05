#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "TileState.h"
#include "Seed.h"
#include "TileCluster.h"
#include "Ease.h"
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

Seed::Seed( Vec2f _center, Vec3f _color )
    : center( _center ),
      color( _color ),
      originalColor( _color ), 
      alpha( 0.8f ),
      radius( 1.0f ),
      zoomFactor( 1.0f )
{
    z = 0; // Rand::randFloat() * -20.0 - 20.0;
    baseRadius = Rand::randFloat() * 50.0f + 40.0f;
    hoverRadius = baseRadius * 1.2f;
    radiusEase = Ease( 1.0f, baseRadius, 100 );
}

void Seed::hover()
{
    radiusEase = Ease( radius, hoverRadius, 50 );
}

void Seed::unhover()
{
    radiusEase = Ease( radius, baseRadius, 50 );
}

void Seed::choose()
{
    brightnessEase = Ease( color[2], 1.0f, 50 );
}

void Seed::release()
{
    brightnessEase = Ease( color[2], originalColor[2], 50 );
}

void Seed::seek( ci::Vec2f towards )
{
    center += (towards - center) * 0.8;

    // xEase.realign( center[0], towards[0], 50 );
    // yEase.realign( center[1], towards[1], 50 );
}

void Seed::zoom( float factor )
{
    zoomFactor += (factor - zoomFactor) * 0.1;

    // zoomFactor += factor * 0.05;
    // zoomFactor += factor * 0.1;
    // // zoomFactor -= zoomFactor * 0.005;

    // // float adjustment = radius * pow( 1.02f, factor ) - radius;
    // // zoomFactor += adjustment * 0.1;
    // // zoomFactor -= zoomFactor * 0.01;

    // radius += zoomFactor;
    // baseRadius += zoomFactor;
    // hoverRadius += zoomFactor * 1.2f;

    // // radius += zoomFactor * 0.2;
    // // baseRadius += zoomFactor * 0.2;
}

void Seed::update()
{
    if ( !radiusEase.done() ) {
        radius = radiusEase.out();
    }
    if ( !brightnessEase.done() ) {
        color[2] = brightnessEase.out();
    }

    // if ( !xEase.done() ) {
    //     center[0] = xEase.out();
    // }
    // if ( !yEase.done() ) {
    //     center[1] = yEase.out();
    // }
}

void Seed::draw()
{
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r, colorcolor.g, colorcolor.b, alpha );
    gl::pushModelView();
    gl::translate( Vec3f( 0.0f, 0.0f, z ) );
    gl::drawSolidCircle( center, radius * zoomFactor );
    gl::popModelView();
}

