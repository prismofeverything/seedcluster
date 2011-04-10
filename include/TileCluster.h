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

    void bloomPoint( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );

    std::vector<Tile> tiles;
};
