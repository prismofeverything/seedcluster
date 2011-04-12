#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/enable_shared_from_this.hpp"
#include "Tile.h"
#include "TileState.h"
#include "penner/Expo.h"


using namespace ci;
using namespace boost;

#define TAU 6.2831853071795862f

shared_ptr<TileState> TileState::update( Tile & tile )
{
    return shared_from_this();
}

shared_ptr<TileState> EnterTileState::update( Tile & tile )
{
    if ( tile.alpha < 0.9 ) {
        
		tile.setAlpha( Expo::easeOut( time, 0.0f, 0.91f, 140.0f ) );
		time += 1.0f;
		
		tile.setRotation( Expo::easeOut( time, TAU*0.5f, TAU, 140.0f ) );
		rot += 1.0f;
		
        return shared_from_this();
    } else {
        return shared_ptr<TileState>( new BloomTileState() );
    }
}

shared_ptr<TileState> BloomTileState::update( Tile & tile )
{
    bool full = false;
    if ( Rand::randFloat() < 0.02 ) {
        full = tile.branch();
    }

    return shared_from_this();

    // if ( full ) {
    //     return shared_ptr<TileState>( new LeaveTileState() );
    // } else {
    //     return shared_from_this();
    // }
}

shared_ptr<TileState> LeaveTileState::update( Tile & tile )
{
    if ( tile.alpha > 0.2f ) {
        tile.addAlpha( -0.01f );
    } 

    return shared_from_this();
}
