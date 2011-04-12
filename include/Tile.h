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
	void setAlpha( float newAlpha );
    bool branch();

    static const int atomWidth = 270;
    static const int atomHeight = 240;

    TileCluster & cluster;

    ci::Rectf box;
    ci::Vec3f position;
    ci::Vec3f velocity;
    ci::Vec3f color;
  
    ci::Vec2i corner;
    int rows;
    int columns;
    float alpha;

    boost::shared_ptr<TileState> state;
    int liberties[4];
};
