#pragma once

#include "cinder/app/App.h"
#include "cinder/audio/Output.h"
#include "Resources.h"

using namespace ci::audio;

namespace ix
{

    static TrackRef hoverTrack;
    static TrackRef handInTrack;
    static TrackRef handOutTrack;
    static TrackRef tileFlipTrack;
    
    class SoundFXPlayer
    {
    private:
        
        
        SoundFXPlayer() {};
        
        static void playSound( TrackRef track )
        {
            track->stop();
            track->setTime( 0 );
            track->play();
        }
        
    public:
        
        static void init()
        {
            hoverTrack = Output::addTrack( audio::load( loadResource( RES_HOVER_SOUND ) ), false );
            hoverTrack->setLooping( false );
            
            handInTrack = Output::addTrack( audio::load( loadResource( RES_HAND_ENTER_SOUND ) ), false );
            handInTrack->setLooping( false );
            
            handOutTrack = Output::addTrack( audio::load( loadResource( RES_HAND_LEAVE_SOUND ) ), false );
            handOutTrack->setLooping( false );
            
            tileFlipTrack = Output::addTrack( audio::load( loadResource( RES_TILE_FLIP_SOUND ) ), false );
            tileFlipTrack->setLooping( false );
        }
        
        static void hoverEnter() { playSound( hoverTrack ); }
        
        static void handIn() { playSound( handInTrack ); }
        
        static void handOut() { playSound( handOutTrack ); }
        
        static void tileFlip() { playSound( tileFlipTrack ); }
    };
    
} // -- ix namespace