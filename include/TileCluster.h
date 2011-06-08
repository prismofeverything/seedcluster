#pragma once
#include <vector>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>
#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "cinder/DataSource.h"
#include "Seed.h"
#include "Tile.h"

namespace ix
{

class TileCluster {
 public:
    TileCluster();
    void setup();
    void setupPosters();
    void setupShadows();
    void update();
    void draw();
    void drawSeeds();
    void drawTiles( bool posterMode );

    TileDimension chooseDimension();
    ci::Vec2i chooseOrientation();
    MovieInfo choosePoster();

    void clearSeeds();
    void addTile( ci::Vec2i position, 
                  TileDimension dim, float z, ci::Vec3f color );
    void mouseDown( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );
    void handOver( ci::Vec2i point );
    void generate( ci::Vec2f center );
    void unhover();
    void plantSeed( ci::Vec2i center, ci::Vec3f color );
    bool chooseSeed( ci::Vec2i point );
    void releaseSeed();
    bool isSeedChosen();
    
    static const float branchRate = 0.3;

    boost::adjacency_list<> tileGraph;

    ci::Vec2f tileOffset;
    ci::Vec3f tileScale;

    std::vector<Tile> tiles;
    std::vector<Tile> available;
    Tile * hoverTile;
    Tile * previousTile;
    Tile * chosenTile;

    std::vector<TileDimension> tileDimensions;
    std::vector<ci::Vec2i> orientations;
    std::vector<MovieInfo> posters;

    std::vector<Seed> seeds;
    std::vector<Seed>::iterator hoverSeed;
    std::vector<Seed>::iterator chosenSeed;
    
    ci::Vec2f lens;
};
    
struct z_depth_compare
{
    inline bool operator() (const Tile& t1, const Tile& t2)
    {
        return t1.position.z < t2.position.z;
    }
};

};

