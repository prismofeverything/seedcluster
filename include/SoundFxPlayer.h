#pragma once

#include "cinder/app/App.h"
#include "cinder/audio/Output.h"
#include "Resources.h"

// -- not using this class
// -- causing a crash when used as such from more than one object instance 
// -- ix::SoundFXPlayer::hoverEnter();
using namespace ci;

namespace ix
{
    static audio::TrackRef hoverTrack;
    static audio::TrackRef handInTrack;
    static audio::TrackRef handOutTrack;
    static audio::TrackRef tileFlipTrack;
    
    class SoundFXPlayer
    {
        
    private:
        
        SoundFXPlayer() {};
        
        static void playSound( audio::TrackRef track )
        {
            track->stop();
            track->setTime( 0 );
            track->play();
        }
        
    public:
        
        static void init()
        {
            hoverTrack = audio::Output::addTrack( audio::load( loadResource( RES_HOVER_SOUND ) ), false );
            hoverTrack->setLooping( false );
            
            handInTrack = audio::Output::addTrack( audio::load( loadResource( RES_HAND_ENTER_SOUND ) ), false );
            handInTrack->setLooping( false );
            
            handOutTrack = audio::Output::addTrack( audio::load( loadResource( RES_HAND_LEAVE_SOUND ) ), false );
            handOutTrack->setLooping( false );
            
            tileFlipTrack = audio::Output::addTrack( audio::load( loadResource( RES_TILE_FLIP_SOUND ) ), false );
            tileFlipTrack->setLooping( false );
        }
        
        static void hoverEnter() { playSound( hoverTrack ); }
        static void handIn() { playSound( handInTrack ); }
        static void handOut() { playSound( handOutTrack ); }
        static void tileFlip() { playSound( tileFlipTrack ); }
    };
    
} // -- ix namespace
