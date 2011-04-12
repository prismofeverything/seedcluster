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

void TileCluster::addTile( Vec2i position, int rows, int columns, float z, Vec3f color, int from, int liberty )
{
    tiles.push_back( Tile( *this, tiles.size(), position, rows, columns, z, color ) );
    if ( from >= 0 ) {
        tiles[tiles.size() - 1].liberties[liberty] = from;
    }
}

void TileCluster::mouseDown( Vec2i position, Vec2f vel, Vec3f color )
{
    addTile( position, Rand::randInt( 5 ) + 1, Rand::randInt( 3 ) + 2, Rand::randFloat() * 500 - 25, color );
}

void TileCluster::update()
{
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].update();
    }
}

void TileCluster::draw()
{
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].draw();
    }
}
