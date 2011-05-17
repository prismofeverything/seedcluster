#pragma once
#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/enable_shared_from_this.hpp"
#include "Ease.h"

class Tile;

class TileState : public boost::enable_shared_from_this<TileState> {
 public:
    TileState() {};
    virtual ~TileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile ) = 0;
};

class EnterTileState : public TileState {
 public:
    EnterTileState() : alpha( 0.0f, 0.9f, 50 ) {};
    virtual ~EnterTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );

    Ease alpha;
};

class BloomTileState : public TileState {
 public:
    BloomTileState() {};
    virtual ~BloomTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );
};

class LeaveTileState : public TileState {
 public:
    LeaveTileState() : alpha( 0.9f, 0.0f, 50 ) {};
    virtual ~LeaveTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );

    Ease alpha;
};

