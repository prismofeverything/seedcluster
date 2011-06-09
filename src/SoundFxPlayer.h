#pragma once

#include "cinder/app/App.h"
#include "cinder/audio/Output.h"
#include "Resources.h"

using namespace ci::audio;

namespace ix
{
    class Player
    {
    protected:
        
        TrackRef hoverTrack;
        TrackRef handInTrack;
        TrackRef handOutTrack;
        
        void playSound( TrackRef track )
        {
            track->stop();
            track->setTime( 0 );
            track->play();
        }
        
    public:

        void init() 
        {
            hoverTrack = Output::addTrack( audio::load( loadResource( RES_HOVER_SOUND ) ), false );
            hoverTrack->setLooping( false );
            
            handInTrack = Output::addTrack( audio::load( loadResource( RES_HAND_ENTER_SOUND ) ), false );
            handInTrack->setLooping( false );
            
            handOutTrack = Output::addTrack( audio::load( loadResource( RES_HAND_LEAVE_SOUND ) ), false );
            handOutTrack->setLooping( false );
        }
        
        void hoverEnter() { playSound( hoverTrack ); }
        
        void handIn() { playSound( handInTrack ); }
        
        void handOut() { playSound( handOutTrack ); }
    };
    
    static Player SoundFXPlayer;
    
} // -- ix namespace