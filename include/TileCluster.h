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
    Tile & addTile( ci::Vec2i position, 
                    TileDimension dim, float z, ci::Vec3f color );
    void mouseDown( ci::Vec2i position, ci::Vec2f velocity, ci::Vec3f color );
    void handOver( ci::Vec2i point );
    void generateUnder( ci::Vec2f center );
    void generate( TileDimension dim, ci::Vec2i topLeft );
    void unhover();
    void plantSeed( ci::Vec2i center, ci::Vec3f color );
    bool chooseSeed( ci::Vec2i point );
    void releaseSeed();
    bool isSeedChosen();
    
    static const float branchRate = 0.3;

    TileGraph tileGraph;

    ci::Vec2f tileOffset;

    std::vector<Tile> tiles;
    std::vector<Tile> available;
    Tile * hoverTile;
    Tile * previousTile;
    Tile * chosenTile;
    std::map<Vertex, Tile *> vertexmap;

    std::vector<TileDimension> tileDimensions;
    std::vector<ci::Vec2i> orientations;
    std::vector<MovieInfo> posters;

    std::vector<Seed> seeds;
    std::vector<Seed>::iterator hoverSeed;
    std::vector<Seed>::iterator chosenSeed;
    
    ci::Vec2f lens;
    
    // -- two hands
    double distance1;
    double distance2;
    
    float scaling;
    float prevDelta;
    float currDelta;
    
    ci::Vec2f startOffset;
    ci::Vec3f targScale;
    ci::Vec3f tileScale;
    
    void twoHandsIn( ci::Vec2i first, ci::Vec2i second );
    void twoHandsMove( ci::Vec2i first, ci::Vec2i second );
    void secondHandOut();
};
    
struct z_depth_compare
{
    inline bool operator() (const Tile& t1, const Tile& t2)
    {
        return t1.position.z > t2.position.z;
    }
};

};

