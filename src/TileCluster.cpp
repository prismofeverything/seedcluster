#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include <vector>
#include <algorithm>
#include "Ease.h"

using namespace ci;
using namespace std;

namespace ix
{

TileCluster::TileCluster()
{
    chosenSeed = seeds.end();
    hoverSeed = seeds.end();

    tileDimensions.push_back( Vec2i( 5, 3 ) );
    tileDimensions.push_back( Vec2i( 3, 2 ) );
    tileDimensions.push_back( Vec2i( 4, 3 ) );
    tileDimensions.push_back( Vec2i( 1, 2 ) );
    tileDimensions.push_back( Vec2i( 3, 4 ) );
    tileDimensions.push_back( Vec2i( 2, 3 ) );
    tileDimensions.push_back( Vec2i( 2, 2 ) );

    orientations.push_back( Vec2i( 0, 1 ) );
    orientations.push_back( Vec2i( -1, 0 ) );
    orientations.push_back( Vec2i( 0, -1 ) );
    orientations.push_back( Vec2i( 1, 0 ) );
}

Vec2i TileCluster::chooseDimension()
{
    return tileDimensions[ Rand::randInt( tileDimensions.size() ) ];
}

Vec2i TileCluster::chooseOrientation()
{
    return orientations[ Rand::randInt( orientations.size() ) ];
}

void TileCluster::addTile( Vec2i position, Vec2i dim, float z, Vec3f color, int from, int liberty )
{
    tiles.push_back( Tile( this, tiles.size(), position, dim, z, color ) );
    if ( from >= 0 ) {
        tiles[tiles.size() - 1].liberties[liberty] = from;
    }
}

void TileCluster::mouseDown( Vec2i position, Vec2f vel, Vec3f color )
{
    addTile( position, chooseDimension(), 0, color );
}

void TileCluster::clearSeeds()
{
    seeds.clear();
}

void TileCluster::handOver( Vec2i point )
{
    std::vector<Seed>::iterator previous = hoverSeed;
    hoverSeed = std::find_if ( seeds.begin(), seeds.end(), SeedContains( point ) );

    if ( previous == seeds.end() ) {
        if ( hoverSeed != seeds.end() ) {
            hoverSeed->hover();
        }
    } else if ( previous != hoverSeed ) {
        previous->unhover();
        if ( hoverSeed != seeds.end() ) {
            hoverSeed->hover();
        }
    }
}

void TileCluster::unhover()
{
    if ( hoverSeed != seeds.end() ) {
        hoverSeed->unhover();
        hoverSeed = seeds.end();
    }
}

void TileCluster::plantSeed( Vec2i center, Vec3f color )
{
    chosenSeed = seeds.insert( seeds.begin(), Seed( Vec2f( center.x, center.y ), color ) );
    chosenSeed->choose();
    hoverSeed = seeds.end();
}

bool TileCluster::chooseSeed( Vec2i point )
{
    chosenSeed = std::find_if ( seeds.begin(), seeds.end(), SeedContains( point ) );
    unhover();

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
    bool branching = Rand::randFloat() < branchRate && tiles.size() > 0;
    int yellow;
    Vec2i dim, topLeft, bottomRight;

    if ( branching ) {
        yellow = Rand::randInt( tiles.size() );
        dim = chooseDimension();
        topLeft = tiles[ yellow ].relativeCorner( dim, chooseOrientation() );
        bottomRight = topLeft + dim;
    }

    bool tileFits = true;
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].update();

        if ( branching && tileFits ) {
            tileFits = !tiles[ii].collidesWith( topLeft, bottomRight );
        }
    }

    if ( branching && tileFits ) {
        Vec3f newColor = tiles[ yellow ].color;
        newColor[2] = Rand::randFloat();
        addTile( topLeft, dim, 0, newColor );
    }

    size = seeds.size();
    for ( int ee = 0; ee < size; ee++ ) {
        seeds[ee].update();
    }
}

void TileCluster::draw()
{
    gl::pushModelView();
    gl::translate( Vec3f( 500.0f, 200.0f, 0.0f ) );
    gl::scale( Vec3f( 0.02f, 0.02f, 1.0f ) );

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].draw();
    }

    gl::popModelView();
    gl::disableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();

    // size = seeds.size();
    // for ( int ee = 0; ee < size; ee++ ) {
    //     seeds[ee].draw();
    // }
}

};
