#include <iostream>
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "cinder/Vector.h"
#include "cinder/Matrix.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/CinderMath.h"
#include "cinder/Cinder.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "Kinect.h"
#include "Ease.h"
#include "HandTracker.h"
#include "HandCursor.h"
#include "HandMap.h"
#include "PosterCursor.h"
#include "TileCluster.h"
#include "Resources.h"
#include "ImageSequence.h"
#include "ImageSequenceCursor.h"
#include "cinder/audio/Output.h"
#include "SoundFxPlayer.h"
//#include "cinder/qtime/MovieWriter.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Particle {
public:
    Particle( Vec2f aPosition )
    : mPosition( aPosition ), mLastPosition( aPosition ), mVelocity( Vec2f::zero() ), mZ( 0 )
    {}
    
    Vec2f mPosition, mVelocity, mLastPosition;
    Vec2i centering;
    float mZ;
};

class SeedClusterApp : public AppBasic, public ix::HandListener {
  public:
    void prepareSettings( Settings *settings );
    void setup();
    void update();
    void draw();

    Vec3f randomVec3f();
    Color randomColor();
    Matrix44f randomMatrix44f( float scale = 1.0f, float offset = 0.0f );
    void setColor( Vec3f color, float alpha );

    void setupRectangle();
    void setupParticles();
    void setupLighting();
    void setupMovieWriter();
    void updateCamera();
    void updateParticles();
    void drawMat( cv::Mat & mat );
    void drawCursor( cv::Point, float radius, float alpha );
    void drawRawHands();
    void drawSmoothHands();
    void drawField();
    void drawMovieFrame();
    void drawParticles();
    void drawRectangle();

    void keyDown( KeyEvent event ); 
    void keyUp( KeyEvent event );   
    void mouseDown( MouseEvent event ); 
    void mouseUp( MouseEvent event );   
    void mouseMove( MouseEvent event ); 
    void mouseDrag( MouseEvent event ); 

    void handIn( const ix::Hand & hand );
    void handMove( const ix::Hand & hand );
    void handOut( const ix::Hand & hand );
    void handClose( const ix::Hand & hand );
    void handDrag( const ix::Hand & hand );
    void handOpen( const ix::Hand & hand );

    void secondHandIn( const ix::Hand & in, const ix::Hand & other );
    void secondHandOut( const ix::Hand & out, const ix::Hand & other );
    void firstHandClose( const ix::Hand & close, const ix::Hand & other );
    void firstHandOpen( const ix::Hand & open, const ix::Hand & other );
    void secondHandClose( const ix::Hand & close, const ix::Hand & other );
    void secondHandOpen( const ix::Hand & open, const ix::Hand & other );
    void openHandsMove( const ix::Hand & first, const ix::Hand & second );
    void mixedHandsMove( const ix::Hand & open, const ix::Hand & close );
    void closedHandsMove( const ix::Hand & first, const ix::Hand & second );
    
    void scaleScene( const ix::Hand & first, const ix::Hand & second );

    ix::TileCluster cluster;
    Vec3f bloomColor;

    // viewport
    CameraPersp camera;
    Quatf rotation;
    Vec3f eye, towards, up;
    Vec3f background;
    Vec3f changeColor;
    float fovea;
    float near;
    float far;
    gl::Texture backgroundTexture;
    gl::Texture cursorTexture;

    int width;
    int height;

    int cannyLowerThreshold;
    int cannyUpperThreshold;
    //qtime::MovieWriter movieWriter;
    params::InterfaceGl params;
    ix::PointDistance distance;

    Vec3f defaultBackground;
    Vec3f oneBackground;
    Vec3f manyBackground;
    Ease hueEase;
    Ease saturationEase;
    Ease brightnessEase;

    // opencv
    cv::Mat depth;

    // kinect
    Kinect kinect;
    bool kinectEnabled;
    gl::Texture depthTexture;
    ci::Surface depthSurface;
    std::shared_ptr<uint16_t> kinectDepth;
    float kinectTilt, kinectScale;
    float XOff, mYOff;
    int kinectWidth, kinectHeight;
    Vec3f kinectColor;

    ix::HandTracker tracker;
    ix::HandMap<ix::ImageSequenceCursor> handmap;
    std::vector<float> hues;
    ci::Vec2f closePoint;
    ci::Vec2f zoomPoint;
    float zoomAnchor;
    float bgx, bgy;
    
    // particles
    bool particleOffscreen( const Vec2f &v );
    float particleInertia, particleSpeed;
    static const int numberOfParticles = 40;
    
    Perlin perlin;
    list<Particle> particles;
    float animationCounter;
    float rotationCounter;

    // input
    char key;
    bool keyIsDown;
    bool mouseIsDown;
    Vec2i mousePosition;
    Vec2f mouseVelocity;
    Vec2i centering;
    Vec2f shiftOffset;

    // modes
    bool innardsMode;
    bool rectangleMode;
    bool seedMode;
    bool greenMode;
    bool tileMode;
    bool posterMode;
    cv::RotatedRect rectangle;
    ci::Vec3f rectangleColor;
    float rectangleAlpha;
    float rectangleFactor;
    Ease rectangleHoverEase;
    
    // graphics
    gl::Texture bgImage;
    
    // cursor sequence
    void setupCursorSequence();
    ix::ImageSequence cursorImageSequence;
};

Vec3f SeedClusterApp::randomVec3f()
{
    return Vec3f( Rand::randFloat(), Rand::randFloat(), Rand::randFloat() );
}

Matrix44f SeedClusterApp::randomMatrix44f( float scale, float offset )
{
    return Matrix44f( Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset,
                      Rand::randFloat() * scale + offset
    );
}

Color SeedClusterApp::randomColor()
{
    return Color( CM_HSV, randomVec3f() );
}

void SeedClusterApp::setColor( ci::Vec3f color, float alpha )
{
    Color hsv( CM_HSV, color );
    glColor4f( hsv.r, hsv.g, hsv.b, alpha );
}

void SeedClusterApp::prepareSettings( Settings *settings )
{
    Rand::randomize();

    width = 1920;
    height = 1080;
    settings->setWindowSize( width, height );
    settings->setFrameRate( 60.0f );
    fovea = 75.0f;
    near = 5.0f;
    far = 50000.0f;
    centering = Vec2i( -1, -1 );
}

void SeedClusterApp::updateCamera()
{
    camera.setEyePoint( eye );
    camera.lookAt( towards );
}

void SeedClusterApp::setupCursorSequence()
{
    // -- load up the images for the sequence
    vector<Texture> textures;
    string compareString = "Hand_";
    string filePath = getResourcePath() + "/";
    
    fs::path p( filePath );
    
    for ( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it )
    {
        if ( fs::is_regular_file( *it ) )
        {
            string fileName = it->path().filename().string();
            if( !( fileName.compare( ".DS_Store" ) == 0 ) )
            {
                if( fileName.compare( compareString ) == 1 )
                {
                    textures.push_back( gl::Texture( loadImage( filePath + fileName ) ) );
                }
            }
        }
    }
    
    handmap.cursorTextures = textures;
}

void SeedClusterApp::setupParticles()
{
    perlin.setSeed( clock() );
    animationCounter = 0;
    for( int s = 0; s < numberOfParticles; ++s ) {
        particles.push_back( Particle( Vec2f( -100.0f, 0.0f ) ));
    }
    
    particleInertia = 0.9f;
    particleSpeed = 0.03f;
}

void SeedClusterApp::setupRectangle()
{
    rectangle = cv::RotatedRect( cv::Point( -320, -240 ), cv::Size( 124, 80 ), 0 );
    rectangleColor = Vec3f( 0.2, 0.7, 0.7 );
    rectangleAlpha = 0.6f;
    rectangleFactor = 1.0f;
}

void SeedClusterApp::setup()
{
    ix::SoundFXPlayer::init();
    
    tracker.registerListener( this );

    bgImage = gl::Texture( loadImage( loadResource( RES_BG_IMAGE ) )  );
    
    shiftOffset = Vec2f( 0, 0 );
    
	setupParticles();
    setupRectangle();
	
    cannyLowerThreshold = 0;
    cannyUpperThreshold = 0;

    seedMode = false;
    innardsMode = false;
    rectangleMode = false;
    greenMode = false;
    tileMode = true;
    posterMode = false;

    params = params::InterfaceGl( "seedcluster", Vec2i( 200, 180 ) );
    params.addParam( "canny lower threshold", &cannyLowerThreshold, "min=0 max=250 step=1" );
    params.addParam( "canny upper threshold", &cannyUpperThreshold, "min=0 max=250 step=1" );
    
    defaultBackground = Vec3f( 0.0f, 0.0f, 0.9f );
    oneBackground = Vec3f( 0.0f, 0.1f, 0.65f );
    manyBackground = Vec3f( 0.0f, 0.1f, 0.95f );
    background = defaultBackground;
    bloomColor = Vec3f( Rand::randFloat(), 0.4f, 1.0f );

    backgroundTexture = gl::Texture( loadImage( loadResource( RES_BACKGROUND ) ) );
    cursorTexture = gl::Texture( loadImage( loadResource( RES_CURSOR ) ) );

    mouseIsDown = false;
    keyIsDown = false;

    rotation.w = 0.0f;
    eye = Vec3f( 0.0f, 0.0f, 500.0f );
    towards = Vec3f( 0.0f, 0.0f, 0.0f );
    up = Vec3f::yAxis();
    camera.setPerspective( fovea, getWindowAspectRatio(), near, far );

    for ( int hue = 0; hue < 20; hue++ ) {
        hues.push_back( Rand::randFloat() );
    }

    kinectEnabled = Kinect::getNumDevices() > 0;
    if ( kinectEnabled ) {
        kinectTilt = 5.0f;
        kinect = Kinect( Kinect::Device() );
        kinectWidth = 640;
        kinectHeight = 480;
        depthTexture = gl::Texture( kinectWidth, kinectHeight );
        kinectColor = Vec3f( 0.0f, 0.0f, 1.0f );
    }

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    cluster.setup();
    handmap.setup();
    setupCursorSequence();
}

void SeedClusterApp::setupMovieWriter()
{
    /*std::string path = getSaveFilePath();
    if ( !path.empty() ) {
        qtime::MovieWriter::Format format;
        if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadResource( RES_WHIRLPOOL ) ) ) ) {
            movieWriter = qtime::MovieWriter( path, getWindowWidth(), getWindowHeight(), format );
        }
    }*/
}

void SeedClusterApp::keyDown( KeyEvent event )
{
    key = event.getChar();
    keyIsDown = true;

    if ( key == 68 || key == 100 ) { // 'd'
        rectangleMode = !rectangleMode;
    } else if ( key == 79 || key == 111 ) { // 'o'
        innardsMode = !innardsMode;
        cluster.clearSeeds();
    } else if ( key == 80 || key == 112 ) { // 'p'
        posterMode = !posterMode;
    } else if ( key == 83 || key == 115 ) { // 's'
        seedMode = !seedMode;
    } else if ( key == 84 || key == 116 ) { // 't'
        tileMode = !tileMode;
    } else if ( key == 71 || key == 103 ) { // 'g'
        greenMode = !greenMode;
    }
}

void SeedClusterApp::keyUp( KeyEvent event )
{
    keyIsDown = false;
}

void SeedClusterApp::mouseDown( MouseEvent event )
{
    mouseIsDown = true;
    bloomColor[2] = Rand::randFloat();
    mousePosition = event.getPos();
    Vec2i gridPosition = Vec2i(((float) mousePosition[0]) / ix::Tile::atomWidth, ((float) mousePosition[1]) / ix::Tile::atomHeight);

    cluster.mouseDown( gridPosition, mouseVelocity, bloomColor );
}

void SeedClusterApp::mouseUp( MouseEvent event )
{
    mouseIsDown = false;
}

void SeedClusterApp::mouseMove( MouseEvent event )
{
    mouseVelocity = event.getPos() - mousePosition;
    mousePosition = event.getPos();
}

void SeedClusterApp::mouseDrag( MouseEvent event )
{
    mouseMove( event );
    // system->mouseImpact( mousePosition, mouseVelocity, changeColor );
}

void SeedClusterApp::handIn( const ix::Hand & hand )
{
    //std::cout << "hand in - " << hand.hue << std::endl;
    handmap.in( hand );

    cv::Point p = hand.smoothCenter( 20 );
    cluster.handOver( Vec2f( p.x, p.y ) );
    Output::play( audio::load( loadResource( RES_HAND_ENTER_SOUND ) ) );
}

void SeedClusterApp::handOut( const ix::Hand & hand )
{
    //std::cout << "hand out - " << hand.hue << std::endl;
    handmap.out( hand );

    cluster.releaseSeed();
    
    Output::play( audio::load( loadResource( RES_HAND_LEAVE_SOUND ) ) );
}

void SeedClusterApp::handMove( const ix::Hand & hand )
{
    handmap.move( hand );
    cluster.tileOffset += handmap.get( hand ).shift;
    
    cv::Point p = hand.smoothCenter( 20 );
    
    cluster.handOver( Vec2f( p.x, p.y ) );

    if ( rectangle.boundingRect().contains( hand.center ) ) {
        rectangleHoverEase = Ease( rectangleFactor, 1.2f, 30 );
    } else {
        rectangleHoverEase = Ease( rectangleFactor, 1.0f, 30 );
    }
}

void SeedClusterApp::handClose( const ix::Hand & hand )
{
    //std::cout << "hand close - " << hand.hue << std::endl;
    handmap.close( hand );
    cluster.tileOffset += handmap.get( hand ).shift;

    closePoint = Vec2i( hand.center.x, hand.center.y );
    cv::Point p = hand.smoothCenter( 20 );
    
    cluster.handOver( Vec2f( p.x, p.y ) );

    if ( cluster.chooseSeed( closePoint ) ) {
        
    } else {
        cluster.plantSeed( closePoint, Vec3f( hand.hue, Rand::randFloat() * 0.3 + 0.6, Rand::randFloat() * 0.4 + 0.3 ) );
    }
}

void SeedClusterApp::handOpen( const ix::Hand & hand )
{
    //std::cout << "hand open - " << hand.hue << std::endl;
    handmap.open( hand );
    cluster.tileOffset += handmap.get( hand ).shift;

    cv::Point p = hand.smoothCenter( 20 );
    cluster.handOver( Vec2f( p.x, p.y ) );
    
    cluster.releaseSeed();
}

void SeedClusterApp::handDrag( const ix::Hand & hand )
{
    handmap.drag( hand );
    cluster.tileOffset += handmap.get( hand ).shift;

    cv::Point average = hand.smoothCenter( 10 );
    cluster.handOver( Vec2f( average.x, average.y ) );
    
    Vec2f smooth( average.x, average.y );

    if ( cluster.isSeedChosen() ) 
    {
        cluster.chosenSeed->seek( smooth );
    }
}

void SeedClusterApp::secondHandIn( const ix::Hand & in, const ix::Hand & other ) 
{
    handmap.in( in );
    
    cv::Point p1 = in.smoothCenter( 10 );
    Vec2i v1 = Vec2i( p1.x, p1.y );
    
    cv::Point p2 = other.smoothCenter( 10 );
    Vec2i v2 = Vec2i( p2.x, p2.y );
    
    cluster.twoHandsIn( v1, v2 );
}

void SeedClusterApp::secondHandOut( const ix::Hand & out, const ix::Hand & other ) 
{
    //std::cout << "second hand out - " << out.hue << std::endl;
    handmap.out( out );
}

void SeedClusterApp::firstHandClose( const ix::Hand & close, const ix::Hand & other ) 
{
    //std::cout << "first hand close - " << close.hue << std::endl;

    handClose( close );
}

void SeedClusterApp::firstHandOpen( const ix::Hand & open, const ix::Hand & other ) 
{
    //std::cout << "first hand open - " << open.hue << std::endl;
    handOpen( open );
}

void SeedClusterApp::secondHandClose( const ix::Hand & close, const ix::Hand & other )
{
    //std::cout << "second hand close - " << close.hue << std::endl;
    handmap.close( close );

    closePoint = Vec2i( other.center.x, other.center.y );
    zoomPoint = Vec2i( close.center.x, close.center.y );

    zoomAnchor = sqrt( distance( close.center, other.center ) );
}

void SeedClusterApp::secondHandOpen( const ix::Hand & open, const ix::Hand & other ) 
{
    //std::cout << "second hand open - " << open.hue << std::endl;

    //handOpen( open );
}

void SeedClusterApp::openHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{
    scaleScene( first, second );
}

void SeedClusterApp::mixedHandsMove( const ix::Hand & open, const ix::Hand & close ) 
{
    scaleScene( open, close );
}

void SeedClusterApp::closedHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{
    scaleScene( first, second );
}

void SeedClusterApp::scaleScene( const ix::Hand & first, const ix::Hand & second )
{
    handmap.move( first );
    handmap.move( second );
    
    cv::Point p1 = first.smoothCenter( 10 );
    Vec2i v1 = Vec2i( p1.x, p1.y );
    
    cv::Point p2 = second.smoothCenter( 10 );
    Vec2i v2 = Vec2i( p2.x, p2.y );
    
    cluster.twoHandsMove( v1, v2 );
    
    cluster.tileOffset += ( handmap.get( first ).shift ) + ( handmap.get( second ).shift );
}

void SeedClusterApp::setupLighting()
{
    // enable lighting 
    glEnable( GL_LIGHTING );
    glEnable( GL_COLOR_MATERIAL );
    glEnable( GL_LIGHT0 );
    glShadeModel( GL_SMOOTH );
    
    // Create light components
    GLfloat ambientLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
    
    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
}

void SeedClusterApp::update()
{
    if ( kinectEnabled ) {
        if( kinect.checkNewDepthFrame() ) {
            ImageSourceRef depthImage = kinect.getDepthImage();
            depthSurface = depthImage;
            depthTexture = depthImage;
            kinectDepth = kinect.getDepthData();
            depth = toOcv( Channel8u( depthSurface ) );
            tracker.detectHands( depth ); // , 150, 280 );
        }

        if( kinectTilt != kinect.getTilt() ) {
            kinect.setTilt( kinectTilt );
        }
    }

    if ( !hueEase.done() ) {
        background[0] = hueEase.out();
    }
    if ( !saturationEase.done() ) {
        background[1] = saturationEase.out();
    }
    if ( !hueEase.done() ) {
        background[2] = brightnessEase.out();
    }
    if ( !rectangleHoverEase.done() ) {
        rectangleFactor = rectangleHoverEase.out();
    }

    // eye[2] += eye[2] * 0.0035f;
    // towards[0] += 1.0f;
    updateCamera();
    updateParticles();

    gl::setMatrices( camera );
    gl::rotate( rotation );

    cluster.update();
    handmap.update( tracker );
}

void SeedClusterApp::updateParticles()
{
    animationCounter += 10.0f;

    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt )
        partIt->mLastPosition = partIt->mPosition;
    
    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt ) {
        Vec3f deriv = perlin.dfBm( Vec3f( partIt->mPosition.x, partIt->mPosition.y, animationCounter ) * 0.001f );
        partIt->mZ = deriv.z;
        Vec2f deriv2( deriv.x, deriv.y+.3 );
        deriv2.normalize();
        partIt->mVelocity += deriv2 * particleSpeed;
    }
    
    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt )
        partIt->mPosition += partIt->mVelocity;
    
    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt )
        partIt->mVelocity *= particleInertia;
    
    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt ) {
        if( particleOffscreen( partIt->mPosition ) )
            *partIt = Particle( Vec2f( Rand::randFloat( 1000.0f ), Rand::randFloat( 480.0f ) ) );
    }
}

bool SeedClusterApp::particleOffscreen( const Vec2f &v )
{
    return ( ( v.x < 0.0f ) || ( v.x > 640.0f ) || ( v.y < 0 ) || ( v.y > 480) );
}

void SeedClusterApp::drawMat( cv::Mat & mat ) 
{
    gl::Texture texture( fromOcv( mat ) );
    gl::draw( texture );
}

void SeedClusterApp::drawRawHands()
{
    for ( std::vector<ix::Hand>::iterator hand = tracker.hands.begin(); hand < tracker.hands.end(); hand++ ) {
        if ( hand->isHand ) {
            setColor( Vec3f( hand->hue, 0.5f, 0.7f ), 0.8f );
            gl::drawSolidCircle( ci::Vec2f( hand->center.x, hand->center.y ), 20.0f );

            setColor( Vec3f( hand->hue, 0.5f, 1.0f ), 0.8f );
            for( vector<cv::Point2i>::iterator fingertip = hand->fingertips.begin(); fingertip != hand->fingertips.end(); fingertip++ ) {
                gl::drawSolidCircle( ci::Vec2f( fingertip->x, fingertip->y ), 10.0f );
                gl::drawLine( ci::Vec2f( hand->center.x, hand->center.y ), ci::Vec2f( fingertip->x, fingertip->y ) );
            }
        }
    }
}

void SeedClusterApp::drawCursor( cv::Point smooth, float radius, float alpha )
{
    gl::pushModelView();
    gl::enableAlphaBlending();
    ci::Vec2f center = ci::Vec2f( smooth.x, smooth.y );
    glColor4f( 1, 1, 1, alpha );
    gl::drawSolidCircle( center, radius * 0.2 );
    gl::popModelView();
    
    gl::pushModelView();
    setColor( Vec3f( 0.364, 1, 0.6 ), alpha );
    gl::drawSolidCircle( center, radius * 0.9 );
    gl::popModelView();
    
    gl::pushModelView();
    glColor4f( 1, 1, 1, alpha );
    gl::translate( Vec3f( 0, 0, -1 ) );
    gl::drawSolidCircle( center, radius );
    gl::disableAlphaBlending();
    gl::popModelView();
}

void SeedClusterApp::drawSmoothHands()
{
    handmap.draw();
}

void SeedClusterApp::drawField()
{
    gl::pushModelView();
    gl::translate( Vec3f( 0.0f, 0.0f, -2.0f ) );
    setColor( Vec3f( hues[0], 0.5f, 0.5f ), 1.0f );
    cv::Mat field = tracker.displayField( cannyLowerThreshold, cannyUpperThreshold );
    drawMat( field );
    gl::popModelView();
}

void SeedClusterApp::drawMovieFrame()
{
    //if ( movieWriter ) {
        //movieWriter.addFrame( copyWindowSurface() );
    //}
}

void SeedClusterApp::drawParticles()
{
    glBegin( GL_LINES );
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    
    // draw all the particles as lines from mPosition to mLastPosition
    gl::enableAlphaBlending();
	
    for( list<Particle>::iterator partIt = particles.begin(); partIt != particles.end(); ++partIt ) {
        glLineWidth(Rand::randFloat(0.3f));
		glColor4f( 1.0f,1.0f,1.0f,0.8f );
        glVertex2f( partIt->mLastPosition );
        glVertex2f( partIt->mPosition );
    }
	
	gl::disableAlphaBlending();
	
    glEnd();
}

// void SeedClusterApp::drawRectangle()
// {
//     gl::pushModelView();
//     setColor( rectangleColor, rectangleAlpha * rectangleFactor );
//     // glColor4f( 1.0, 0.8, 0.2, 0.9 );
//     gl::translate( ci::Vec3f( -rectangle.center.x, -rectangle.center.y, 10 ) );
//     gl::rotate( ci::Vec3f( 0, 0, rectangle.angle ) );
//     gl::drawSolidRect( Rectf( -rectangle.size.width * rectangleFactor / 2, -rectangle.size.height * rectangleFactor / 2, rectangle.size.width * rectangleFactor, rectangle.size.height * rectangleFactor ), true );
//     gl::popModelView();
// }

void SeedClusterApp::draw()
{
    // clear it out to the bg
    gl::clear( Color( CM_HSV, background ) );

    
    gl::pushMatrices();
    gl::enableAlphaBlending();
    gl::color( ColorA( 1, 1, 1, 1 ) );
    gl::translate( Vec3f( -960.0f, -540.0f, 0 ) );
    if( bgImage ) gl::draw( bgImage );
    gl::disableAlphaBlending();
    gl::popMatrices();
    
    //gl::pushModelView();
    //gl::disableAlphaBlending();
    //glColor4f( 1, 1, 1, 1 );

    /*if ( greenMode ) {
        gl::pushModelView();
        gl::translate( Vec3f( -690.0f, -390.0f, -5.0f ) );
        gl::scale( Vec3f( 0.72f, 0.72f, 1.0f ) );
        gl::draw( backgroundTexture );

        // if ( rectangleMode ) {
        //     drawRectangle();
        // }

        gl::translate( Vec3f( 250.0f, 10.f, 3.0f ) );
        gl::scale( Vec3f( 2.25f, 2.25f, 1.0f ) );
	 
        gl::pushModelView();
        gl::translate( Vec3f( -200.0f, -100.0f, 0 ) );
        gl::scale( Vec3f( 1.5f, 1.5f, 1.0f ) );
        drawParticles();
        gl::popModelView();

        gl::enableAlphaBlending();
        drawSmoothHands();
        gl::disableAlphaBlending();
        gl::popModelView();
    }*/

    if ( tileMode ) 
    {
        cluster.drawTiles( posterMode );

        gl::color( ColorA( 0, 0, 0, 0 ) );
        gl::pushModelView();
        gl::translate( Vec3f( -320.0f, -240.0f, 0 ) );
        drawSmoothHands();
        gl::popModelView();
    }
    
    //gl::color( Color( 1, 0, 0 ) );
    //gl::drawSolidCircle( Vec2i( 0, 0 ), 20 );

    /*if ( innardsMode ) {
        drawRawHands();
        drawField();
        if ( seedMode ) {
            cluster.drawSeeds();
        }
    }*/

    //gl::popModelView();
    
    // params::InterfaceGl::draw();
    //drawMovieFrame();
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )






