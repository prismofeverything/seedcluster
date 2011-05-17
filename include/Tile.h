#pragma once
#include <vector>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "Ease.h"

namespace ix
{

enum TileState { Entering, Blooming, Leaving };

class TileCluster;

class Tile {
 public:
    Tile( TileCluster * clust, 
          int id,
          ci::Vec2i grid, 
          int row, 
          int column, 
          float z, 
          ci::Vec3f col );

    //    virtual ~Tile();

    void update();
    void draw();
    bool branch();

    static const int atomWidth = 240;
    static const int atomHeight = 270;

    TileCluster * cluster;
    TileState state;

    ci::Rectf box;
    ci::Vec3f position;
    ci::Vec3f velocity;
    ci::Vec3f color;
  
    int id;
    ci::Vec2i corner;
    int rows;
    int columns;
    float alpha;
    Ease alphaEase;

    int liberties[4];
};

};
