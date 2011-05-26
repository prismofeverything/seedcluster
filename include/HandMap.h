#pragma once
#include <map>
#include <vector>
#include "HandTracker.h"

namespace ix {

template <class Cursor>
class HandMap {
 public:
    HandMap() {};
    Cursor & get( const Hand & hand );
    void update( const HandTracker & tracker );
    void draw();
    void flush( const std::vector<Hand> & hands );

    void in( const Hand & hand );
    void out( const Hand & hand );
    void close( const Hand & hand );
    void open( const Hand & hand );
    void move( const Hand & hand );
    void drag( const Hand & hand );

    const static int smoothing = 10;
    std::map<float, Cursor> handmap;
};

template <class Cursor>
Cursor & HandMap<Cursor>::get( const Hand & hand )
{
    return handmap[ hand.hue ];
}

template <class Cursor>
void HandMap<Cursor>::update( const HandTracker & tracker )
{
    std::vector<float> outs;

    for ( typename std::map<float, Cursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        cursor->second.update();
        if ( cursor->second.isOut() ) {
            outs.push_back( cursor->first );
        }
    }

    for ( std::vector<float>::iterator oo = outs.begin(); oo != outs.end(); oo++ ) {
        handmap.erase( *oo );
    }

    flush( tracker.hands );
}

template <class Cursor>
void HandMap<Cursor>::draw()
{
    for ( typename std::map<float, Cursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        cursor->second.draw();
    }
}

template <class Cursor>
void HandMap<Cursor>::flush( const std::vector<Hand> & hands )
{
    std::vector<float> outs;
    for ( typename std::map<float, Cursor>::iterator cursor = handmap.begin(); cursor != handmap.end(); cursor++ ) {
        outs.push_back( cursor->first );
    }

    for ( std::vector<Hand>::const_iterator hand = hands.begin(); hand != hands.end(); hand++ ) {
        std::vector<float>::iterator found = std::find( outs.begin(), outs.end(), hand->hue );
        if ( found != outs.end() ) {
            outs.erase( found );
        }
    }

    for ( std::vector<float>::iterator gone = outs.begin(); gone < outs.end(); gone++ ) {
        if ( !handmap[ *gone ].goingOut ) {
            handmap.erase( *gone );
        }
    }
}

template <class Cursor>
void HandMap<Cursor>::in( const Hand & hand )
{
    Cursor cursor;
    cursor.in( hand, hand.center );
    handmap[ hand.hue ] = cursor;
}

template <class Cursor>
void HandMap<Cursor>::out( const Hand & hand )
{
    handmap[ hand.hue ].out( hand.smoothCenter( smoothing ) );
    // handmap.erase( hand.hue );
}

template <class Cursor>
void HandMap<Cursor>::close( const Hand & hand )
{
    handmap[ hand.hue ].close( hand.smoothCenter( smoothing ) );
}

template <class Cursor>
void HandMap<Cursor>::open( const Hand & hand )
{
    handmap[ hand.hue ].open( hand.smoothCenter( smoothing ) );
}

template <class Cursor>
void HandMap<Cursor>::move( const Hand & hand )
{
    handmap[ hand.hue ].move( hand.smoothCenter( smoothing ) );
}

template <class Cursor>
void HandMap<Cursor>::drag( const Hand & hand )
{
    handmap[ hand.hue ].drag( hand.smoothCenter( smoothing ) );
}

};
