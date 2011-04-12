#pragma once
#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/enable_shared_from_this.hpp"

class Tile;

class TileState : public boost::enable_shared_from_this<TileState> {
 public:
    TileState() {};
    virtual ~TileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );
};

class EnterTileState : public TileState {
 public:
    EnterTileState() : time( 0.0f ) {};
    virtual ~EnterTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );
	float time;
};

class BloomTileState : public TileState {
 public:
    BloomTileState() {};
    virtual ~BloomTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );
};

class LeaveTileState : public TileState {
 public:
    LeaveTileState() {};
    virtual ~LeaveTileState() {};
    virtual boost::shared_ptr<TileState> update( Tile & tile );
};

