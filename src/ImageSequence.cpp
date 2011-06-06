#include "ImageSequence.h"

ImageSequence::ImageSequence() : playheadPosition( 0 ), playheadFrameInc( 1 ), paused( false ), playing( true ), looping( true )
{
    
}

ImageSequence::~ImageSequence()
{
    textures.clear();
}

void ImageSequence::play() 
{
    paused = false;
    playing = true;
}

void ImageSequence::pause()
{
    paused = true;
    playing = false;
}

void ImageSequence::stop()
{
    playheadPosition = 0;
    playing = false;
    paused = false;
}

void ImageSequence::update()
{
    if( !paused && playing )
    {
        int newPosition = playheadPosition + playheadFrameInc;
        if( newPosition > totalFrames - 1 )
        {
            if( looping )
            {
                complete = false;
                playheadPosition = newPosition - totalFrames;
            } else {
                complete = true;
            }
            
        } else if( newPosition < 0 ) {
            if( looping )
            {
                complete = false;
                playheadPosition = totalFrames - abs( newPosition );
            } else {
                complete = true;
            }
            
        } else {
            complete = false;
            playheadPosition = newPosition;
        }
    }
}

void ImageSequence::setPlayheadPosition( int newPosition )
{
    playheadPosition = max( 0, min( newPosition, totalFrames ) );
}

void ImageSequence::createFromDir( string filePath )
{
    fs::path p( filePath );
    for ( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it )
    {
        if ( fs::is_regular_file( *it ) )
        {
            string fileName =  it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) )
            {
                textures.push_back( Texture( loadImage( filePath + fileName ) ) );
            }
        }
    }
    
    totalFrames = textures.size();
}