#pragma once
#include "cinder/Cinder.h"
#include "cinder/Rand.h"
#include "boost/statechart/event.hpp"
#include "boost/statechart/state_machine.hpp"
#include "boost/statechart/simple_state.hpp"
#include "boost/statechart/transition.hpp"
#include "Ease.h"

namespace sc = boost::statechart;

namespace ix {

class Tile;

struct TileUpdater
{
    virtual void updateTile( Tile & tile ) const = 0;
};

struct Enter : sc::event< Enter > {};
struct Bloom : sc::event< Bloom > {};
struct Leave : sc::event< Leave > {};

struct Active;
struct TileState : sc::state_machine< TileState, Active > {
    void updateTile( Tile & tile ) const {
        state_cast< const TileUpdater & >().updateTile( tile );
    }
};

struct Entering;
struct Blooming;
struct Leaving;

struct Active : TileUpdater, sc::simple_state< Active, TileState, Entering > {
    typedef sc::transition< Enter, Entering > reactions;
    virtual void updateTile( Tile & tile ) const {};
};

struct Entering : TileUpdater, sc::simple_state< Entering, Active > {
    typedef sc::transition< Bloom, Blooming > reactions;

    Entering();
    virtual void updateTile( Tile & tile ) const;

    Ease alpha;
};

struct Blooming : TileUpdater, sc::simple_state< Blooming, Active > {
    typedef sc::transition< Leave, Leaving > reactions;

    virtual void updateTile( Tile & tile ) const;
};

struct Leaving : TileUpdater, sc::simple_state< Leaving, Active > {
    Leaving();
    virtual void updateTile( Tile & tile ) const;

    Ease alpha;
};

