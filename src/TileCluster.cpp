#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "TileCluster.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "Resources.h"
#include "Ease.h"
#include "SoundFxPlayer.h"
#include <iostream>
#include <algorithm>
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ix
{

TileCluster::TileCluster()
{
    orientations.push_back( Vec2i( 0, 1 ) );
    orientations.push_back( Vec2i( -1, 0 ) );
    orientations.push_back( Vec2i( 0, -1 ) );
    orientations.push_back( Vec2i( 1, 0 ) );
}

void TileCluster::setup()
{
    hoverTrack = audio::Output::addTrack( audio::load( loadResource( RES_HOVER_SOUND ) ), false );
    hoverTrack->setLooping( false );
    flipTrack = audio::Output::addTrack( audio::load( loadResource( RES_TILE_FLIP_SOUND ) ), false );
    flipTrack->setLooping( false );

    chosenSeed = seeds.end();
    hoverSeed = seeds.end();
    chosenTile = NULL;
    hoverTile = NULL;
    previousTile = NULL;

    tileOffset = Vec2f( 0, 0 );
    
    anchorDistance = 0;
    scaleDistance = 0;
    tileScale = targetScale = initialScale;
    scaleTriggered = false;

    setupShadows();
    setupPosters();
}

void TileCluster::playSound( audio::TrackRef & track )
{
    track->stop();
    track->setTime( 0 );
    track->play();
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

Tile & TileCluster::addTile( Vec2i position, TileDimension dim, float z, Vec3f color )
{
    Vertex v = add_vertex( tileGraph );
    tiles.push_back( Tile( this, tiles.size(), position, dim, z, color, choosePoster(), v ) );
    vertexmap[v] = &tiles.back();

    return tiles.back();
}

void TileCluster::mouseDown( Vec2i position, Vec2f vel, Vec3f color )
{
    // addTile( position, chooseDimension(), 0, color );
}

void TileCluster::clearSeeds()
{
    seeds.clear();
}

void TileCluster::handOver( Vec2i point )
{
    lens.set( point );
    lens.x = 640.0f - lens.x;
    lens -= Vec2f( 320.0f, 240.0f ) + ( tileOffset * translationFactor() );
    lens /= Vec2f( tileScale, tileScale );
    
    previousTile = hoverTile;
    std::vector<Tile>::iterator ht = std::find_if ( tiles.begin(), tiles.end(), TileContains( lens ) );
    
    if ( ht == tiles.end() ) {
        generateUnder( lens );
        ht = std::find_if ( tiles.begin(), tiles.end(), TileContains( lens ) );
        // if ( ht != tiles.end() ) playSound( flipTrack );
    }
    
    if( previousTile && previousTile != &( *ht ) ) {
        previousTile->unhover();
    }

    if ( ht != tiles.end() ) {
        hoverTile = &( *ht );
        previousTile = hoverTile;
        ht->hover();

        int yoyo = 0;
        Adjacency adjacent, adjacent_end;
        for ( tie(adjacent, adjacent_end) = adjacent_vertices( ht->vertex, tileGraph ); 
              adjacent != adjacent_end; adjacent++ ) {
            // vertexmap[*adjacent]->leaveTimer = 0;
            yoyo++;
        }

        if ( yoyo < 4 ) {
            TileDimension dim = chooseDimension();
            ci::Vec2i topLeft = ht->relativeCorner( dim.first, chooseOrientation() );
            generate( dim, topLeft );
        }
    }
}

void TileCluster::unhover()
{
    
}
    
void TileCluster::twoHandsIn( ci::Vec2i first, ci::Vec2i second )
{
    anchorDistance = first.distance( second );
    scaleDistance = anchorDistance;
}
    
void TileCluster::twoHandsMove( ci::Vec2i first, ci::Vec2i second )
{
    scaleDistance = (float) first.distance( second );
    if ( !scaleTriggered && scaleDistance - anchorDistance > 0 ) {
        scaleEase = Ease( tileScale, outScale, scaleDuration + 30 );
        scaleTriggered = true;
    } else if ( scaleTriggered && scaleDistance - anchorDistance < 0 ) {
        scaleEase = Ease( tileScale, initialScale, scaleDuration + 30 );
        scaleTriggered = false;
    }

    // targetScale += ( scaleDistance - anchorDistance ) * 0.00005;
    // targetScale =  max( 0.05f, min( targetScale, 1.0f ) );
    // tileScale = targetScale;
}
    
void TileCluster::secondHandOut()
{
    //tileScale = tileScale + targetScale;
    //targetScale = 0;
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

void TileCluster::generateUnder( ci::Vec2f center )
{
    TileDimension dim = chooseDimension();
    Vec2i topLeft = center / Vec2i( Tile::atomWidth, Tile::atomHeight ) - (dim.first / 2);
    generate( dim, topLeft );
}

void TileCluster::generate( TileDimension dim, ci::Vec2i topLeft ) {
    Vec2i bottomRight = topLeft + dim.first;
    std::vector<int> adjacent;

    bool tileFits = true;
    Collision collision = Unrelated;
    int tt; 
    for ( tt = 0; tileFits && tt < tiles.size(); tt++ ) {
        collision = tiles[tt].collidesWith( topLeft, bottomRight );
        tileFits = ( collision == Unrelated || collision == Adjacent );
        if ( collision == Adjacent ) {
            adjacent.push_back( tt );
        }
    }

    if ( tileFits ) {
        Vec3f newColor = tiles.size() > 0 ? tiles[0].color : Vec3f( Rand::randFloat(), Rand::randFloat(), 0 );
        newColor[2] = Rand::randFloat();
        Tile & tile = addTile( topLeft, dim, 0, newColor );
        for ( tt = 0; tt < adjacent.size(); tt++ ) {
            add_edge( tile.vertex, tiles[adjacent[tt]].vertex, tileGraph );
        }
    }
}

void TileCluster::update()
{
    // int size = tiles.size();
    // if ( size > 0 ) {
    //     std::cout << tiles.front().alpha << " --- " << tiles.size() << std::endl;
    // }

    //     if ( tiles.front().state == Entering ) {
    //         std::cout << "Entering" << std::endl;
    //     } else if ( tiles.front().state == Leaving ) {
    //         std::cout << "Leaving" << std::endl;
    //     } else if ( tiles.front().state == FirstHover ) {
    //         std::cout << "First Hover" << std::endl;
    //     } else if ( tiles.front().state == Blooming ) {
    //         std::cout << "Blooming" << std::endl;
    //     } else if ( tiles.front().state == Hovering ) {
    //         std::cout << "Hovering" << std::endl;
    //     } else if ( tiles.front().state == Unhover ) {
    //         std::cout << "Unhovering" << std::endl;
    //     } else if ( tiles.front().state == Nixed ) {
    //         std::cout << "Nixed" << std::endl;
    //     } else if ( tiles.front().state == Init ) {
    //         std::cout << "Init" << std::endl;
    //     }
    // }

    if ( !scaleEase.done() ) {
        if ( scaleTriggered ) {
            tileScale = scaleEase.out();
        } else {
            tileScale = scaleEase.out();
        }
    }

    int age = tiles.size();
    for ( std::vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
        tile->update( age > ageThreshold );
        if ( tile->state == Nixed ) {
            // remove_vertex( tile->vertex, tileGraph );
            vertexmap.erase( tile->vertex );
        }
        age--;
    }

    std::vector<Tile>::iterator cutoff = remove_if( tiles.begin(), tiles.end(), isNixed() );
    tiles.erase( cutoff, tiles.end() );
}

void TileCluster::draw()
{
    drawTiles( false );
    drawSeeds();
}

void TileCluster::drawSeeds()
{
    int size = seeds.size();
    for ( int ee = 0; ee < size; ee++ ) {
        seeds[ee].draw();
    }
}

void TileCluster::drawTiles( bool posterMode )
{
    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::pushModelView();

        gl::translate( tileOffset * translationFactor() );
        gl::scale( Vec3f( tileScale, tileScale, tileScale ) );
        
        if ( posterMode ) {
            for ( std::vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
                tile->drawPoster();
            }
        } else {
            for ( std::vector<Tile>::iterator tile = tiles.begin(); tile != tiles.end(); tile++ ) {
                tile->draw();
            }
        }
   
    gl::popModelView();
    gl::disableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();
}

void TileCluster::setupPosters()
{
    gl::Texture::Format format;
    format.enableMipmapping( true );
    format.setMinFilter( GL_LINEAR_MIPMAP_LINEAR );
    format.setMagFilter( GL_LINEAR_MIPMAP_LINEAR );

    posters.push_back( MovieInfo( "The Matrix", "2001", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Matrix_Reloaded_Wallpaper3 ) ), format ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_04 ) ), format ) ) );
    posters.push_back( MovieInfo( "Eternal Sunshine of the Spotless Mind", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_1166952147_1024x768_movie_stills_of_eternal_sunshine_of_the_spotless_mind ) ), format ) ) );
    posters.push_back( MovieInfo( "Sleepy Hollow", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_1287369723_1024x768_sleepy_hollow_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "2001: Space Odyssey", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_2001_space_odyssey_1280x1048 ) ), format ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_2007_sunshine_2 ) ), format ) ) );
    posters.push_back( MovieInfo( "2010: Contact", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_2010_movie_poster_year_we_make_contact ) ), format ) ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_936full_solaris_poster_1 ) ), format ) ) );
    posters.push_back( MovieInfo( "Solaris", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_936full_solaris_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "A Christmas Carol", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Jim_Carrey_in_A_Christmas_Carol_Wallpaper_2_800 ) ), format ) ) );
    posters.push_back( MovieInfo( "Little Miss Sunshine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Little_Miss_Sunshine_by_lakikaki ) ), format ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_MOON_Poster_by_pepperberetta ) ), format ) ) );
    posters.push_back( MovieInfo( "Aliens", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Movie_Poster_Aliens ) ), format ) ) );
    posters.push_back( MovieInfo( "Liar Liar", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Movie_Poster_Liar_Liar ) ), format ) ) );
    posters.push_back( MovieInfo( "Nine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Nine_movie_still_3 ) ), format ) ) );
    posters.push_back( MovieInfo( "Terminator 2: Judgment Day", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_Terminator_2_Judgment_Day_movie_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "The Fountain", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_The_Fountain_the_fountain_5530120_1280_1024 ) ), format ) ) );
    posters.push_back( MovieInfo( "The Matrix: Reloaded", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_The_Matrix_Reloaded_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "True Lies", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_True_Lies_lobby_card_Arnold_Schwarzenegger_Jamie_Lee_Curtis_James_Cameron_1994_TV_remake ) ), format ) ) );
    posters.push_back( MovieInfo( "Alice in Wonderland", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_alice_in_wonderland_xlg ) ), format ) ) );
    posters.push_back( MovieInfo( "Alien 3", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_alien3 ) ), format ) ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_avatar_poster_frensc ) ), format ) ) );
    posters.push_back( MovieInfo( "Avatar", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_avatar_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "Black Swan", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_blackswan_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "Close Encounters of the Third Kind", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_close_encounters_of_the_third_kind_1_1024 ) ), format ) ) );
    posters.push_back( MovieInfo( "Corpse Bride", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_corpse_bride_6_1024 ) ), format ) ) );
    posters.push_back( MovieInfo( "Green Hornet", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_greenhornet ) ), format ) ) );
    posters.push_back( MovieInfo( "Moon", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_mpamoonposter2b ) ), format ) ) );
    posters.push_back( MovieInfo( "Sunshine", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_mpayesmanposterb ) ), format ) ) );
    posters.push_back( MovieInfo( "Sweeney Todd", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_sweeneytodd ) ), format ) ) );
    posters.push_back( MovieInfo( "Terminator: Salvation", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_terminator_salvation_poster2 ) ), format ) ) );
    posters.push_back( MovieInfo( "Terminator", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_terminator_poster4 ) ), format ) ) );
    posters.push_back( MovieInfo( "Terminator: Other", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_terminator_salvation_21 ) ), format ) ) );
    posters.push_back( MovieInfo( "The Mask", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_the_mask_original ) ), format ) ) );
    posters.push_back( MovieInfo( "The Abyss", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_the_abyss_1989 ) ), format ) ) );
    posters.push_back( MovieInfo( "The Nightmare Before Christmas", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_the_nightmare_before_christmas_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "Titanic", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_titanic_ver2_xlg ) ), format ) ) );
    posters.push_back( MovieInfo( "Alien", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_vintage_alien_movie_poster ) ), format ) ) );
    posters.push_back( MovieInfo( "Batman", "2009", "Science Fiction", gl::Texture( loadImage( loadResource( RES_viral_batman_poster ) ), format ) ) );
}

void TileCluster::setupShadows()
{
    tileDimensions.push_back( TileDimension( Vec2i( 5, 3 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_1200x810 ) ) ) ) );
    tileDimensions.push_back( TileDimension( Vec2i( 2, 2 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_480x540 ) ) ) ) );
    tileDimensions.push_back( TileDimension( Vec2i( 2, 3 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_480x810 ) ) ) ) );
    tileDimensions.push_back( TileDimension( Vec2i( 3, 4 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_720x1080 ) ) ) ) );
    tileDimensions.push_back( TileDimension( Vec2i( 3, 2 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_720x540 ) ) ) ) );
    tileDimensions.push_back( TileDimension( Vec2i( 4, 3 ), gl::Texture( loadImage( loadResource( RES_LONGSHADOW_960x810 ) ) ) ) );
}

};
