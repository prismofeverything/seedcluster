#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include <vector>
#include "Seed.h"
#include "Tile.h"

class TileCluster {
 public:
    TileCluster();
    void update();
    void draw();

    void clearSeeds();
    void addTile( ci::Vec2i position, 
                  int rows, int columns, float z, ci::Vec3f color, 
                  int from=-1, int liberty=-1 );
    void mouseDown( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );
    void handOver( ci::Vec2i point );
    void unhover();
    void plantSeed( ci::Vec2i center, ci::Vec3f color );
    bool chooseSeed( ci::Vec2i point );
    void releaseSeed();
    bool isSeedChosen();

    std::vector<Seed>::iterator hoverSeed;
    std::vector<Seed>::iterator chosenSeed;
    std::vector<Seed> seeds;
    std::vector<Tile> tiles;
};
