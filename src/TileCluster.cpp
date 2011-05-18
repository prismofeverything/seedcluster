#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include <vector>
#include <algorithm>
#include "Ease.h"

using namespace ci;
using namespace std;

namespace ix
{

TileCluster::TileCluster()
{
    chosenSeed = seeds.end();
    hoverSeed = seeds.end();

    tileDimensions.push_back( Vec2i( 5, 3 ) );
    tileDimensions.push_back( Vec2i( 3, 2 ) );
    tileDimensions.push_back( Vec2i( 4, 3 ) );
    tileDimensions.push_back( Vec2i( 1, 2 ) );
    tileDimensions.push_back( Vec2i( 3, 4 ) );
    tileDimensions.push_back( Vec2i( 2, 3 ) );
    tileDimensions.push_back( Vec2i( 2, 2 ) );

    orientations.push_back( Vec2i( 0, 1 ) );
    orientations.push_back( Vec2i( -1, 0 ) );
    orientations.push_back( Vec2i( 0, -1 ) );
    orientations.push_back( Vec2i( 1, 0 ) );

    posters.push_back( MovieInfo( "The Matrix", "2001", "Science Fiction", "(311009004425)Matrix_Reloaded_Wallpaper3.jpg" ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", "04.jpg" ) );
    posters.push_back( MovieInfo( "Eternal Sunshine of the Spotless Mind", "2009", "Science Fiction", "1166952147_1024x768_movie-stills-of-eternal-sunshine-of-the-spotless-mind.jpg" ) );
    posters.push_back( MovieInfo( "Sleepy Hollow", "2009", "Science Fiction", "1287369723_1024x768_sleepy-hollow-poster.jpg" ) );
    posters.push_back( MovieInfo( "2001: Space Odyssey", "2009", "Science Fiction", "2001-space-odyssey_1280x1048.jpg" ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", "2007-sunshine-2.jpg" ) );
    posters.push_back( MovieInfo( "2010: Contact", "2009", "Science Fiction", "2010_movie_poster_year_we_make_contact.jpg" ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", "936full-solaris-poster-1.jpg" ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", "936full-solaris-poster.jpg" ) );
    posters.push_back( MovieInfo( "A Christmas Carol", "2009", "Science Fiction", "Jim_Carrey_in_A_Christmas_Carol_Wallpaper_2_800.jpg" ) );
    posters.push_back( MovieInfo( "Little Miss Sunshine", "2009", "Science Fiction", "Little_Miss_Sunshine_by_lakikaki.jpg" ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", "MOON_Poster_by_pepperberetta.jpg" ) );
    posters.push_back( MovieInfo( "Aliens", "2009", "Science Fiction", "Movie-Poster-Aliens[1].jpg" ) );
    posters.push_back( MovieInfo( "Liar Liar", "2009", "Science Fiction", "Movie-Poster-Liar-Liar.jpg" ) );
    posters.push_back( MovieInfo( "Nine", "2009", "Science Fiction", "Nine-movie-still-3.jpg" ) );
    posters.push_back( MovieInfo( "Terminator 2: Judgment Day", "2009", "Science Fiction", "Terminator-2-Judgment-Day-movie-poster.jpg" ) );
    posters.push_back( MovieInfo( "The Fountain", "2009", "Science Fiction", "The-Fountain-the-fountain-5530120-1280-1024.jpg" ) );
    posters.push_back( MovieInfo( "The Matrix: Reloaded", "2009", "Science Fiction", "The_Matrix_Reloaded_poster.jpg" ) );
    posters.push_back( MovieInfo( "True Lies", "2009", "Science Fiction", "True_Lies_lobby_card_Arnold_Schwarzenegger_Jamie_Lee_Curtis_James_Cameron_1994_TV_remake.jpg" ) );
    posters.push_back( MovieInfo( "Alice in Wonderland", "2009", "Science Fiction", "alice_in_wonderland_xlg.jpg" ) );
    posters.push_back( MovieInfo( "Alien 3", "2009", "Science Fiction", "alien3.jpg" ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", "avatar-poster-frensc.jpg" ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", "avatar-poster.jpg" ) );
    posters.push_back( MovieInfo( "Black Swan", "2009", "Science Fiction", "blackswan_poster.jpg" ) );
    posters.push_back( MovieInfo( "Close Encounters of the Third Kind", "2009", "Science Fiction", "close-encounters-of-the-third-kind-1-1024.jpg" ) );
    posters.push_back( MovieInfo( "Corpse Bride", "2009", "Science Fiction", "corpse-bride-6-1024.jpg" ) );
    posters.push_back( MovieInfo( "Green Hornet", "2009", "Science Fiction", "greenhornet.jpg" ) );
    posters.push_back( MovieInfo( "Moon", "2009", "Science Fiction", "mpamoonposter2b.jpg" ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", "mpayesmanposterb.jpg" ) );
    posters.push_back( MovieInfo( "Sweeney Todd", "2009", "Science Fiction", "sweeneytodd.jpg" ) );
    posters.push_back( MovieInfo( "Terminator: Salvation", "2009", "Science Fiction", "terminator-salvation-poster2.jpg" ) );
    posters.push_back( MovieInfo( "Terminator", "2009", "Science Fiction", "terminator_poster4.jpg" ) );
    posters.push_back( MovieInfo( "Terminator: Other", "2009", "Science Fiction", "terminator_salvation_21.jpg" ) );
    posters.push_back( MovieInfo( "The Mask", "2009", "Science Fiction", "the-mask-original.jpg" ) );
    posters.push_back( MovieInfo( "The Abyss", "2009", "Science Fiction", "the_abyss_1989.jpg" ) );
    posters.push_back( MovieInfo( "The Nightmare Before Christmas", "2009", "Science Fiction", "the_nightmare_before_christmas_poster.jpg" ) );
    posters.push_back( MovieInfo( "Titanic", "2009", "Science Fiction", "titanic_ver2_xlg.jpg" ) );
    posters.push_back( MovieInfo( "Alien", "2009", "Science Fiction", "vintage-alien-movie-poster.jpg" ) );
    posters.push_back( MovieInfo( "Batman", "2009", "Science Fiction", "viral-batman-poster.jpg" ) );
}

Vec2i TileCluster::chooseDimension()
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

void TileCluster::addTile( Vec2i position, Vec2i dim, float z, Vec3f color )
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
    Vec2i dim, topLeft, bottomRight;

    if ( branching ) {
        yellow = Rand::randInt( tiles.size() );
        dim = chooseDimension();
        topLeft = tiles[ yellow ].relativeCorner( dim, chooseOrientation() );
        bottomRight = topLeft + dim;
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
    gl::translate( Vec3f( 300.0f, 200.0f, 0.0f ) );
    gl::scale( Vec3f( 0.15f, 0.15f, 1.0f ) );

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

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

    gl::popModelView();
    gl::disableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();

    // size = seeds.size();
    // for ( int ee = 0; ee < size; ee++ ) {
    //     seeds[ee].draw();
    // }
}

};
