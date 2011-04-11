#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"
#include <vector>
#include "Tile.h"

class TileCluster {
 public:
    TileCluster();
    void update();
    void draw();

    void addTile( ci::Vec2i position, 
                  int rows, int columns, float z, ci::Vec3f color );
                  //                  int from=-1, int liberty=-1 );
    void mouseDown( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );

    std::vector<Tile> tiles;
};
