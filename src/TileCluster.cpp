#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include <vector>
#include <algorithm>

using namespace ci;
using namespace std;

TileCluster::TileCluster()
{

}

void TileCluster::bloomPoint( Vec2i position, Vec2f vel, Vec3f color )
{
    Tile bloom = Tile( position, Rand::randInt( 5 ) + 1, Rand::randInt( 5 ) + 1, Rand::randFloat()*20-10, color );
    tiles.push_back( bloom );
}

void TileCluster::update()
{
    for ( vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
        tile->update();
    }
}

void TileCluster::draw()
{
    for ( vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
        tile->draw();
    }
}
