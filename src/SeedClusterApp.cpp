#include <iostream>
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Matrix.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/CinderMath.h"
#include "cinder/Cinder.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "cinder/qtime/MovieWriter.h"
#include "CinderOpenCv.h"
#include "Kinect.h"
#include "Ease.h"
#include "HandTracker.h"
#include "TileCluster.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

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

    void setupMovieWriter();
    void updateCamera();
    void drawMat( cv::Mat & mat );
    void drawRawHands();
    void drawSmoothHands();
    void drawField();
    void drawMovieFrame();

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

    TileCluster cluster;
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

    int width;
    int height;

    int cannyLowerThreshold;
    int cannyUpperThreshold;
    qtime::MovieWriter movieWriter;
	params::InterfaceGl	params;
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
    std::vector<float> hues;
    ci::Vec2f closePoint;
    ci::Vec2f zoomPoint;
    float zoomAnchor;

    // input
    char key;
    bool keyIsDown;
    bool mouseIsDown;
    Vec2i mousePosition;
    Vec2f mouseVelocity;
    Vec2i centering;
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

void SeedClusterApp::setup()
{
    tracker.registerListener( this );

    cannyLowerThreshold = 0;
    cannyUpperThreshold = 0;

	params = params::InterfaceGl( "seedcluster", Vec2i( 200, 180 ) );
	params.addParam( "canny lower threshold", &cannyLowerThreshold, "min=0 max=250 step=1" );
	params.addParam( "canny upper threshold", &cannyUpperThreshold, "min=0 max=250 step=1" );

    defaultBackground = Vec3f( 0.0f, 0.1f, 0.2f );
    oneBackground = Vec3f( 0.0f, 0.1f, 0.65f );
    manyBackground = Vec3f( 0.0f, 0.1f, 0.95f );
    background = defaultBackground;
    bloomColor = Vec3f( Rand::randFloat(), 0.4f, 1.0f );
    backgroundTexture = gl::Texture( loadImage( loadResource( RES_BACKGROUND ) ) );

    mouseIsDown = false;
    keyIsDown = false;

    rotation.w = 0.0f;
    eye = Vec3f( 0.0f, 0.0f, -500.0f );
    towards = Vec3f( 0.0f, 00.0f, 0.0f );
    up = Vec3f::yAxis();
	camera.setPerspective( fovea, getWindowAspectRatio(), near, far );

    for ( int hue = 0; hue < 20; hue++ ) {
        hues.push_back( Rand::randFloat() );
    }

    kinectEnabled = Kinect::getNumDevices() > 0;
    if ( kinectEnabled ) {
        kinectTilt = 0.0f;
        kinect = Kinect( Kinect::Device() );
        kinectWidth = 640;
        kinectHeight = 480;
        depthTexture = gl::Texture( kinectWidth, kinectHeight );
        kinectColor = Vec3f( 0.0f, 0.0f, 1.0f );
    }

    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    // setupMovieWriter();
}

void SeedClusterApp::setupMovieWriter()
{
    std::string path = getSaveFilePath();
    if ( !path.empty() ) {
        qtime::MovieWriter::Format format;
        if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadResource( RES_WHIRLPOOL ) ) ) ) {
            movieWriter = qtime::MovieWriter( path, getWindowWidth(), getWindowHeight(), format );
        }
    }
}

void SeedClusterApp::keyDown( KeyEvent event )
{
    key = event.getChar();
    keyIsDown = true;
}

void SeedClusterApp::keyUp( KeyEvent event )
{
    keyIsDown = false;
}

void SeedClusterApp::mouseDown( MouseEvent event )
{
    mouseIsDown = true;
    bloomColor[2] = Rand::randFloat();

    cluster.mouseDown( centering, mouseVelocity, bloomColor );
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
    std::cout << "hand in - " << hand.hue << std::endl;

    // hueEase = Ease( background[0], oneBackground[0], 100 );
    // saturationEase = Ease( background[1], oneBackground[1], 100 );
    // brightnessEase = Ease( background[2], oneBackground[2], 100 );
}

void SeedClusterApp::handOut( const ix::Hand & hand )
{
    std::cout << "hand out - " << hand.hue << std::endl;

    // hueEase = Ease( background[0], defaultBackground[0], 100 );
    // saturationEase = Ease( background[1], defaultBackground[1], 100 );
    // brightnessEase = Ease( background[2], defaultBackground[2], 100 );
    cluster.releaseSeed();
}

void SeedClusterApp::handMove( const ix::Hand & hand )
{
    cluster.handOver( Vec2i( hand.center.x, hand.center.y ) );
}

void SeedClusterApp::handClose( const ix::Hand & hand )
{
    std::cout << "hand close - " << hand.hue << std::endl;

    closePoint = Vec2i( hand.center.x, hand.center.y );

    if ( cluster.chooseSeed( closePoint ) ) {
        
    } else {
        cluster.plantSeed( closePoint, Vec3f( hand.hue, Rand::randFloat() * 0.3 + 0.6, Rand::randFloat() * 0.4 + 0.3 ) );
    }
}

void SeedClusterApp::handOpen( const ix::Hand & hand )
{
    std::cout << "hand open - " << hand.hue << std::endl;

    cluster.releaseSeed();
}

void SeedClusterApp::handDrag( const ix::Hand & hand )
{
    cv::Point average = hand.smoothCenter( 5 );
    Vec2f smooth( average.x, average.y );

    if ( cluster.isSeedChosen() ) {
        cluster.chosenSeed->seek( smooth );
    }
}

void SeedClusterApp::secondHandIn( const ix::Hand & in, const ix::Hand & other ) 
{
    std::cout << "second hand in - " << in.hue << std::endl;
}

void SeedClusterApp::secondHandOut( const ix::Hand & out, const ix::Hand & other ) 
{
    std::cout << "second hand out - " << out.hue << std::endl;
}

void SeedClusterApp::firstHandClose( const ix::Hand & close, const ix::Hand & other ) 
{
    std::cout << "first hand close - " << close.hue << std::endl;

    handClose( close );
}

void SeedClusterApp::firstHandOpen( const ix::Hand & open, const ix::Hand & other ) 
{
    std::cout << "first hand open - " << open.hue << std::endl;

    handOpen( open );
}

void SeedClusterApp::secondHandClose( const ix::Hand & close, const ix::Hand & other )
{
    std::cout << "second hand close - " << close.hue << std::endl;

    closePoint = Vec2i( other.center.x, other.center.y );
    zoomPoint = Vec2i( close.center.x, close.center.y );

    zoomAnchor = sqrt( distance( close.center, other.center ) );
}

void SeedClusterApp::secondHandOpen( const ix::Hand & open, const ix::Hand & other ) 
{
    std::cout << "second hand open - " << open.hue << std::endl;

    handOpen( open );
}

void SeedClusterApp::openHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{
    handMove( first );
    handMove( second );
}

void SeedClusterApp::mixedHandsMove( const ix::Hand & open, const ix::Hand & close ) 
{
    handMove( open );
    handDrag( close );
}

void SeedClusterApp::closedHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{
    std::cout << "closed hands move" << std::endl;

    if ( cluster.isSeedChosen() ) {
        float zoom = sqrt( distance( first.smoothCenter( 3 ), second.smoothCenter( 3 ) ) ) / zoomAnchor;
        cluster.chosenSeed->zoom( pow( zoom, 2 ) );
    }
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

    // eye[2] += eye[2] * 0.0035f;
    // towards[0] += 1.0f;
    updateCamera();

    gl::setMatrices( camera );
    gl::rotate( rotation );
    cluster.update();
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

void SeedClusterApp::drawSmoothHands()
{
    for ( std::vector<ix::Hand>::iterator hand = tracker.hands.begin(); hand < tracker.hands.end(); hand++ ) {
        if ( hand->isHand ) {
            setColor( Vec3f( hand->hue, 0.5f, 0.7f ), 0.8f );
            cv::Point2f smooth = hand->smoothCenter( 4 );
            gl::drawSolidCircle( ci::Vec2f( smooth.x, smooth.y ), 50.0f );

            // setColor( Vec3f( hand->hue, 0.5f, 1.0f ), 0.8f );
            // for( vector<cv::Point2i>::iterator fingertip = hand->fingertips.begin(); fingertip != hand->fingertips.end(); fingertip++ ) {
            //     gl::drawSolidCircle( ci::Vec2f( fingertip->x, fingertip->y ), 10.0f );
            //     gl::drawLine( ci::Vec2f( hand->center.x, hand->center.y ), ci::Vec2f( fingertip->x, fingertip->y ) );
            // }
        }
    }
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
    if ( movieWriter ) {
        movieWriter.addFrame( copyWindowSurface() );
    }
}

void SeedClusterApp::draw()
{
	gl::clear( Color( CM_HSV, background ) );

    gl::pushModelView();
    gl::disableAlphaBlending();
    // setColor( Vec3f( 0.364, 1, 1 ), 1.0f );
    glColor4f( 1, 1, 1, 1 );
    gl::translate( Vec3f( -350.0f, -225.0f, -5.0f ) );
    gl::scale( Vec3f( 0.542f, 0.568f, 1.0f ) );
    gl::draw( backgroundTexture );

    // gl::enableAlphaBlending();
    gl::translate( Vec3f( 250.0f, 10.f, 3.0f ) );
    gl::scale( Vec3f( 2.25f, 2.147f, 1.0f ) );
    drawSmoothHands();
    // cluster.draw();
    // drawRawHands();
    // drawField();
    gl::popModelView();

    // gl::enableAdditiveBlending();


	// params::InterfaceGl::draw();
    // drawMovieFrame();
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )
