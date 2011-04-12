#pragma once
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "TileState.h"
#include <vector>

class TileCluster;

class Tile {
 public:
    Tile( TileCluster & clust, 
          int id,
          ci::Vec2i grid, 
          int row, 
          int column, 
          float z, 
          ci::Vec3f col );

    Tile( const Tile & rhs );
    Tile & operator=( const Tile & rhs );

    void update();
    void draw();

    void addAlpha( float variance );
    bool branch();

    static const int atomWidth = 240;
    static const int atomHeight = 270;

    TileCluster & cluster;

    ci::Rectf box;
    ci::Vec3f position;
    ci::Vec3f velocity;
    ci::Vec3f color;
  
    int id;
    ci::Vec2i corner;
    int rows;
    int columns;
    float alpha;

    boost::shared_ptr<TileState> state;
    int liberties[4];
};
