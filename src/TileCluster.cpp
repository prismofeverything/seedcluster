#include <vector>
#include <map>
#include <algorithm>
#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "Resources.h"
#include "Ease.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ix
{

TileCluster::TileCluster()
{
    chosenSeed = seeds.end();
    hoverSeed = seeds.end();

    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 5, 3 ), loadResource( RES_LONGSHADOW_1200x810 ) ) );
    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 2, 2 ), loadResource( RES_LONGSHADOW_480x540 ) ) );
    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 2, 3 ), loadResource( RES_LONGSHADOW_480x810 ) ) );
    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 3, 4 ), loadResource( RES_LONGSHADOW_720x1080 ) ) );
    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 3, 2 ), loadResource( RES_LONGSHADOW_720x540 ) ) );
    tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 4, 3 ), loadResource( RES_LONGSHADOW_960x810 ) ) );
    // tileDimensions.push_back( pair<Vec2i, DataSourceRef>( Vec2i( 1, 2 ), loadResource( RES_LONGSHADOW_240x540 ) ) );

    orientations.push_back( Vec2i( 0, 1 ) );
    orientations.push_back( Vec2i( -1, 0 ) );
    orientations.push_back( Vec2i( 0, -1 ) );
    orientations.push_back( Vec2i( 1, 0 ) );

    loadPosters();
}

TileDimension TileCluster::chooseDimension()
{
    return tileDimensions[ Rand::randInt( tileDimensions.size() ) ];
}

Vec2i TileCluster::chooseOrientation()
{
    return orientations[ Rand::randInt( orientations.size() ) ];
}

MovieInfo TileCluster::choosePoster()
{
    return posters[ Rand::randInt( posters.size() ) ];
}

void TileCluster::addTile( Vec2i position, TileDimension dim, float z, Vec3f color )
{
    tiles.push_back( Tile( this, tiles.size(), position, dim, z, color, choosePoster() ) );
}

void TileCluster::mouseDown( Vec2i position, Vec2f vel, Vec3f color )
{
    addTile( position, chooseDimension(), 0, color );
}

void TileCluster::clearSeeds()
{
    seeds.clear();
}

void TileCluster::handOver( Vec2i point )
{
    std::vector<Seed>::iterator previous = hoverSeed;
    hoverSeed = std::find_if ( seeds.begin(), seeds.end(), SeedContains( point ) );

    if ( previous == seeds.end() ) {
        if ( hoverSeed != seeds.end() ) {
            hoverSeed->hover();
        }
    } else if ( previous != hoverSeed ) {
        previous->unhover();
        if ( hoverSeed != seeds.end() ) {
            hoverSeed->hover();
        }
    }
}

void TileCluster::unhover()
{
    if ( hoverSeed != seeds.end() ) {
        hoverSeed->unhover();
        hoverSeed = seeds.end();
    }
}

void TileCluster::plantSeed( Vec2i center, Vec3f color )
{
    chosenSeed = seeds.insert( seeds.begin(), Seed( Vec2f( center.x, center.y ), color ) );
    chosenSeed->choose();
    hoverSeed = seeds.end();
}

bool TileCluster::chooseSeed( Vec2i point )
{
    chosenSeed = std::find_if ( seeds.begin(), seeds.end(), SeedContains( point ) );
    unhover();

    if ( isSeedChosen() ) {
        chosenSeed->choose();
    }
    return isSeedChosen();
}

void TileCluster::releaseSeed()
{
    if ( isSeedChosen() ) {
        chosenSeed->release();
        chosenSeed = seeds.end();
    }
}

bool TileCluster::isSeedChosen()
{
    return chosenSeed != seeds.end();
}

void TileCluster::update()
{
    bool branching = Rand::randFloat() < branchRate && tiles.size() > 0;
    int yellow;
    TileDimension dim;
    Vec2i topLeft, bottomRight;

    if ( branching ) {
        yellow = Rand::randInt( tiles.size() );
        dim = chooseDimension();
        topLeft = tiles[ yellow ].relativeCorner( dim.first, chooseOrientation() );
        bottomRight = topLeft + dim.first;
    }

    bool tileFits = true;
    int size = tiles.size();
    for ( int ii = 0; ii < size; ii++ ) {
        tiles[ii].update();

        if ( branching && tileFits ) {
            tileFits = !tiles[ii].collidesWith( topLeft, bottomRight );
        }
    }

    if ( branching && tileFits ) {
        Vec3f newColor = tiles[ yellow ].color;
        newColor[2] = Rand::randFloat();
        addTile( topLeft, dim, 0, newColor );
    }

    size = seeds.size();
    for ( int ee = 0; ee < size; ee++ ) {
        seeds[ee].update();
    }
}

void TileCluster::draw( bool posterMode )
{
    gl::pushModelView();
    gl::translate( Vec3f( 100.0f, 200.0f, 0.0f ) );
    gl::scale( Vec3f( 0.15f, 0.15f, 1.0f ) );

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::disableDepthWrite();

    int size = tiles.size();

    if ( posterMode ) {
        for ( int ii = 0; ii < size; ii++ ) {
            tiles[ii].drawPoster();
        }
    } else {
        for ( int ii = 0; ii < size; ii++ ) {
            tiles[ii].draw();
        }
    }

    // gl::disableDepthWrite();

    // for ( int ii = 0; ii < size; ii++ ) {
    //     tiles[ii].drawShadow();
    // }

    gl::popModelView();
    gl::disableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();

    // size = seeds.size();
    // for ( int ee = 0; ee < size; ee++ ) {
    //     seeds[ee].draw();
    // }
}

void TileCluster::loadPosters()
{
    posters.push_back( MovieInfo( "The Matrix", "2001", "Science Fiction", loadResource( RES_Matrix_Reloaded_Wallpaper3 ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", loadResource( RES_04 ) ) );
    posters.push_back( MovieInfo( "Eternal Sunshine of the Spotless Mind", "2009", "Science Fiction", loadResource( RES_1166952147_1024x768_movie_stills_of_eternal_sunshine_of_the_spotless_mind ) ) );
    posters.push_back( MovieInfo( "Sleepy Hollow", "2009", "Science Fiction", loadResource( RES_1287369723_1024x768_sleepy_hollow_poster ) ) );
    posters.push_back( MovieInfo( "2001: Space Odyssey", "2009", "Science Fiction", loadResource( RES_2001_space_odyssey_1280x1048 ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", loadResource( RES_2007_sunshine_2 ) ) );
    posters.push_back( MovieInfo( "2010: Contact", "2009", "Science Fiction", loadResource( RES_2010_movie_poster_year_we_make_contact ) ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", loadResource( RES_936full_solaris_poster_1 ) ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", loadResource( RES_936full_solaris_poster ) ) );
    posters.push_back( MovieInfo( "A Christmas Carol", "2009", "Science Fiction", loadResource( RES_Jim_Carrey_in_A_Christmas_Carol_Wallpaper_2_800 ) ) );
    posters.push_back( MovieInfo( "Little Miss Sunshine", "2009", "Science Fiction", loadResource( RES_Little_Miss_Sunshine_by_lakikaki ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", loadResource( RES_MOON_Poster_by_pepperberetta ) ) );
    posters.push_back( MovieInfo( "Aliens", "2009", "Science Fiction", loadResource( RES_Movie_Poster_Aliens ) ) );
    posters.push_back( MovieInfo( "Liar Liar", "2009", "Science Fiction", loadResource( RES_Movie_Poster_Liar_Liar ) ) );
    posters.push_back( MovieInfo( "Nine", "2009", "Science Fiction", loadResource( RES_Nine_movie_still_3 ) ) );
    posters.push_back( MovieInfo( "Terminator 2: Judgment Day", "2009", "Science Fiction", loadResource( RES_Terminator_2_Judgment_Day_movie_poster ) ) );
    posters.push_back( MovieInfo( "The Fountain", "2009", "Science Fiction", loadResource( RES_The_Fountain_the_fountain_5530120_1280_1024 ) ) );
    posters.push_back( MovieInfo( "The Matrix: Reloaded", "2009", "Science Fiction", loadResource( RES_The_Matrix_Reloaded_poster ) ) );
    posters.push_back( MovieInfo( "True Lies", "2009", "Science Fiction", loadResource( RES_True_Lies_lobby_card_Arnold_Schwarzenegger_Jamie_Lee_Curtis_James_Cameron_1994_TV_remake ) ) );
    posters.push_back( MovieInfo( "Alice in Wonderland", "2009", "Science Fiction", loadResource( RES_alice_in_wonderland_xlg ) ) );
    posters.push_back( MovieInfo( "Alien 3", "2009", "Science Fiction", loadResource( RES_alien3 ) ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", loadResource( RES_avatar_poster_frensc ) ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", loadResource( RES_avatar_poster ) ) );
    posters.push_back( MovieInfo( "Black Swan", "2009", "Science Fiction", loadResource( RES_blackswan_poster ) ) );
    posters.push_back( MovieInfo( "Close Encounters of the Third Kind", "2009", "Science Fiction", loadResource( RES_close_encounters_of_the_third_kind_1_1024 ) ) );
    posters.push_back( MovieInfo( "Corpse Bride", "2009", "Science Fiction", loadResource( RES_corpse_bride_6_1024 ) ) );
    posters.push_back( MovieInfo( "Green Hornet", "2009", "Science Fiction", loadResource( RES_greenhornet ) ) );
    posters.push_back( MovieInfo( "Moon", "2009", "Science Fiction", loadResource( RES_mpamoonposter2b ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", loadResource( RES_mpayesmanposterb ) ) );
    posters.push_back( MovieInfo( "Sweeney Todd", "2009", "Science Fiction", loadResource( RES_sweeneytodd ) ) );
    posters.push_back( MovieInfo( "Terminator: Salvation", "2009", "Science Fiction", loadResource( RES_terminator_salvation_poster2 ) ) );
    posters.push_back( MovieInfo( "Terminator", "2009", "Science Fiction", loadResource( RES_terminator_poster4 ) ) );
    posters.push_back( MovieInfo( "Terminator: Other", "2009", "Science Fiction", loadResource( RES_terminator_salvation_21 ) ) );
    posters.push_back( MovieInfo( "The Mask", "2009", "Science Fiction", loadResource( RES_the_mask_original ) ) );
    posters.push_back( MovieInfo( "The Abyss", "2009", "Science Fiction", loadResource( RES_the_abyss_1989 ) ) );
    posters.push_back( MovieInfo( "The Nightmare Before Christmas", "2009", "Science Fiction", loadResource( RES_the_nightmare_before_christmas_poster ) ) );
    posters.push_back( MovieInfo( "Titanic", "2009", "Science Fiction", loadResource( RES_titanic_ver2_xlg ) ) );
    posters.push_back( MovieInfo( "Alien", "2009", "Science Fiction", loadResource( RES_vintage_alien_movie_poster ) ) );
    posters.push_back( MovieInfo( "Batman", "2009", "Science Fiction", loadResource( RES_viral_batman_poster ) ) );
}

};
