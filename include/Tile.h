#pragma once
#include <vector>
#include "cinder/Cinder.h"
#include "cinder/DataSource.h"
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "cinder/Font.h"
#include "Ease.h"

namespace ix
{

enum TileState { Entering, Blooming, Leaving };

class TileCluster;

struct MovieInfo {
MovieInfo( std::string _title, std::string _year, std::string _genre, ci::DataSourceRef _image )
      : title( _title ), 
        year( _year ),
        genre( _genre ),
        image( _image ) {};

    std::string title;
    std::string year;
    std::string genre;
    ci::DataSourceRef image;
};

class Tile {
 public:
    Tile( TileCluster * clust, 
          int id,
          ci::Vec2i grid, 
          ci::Vec2i dim,
          float z, 
          ci::Vec3f col,
          MovieInfo movie,
          ci::DataSourceRef shadowresource );

    //    virtual ~Tile();

    void update();
    void draw();
    void drawPoster();

    ci::Vec2i relativeCorner( ci::Vec2i dim, ci::Vec2i orientation );
    bool collidesWith( ci::Vec2i tl, ci::Vec2i br );

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

    int id;
    ci::Vec2i topLeft;
    ci::Vec2i bottomRight;
    ci::Vec2i dimension;
    float alpha;
    Ease alphaEase;
};

};
