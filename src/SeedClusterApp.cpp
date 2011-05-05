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

    void drawMat( cv::Mat & mat );
    void updateCamera();

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

    mouseIsDown = false;
    keyIsDown = false;

    rotation.w = 0.0f;
    eye = Vec3f( 320.0f, -240.0f, -250.0f );
    towards = Vec3f( 320.0f, -240.0f, 0.0f );
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
    gl::enableAdditiveBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();

    // std::string path = getSaveFilePath();
    // if ( !path.empty() ) {
    //     qtime::MovieWriter::Format format;
    //     if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadResource( RES_WHIRLPOOL ) ) ) ) {
    //         movieWriter = qtime::MovieWriter( path, getWindowWidth(), getWindowHeight(), format );
    //     }
    // }
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

    // cluster.bloomPoint( mousePosition - centering, mouseVelocity, bloomColor );
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
    if ( tracker.numberOfHands() == 1 ) {
        cluster.handOver( Vec2i( hand.center.x, hand.center.y ) );
    } else {
        cluster.unhover();
    }
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
    cv::Point prepreprevious = hand.previousCenter( 3 );
    cv::Point preprevious = hand.previousCenter( 2 );
    cv::Point previous = hand.previousCenter( 1 );
    closePoint = Vec2f( hand.center.x, hand.center.y );
    Vec2f preprepreviousPoint = Vec2f( prepreprevious.x, prepreprevious.y );
    Vec2f prepreviousPoint = Vec2f( preprevious.x, preprevious.y );
    Vec2f previousPoint = Vec2f( previous.x, previous.y );
    Vec2f average = (closePoint + previousPoint + prepreviousPoint + preprepreviousPoint) / 4.0f;

    if ( cluster.isSeedChosen() ) {
        cluster.chosenSeed->seek( average );
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
}

void SeedClusterApp::firstHandOpen( const ix::Hand & open, const ix::Hand & other ) 
{
    std::cout << "first hand open - " << open.hue << std::endl;
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
}

void SeedClusterApp::openHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{

}

void SeedClusterApp::mixedHandsMove( const ix::Hand & open, const ix::Hand & close ) 
{

}

void SeedClusterApp::closedHandsMove( const ix::Hand & first, const ix::Hand & second ) 
{
    std::cout << "closed hands move" << std::endl;

    if ( cluster.isSeedChosen() ) {
        float zoom = sqrt( distance( first.center, second.center ) ) - zoomAnchor;
        cluster.chosenSeed->zoom( zoom );

        // float distanceMotion = sqrt( distance( first.center, second.center ) ) - sqrt( distance( first.previousCenter(), second.previousCenter() ) );
        // std::cout << "chosen seed zoom - " << distanceMotion << std::endl;
        // if ( distanceMotion < 10 && distanceMotion > -10 ) {
        //     cluster.chosenSeed->zoom( distanceMotion );
        // }
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

void SeedClusterApp::draw()
{
	gl::clear( Color( CM_HSV, background ) );
    cluster.draw();
    Vec3f center;

    // gl::pushModelView();
    // gl::translate( Vec3f( 0.0f, 0.0f, -5.0f ) );
    // glColor4f( 0.4f, 0.5f, 0.4f, 1.0f );
    // drawMat( depth );
    // gl::popModelView();

    for ( std::vector<ix::Hand>::iterator it = tracker.hands.begin(); it < tracker.hands.end(); it++ ) {
        if ( it->isHand ) {
            setColor( Vec3f( it->hue, 0.5f, 0.7f ), 0.8f );
            gl::drawSolidCircle( ci::Vec2f( it->center.x, it->center.y ), 20.0f );

            setColor( Vec3f( it->hue, 0.5f, 1.0f ), 0.8f );
            it->drawFingertips();
        }
    }

    gl::pushModelView();
    gl::translate( Vec3f( 0.0f, 0.0f, -20.0f ) );
    setColor( Vec3f( hues[0], 0.5f, 0.5f ), 1.0f );
    tracker.drawField( cannyLowerThreshold, cannyUpperThreshold );
    gl::popModelView();

	// params::InterfaceGl::draw();
    // if ( movieWriter ) {
    //     movieWriter.addFrame( copyWindowSurface() );
    // }
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )
