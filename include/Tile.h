#pragma once
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp>
#include "cinder/app/App.h"
#include "cinder/Cinder.h"
#include "cinder/DataSource.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Area.h"
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "Ease.h"

using namespace ci;
using namespace ci::app;

namespace ix
{

enum TileState { Entering, FirstHover, Blooming, Hovering, UnHover, Leaving, Nixed };
enum Collision { Unrelated, Adjacent, Overlapping };
typedef std::pair<ci::Vec2i, ci::gl::Texture> TileDimension;
typedef boost::graph_traits< boost::adjacency_list<> >::vertex_descriptor Vertex;

class TileCluster;

struct MovieInfo {
MovieInfo( std::string _title, std::string _year, std::string _genre, ci::gl::Texture _image )
      : title( _title ), 
        year( _year ),
        genre( _genre ),
        image( _image ) {};

    std::string title;
    std::string year;
    std::string genre;
    ci::gl::Texture image;
};

class Tile {
 public:
    Tile( TileCluster * clust, 
          int id,
          ci::Vec2i grid, 
          TileDimension dim,
          float z, 
          ci::Vec3f col,
          MovieInfo movie,
          Vertex v );

    //    virtual ~Tile();

    void update();
    void draw();
    void drawPoster();
    void drawShadow();

    void enter();
    void hover();
    void unhover();
    void leave();
    
    ci::Vec2i relativeCorner( ci::Vec2i dim, ci::Vec2i orientation );
    Collision collidesWith( ci::Vec2i tl, ci::Vec2i br );

    static const int atomWidth = 240;
    static const int atomHeight = 270;

    static ci::Font segoe;
    static ci::Font segoebold;
    static ci::Font segoesemibold;

    TileCluster * cluster;
    TileState state;

    ci::Rectf box;
    ci::Vec3f position;
    
    ci::Vec3f velocity;
    ci::Vec3f color;
    ci::gl::Texture shadow;
  
    MovieInfo movieinfo;

    ci::gl::Texture poster;
    ci::gl::Texture posterinfo;
    ci::TextLayout layout;

    ci::Area field;
    Vertex vertex;

    int id;
    ci::Vec2i topLeft;
    ci::Vec2i bottomRight;
    TileDimension dimension;
    float alpha;
    Ease alphaEase;
    float scrimAlpha;
    Ease scrimAlphaEase;
    ci::Vec3f positionOffset;
    Ease positionOffsetZEase;
    ci::Vec3f rotation;
    Ease rotationYEase;
    
    int leaveTimer;
    bool visible;
};

struct TileContains
{
    TileContains( ci::Vec2i _point ) : point( _point ) {};
    inline bool operator() ( const Tile & tile ) {
        bool result = point[0] > tile.position[0]
            && point[1] > tile.position[1]
            && point[0] < tile.position[0] + tile.box.getLowerRight()[0]
            && point[1] < tile.position[1] + tile.box.getLowerRight()[1];
        
        return result;
    };

    ci::Vec2i point;
};

};
