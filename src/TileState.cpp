#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/enable_shared_from_this.hpp"
#include "Tile.h"
#include "TileState.h"

using namespace ci;
using namespace boost;

// shared_ptr<TileState> TileState::update( Tile & tile )
// {
//     std::cout << "in regular tile state?  SHOULD NOT BE HERE" << this << std::endl;
//     return shared_from_this();
// }

shared_ptr<TileState> EnterTileState::update( Tile & tile )
{
    if ( !alpha.done() ) {
        tile.alpha = alpha.out();
        return shared_from_this();
    } else {
        std::cout << "change to bloom: " << this << std::endl;
        return shared_ptr<TileState>( new BloomTileState() );
    }
}

shared_ptr<TileState> BloomTileState::update( Tile & tile )
{
    bool full = false;
    if ( Rand::randFloat() < 0.02 ) {
        full = tile.branch();
    }

    // return shared_from_this();

    if ( full ) {
        std::cout << "change to leave: " << this << std::endl;
        return shared_ptr<TileState>( new LeaveTileState() );
    } else {
        return shared_from_this();
    }
}

shared_ptr<TileState> LeaveTileState::update( Tile & tile )
{
    if ( !alpha.done() ) {
        tile.alpha = alpha.out();
    }

    return shared_from_this();
}
