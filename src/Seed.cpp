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

Seed::Seed( Vec2i _center, Vec3f _color )
    : center( _center ),
      color( _color ),
      alpha( 0.8f ),
      radius( 1.0f )
{
    z = Rand::randFloat() * 20.0 + 20.0;
    radiusEase = Ease( 1.0f, Rand::randFloat() * 80.0f + 40.0f, 100 );
}

void Seed::update()
{
    if ( !radiusEase.done() ) {
        radius = radiusEase.out();
    }
}

void Seed::draw()
{
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r, colorcolor.g, colorcolor.b, alpha );
    gl::pushModelView();
    gl::translate( Vec3f( 0.0f, 0.0f, z ) );
    gl::drawSolidCircle( center, radius );
    gl::popModelView();
}

