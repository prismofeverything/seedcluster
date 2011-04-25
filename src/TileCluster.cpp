#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include <vector>
#include <algorithm>
#include "Ease.h"

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
    addTile( position, Rand::randInt( 5 ) + 1, Rand::randInt( 3 ) + 2, Rand::randFloat() * 50 - 25, color );
}

void TileCluster::seed( Vec2i center, Vec3f color )
{
    // seeds.push_back( Seed( center, color ) );
    chosenSeed = seeds.insert( seeds.begin(), Seed( center, color ) );
    chosenSeed->choose();
}

bool TileCluster::chooseSeed( Vec2i point )
{
    chosenSeed = std::find_if ( seeds.begin(), seeds.end(), SeedContains( point ) );
    if ( isSeedChosen() ) {
        chosenSeed->choose();
    }
    return isSeedChosen();
}

void TileCluster::releaseSeed()
{
    if ( isSeedChosen() ) {
        chosenSeed->release();
        chosenSeed = seeds.end();
    }
}

bool TileCluster::isSeedChosen()
{
    return chosenSeed != seeds.end();
}

void TileCluster::update()
{
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].update();
    }

    size = seeds.size();
    for ( int ee = 0; ee < size; ee++ ) {
        seeds[ee].update();
    }
}

void TileCluster::draw()
{
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].draw();
    }

    size = seeds.size();
    for ( int ee = 0; ee < size; ee++ ) {
        seeds[ee].draw();
    }
}
