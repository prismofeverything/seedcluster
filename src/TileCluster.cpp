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

void TileCluster::addTile( Vec2i position, int rows, int columns, float z, Vec3f color ) // , int from, int liberty )
{
    tiles.push_back( Tile( *this, position, rows, columns, z, color ) );
    // if ( liberty >= 0 && liberty < 4) {
    //     tiles[tiles.size() - 1].liberties[liberty] = 
    // }
}

void TileCluster::mouseDown( Vec2i position, Vec2f vel, Vec3f color )
{
    addTile( position, Rand::randInt( 5 ) + 1, Rand::randInt( 5 ) + 1, Rand::randFloat() * 20 - 10, color );
    // tiles.push_back( Tile( *this, position, Rand::randInt( 5 ) + 1, Rand::randInt( 5 ) + 1, Rand::randFloat()*20-10, color ) );
}

void TileCluster::update()
{
    // for ( vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].update();
    }
}

void TileCluster::draw()
{
    //    for ( vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].draw();
    }
}
