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
    z = (Rand::randFloat() * -10.0) - currentZ;
	currentZ--;
    radiusEase = Ease( 1.0f, (Rand::randFloat() * 200.0f) + 10.0f, 200 );
}

int Seed::currentZ = -1.1;

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
	gl::drawSphere(Vec3f( center.x, center.y, z), radius);
	//gl::drawSolidCircle( center, radius );
    gl::popModelView();
}

