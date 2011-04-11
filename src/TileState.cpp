#include "cinder/Cinder.h"
#include "boost/enable_shared_from_this.hpp"
#include "Tile.h"
#include "TileState.h"

using namespace boost;

shared_ptr<TileState> TileState::update( Tile & tile )
{
    return shared_from_this();
}

shared_ptr<TileState> EnterTileState::update( Tile & tile )
{
    if ( tile.alpha < 0.9 ) {
        tile.addAlpha( 0.01f );
        return shared_from_this();
    } else {
        return shared_ptr<TileState>( new LeaveTileState() );
    }
}

shared_ptr<TileState> LeaveTileState::update( Tile & tile )
{
    if ( tile.alpha > 0.1f ) {
        tile.addAlpha( -0.01f );
        return shared_from_this();
    } else {
        return shared_ptr<TileState>( new EnterTileState() );
    }
}
