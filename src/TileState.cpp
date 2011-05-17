#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/enable_shared_from_this.hpp"
#include "Tile.h"
#include "TileState.h"

using namespace ci;
using namespace boost;

namespace ix {

Entering::Entering()
    : alpha( 0.0f, 0.9f, 50 )
{
    
}

Leaving::Leaving()
    : alpha( 0.9f, 0.0f, 50 )
{
    
}

void Entering::updateTile( Tile & tile ) const 
{
    if ( !alpha.done() ) {
        tile.alpha = alpha.out();
    } else {
        post_event( Bloom() );
    }
}

void Blooming::updateTile( Tile & tile ) const 
{
    bool full = false;
    if ( Rand::randFloat() < 0.02 ) {
        full = tile.branch();
    }

    if ( full ) {
        post_event( Leave() );
    }
}

void Leaving::updateTile( Tile & tile ) const
{
    if ( !alpha.done() ) {
        tile.alpha = alpha.out();
    }
}

};
