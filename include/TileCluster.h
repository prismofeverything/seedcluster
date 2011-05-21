#pragma once
#include <vector>
#include <string>
#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include "cinder/DataSource.h"
#include "Seed.h"
#include "Tile.h"

namespace ix
{

class TileCluster {
 public:
    TileCluster();
    void update();
    void draw( bool posterMode );

    TileDimension chooseDimension();
    ci::Vec2i chooseOrientation();
    MovieInfo choosePoster();

    void clearSeeds();
    void addTile( ci::Vec2i position, 
                  TileDimension dim, float z, ci::Vec3f color );
    void mouseDown( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );
    void handOver( ci::Vec2i point );
    void unhover();
    void plantSeed( ci::Vec2i center, ci::Vec3f color );
    bool chooseSeed( ci::Vec2i point );
    void releaseSeed();
    bool isSeedChosen();

    static const float branchRate = 0.3;

    std::vector<Tile> tiles;
    std::vector<Tile> available;
    std::vector<TileDimension> tileDimensions;
    std::vector<ci::Vec2i> orientations;
    /* std::map<ci::Vec2i, ci::DataSourceRef> shadowmap; */
    std::vector<MovieInfo> posters;

    std::vector<Seed> seeds;
    std::vector<Seed>::iterator hoverSeed;
    std::vector<Seed>::iterator chosenSeed;
};

};

