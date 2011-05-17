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
          ci::Vec2i dim,
          float z, 
          ci::Vec3f col );

    //    virtual ~Tile();

    void update();
    void draw();

    ci::Vec2i relativeCorner( ci::Vec2i dim, ci::Vec2i orientation );
    bool collidesWith( ci::Vec2i tl, ci::Vec2i br );

    static const int atomWidth = 240;
    static const int atomHeight = 270;

    TileCluster * cluster;
    TileState state;

    ci::Rectf box;
    ci::Vec3f position;
    ci::Vec3f velocity;
    ci::Vec3f color;
  
    int id;
    ci::Vec2i topLeft;
    ci::Vec2i bottomRight;
    ci::Vec2i dimension;
    float alpha;
    Ease alphaEase;

    int liberties[4];
};

};
