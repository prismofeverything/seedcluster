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
#include <vector>

using namespace ci;
using namespace std;

#define TAU 6.2831853071795862f

Seed::Seed( Vec2i _center, Vec3f _color )
    : center( _center ),
      color( _color ),
      radiusEase( 1.0f, 50.0f, 40.0f ),
      alpha( 1.0f )
{

}

void Seed::update()
{
    if ( !radiusEase.done() ) {
        radius = radiusEase.in();
    }
}

void Seed::draw()
{
    Color colorcolor = Color( CM_HSV, color );
    glColor4f( colorcolor.r, colorcolor.g, colorcolor.b, alpha );
    gl::drawSolidCircle( center, radius );
}
