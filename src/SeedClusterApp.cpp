#include <iostream>
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/Vector.h"
#include "cinder/Matrix.h"
#include "cinder/Color.h"
#include "cinder/Camera.h"
#include "cinder/CinderMath.h"
#include "cinder/Cinder.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCv.h"
#include "Kinect.h"
#include "Ease.h"
#include "HandTracker.h"
#include "TileCluster.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SeedClusterApp : public AppBasic {
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
    void handOut( const ix::Hand & hand );
    void handClose( const ix::Hand & hand );
    void handDrag( const ix::Hand & hand );
    void handOpen( const ix::Hand & hand );

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

    ix::HandTracker<SeedClusterApp> tracker;
    std::vector<float> hues;

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

    defaultBackground = Vec3f( 0.0f, 0.1f, 0.2f );
    oneBackground = Vec3f( 0.0f, 0.1f, 0.65f );
    manyBackground = Vec3f( 0.0f, 0.1f, 0.95f );
    background = defaultBackground;
    bloomColor = Vec3f( Rand::randFloat(), 0.4f, 1.0f );

    mouseIsDown = false;
    keyIsDown = false;

    rotation.w = 0.0f;
    eye = Vec3f( 320.0f, -240.0f, -200.0f );
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
    if ( tracker.numberOfHands() == 1 ) {
        hueEase = Ease( background[0], oneBackground[0], 100 );
        saturationEase = Ease( background[1], oneBackground[1], 100 );
        brightnessEase = Ease( background[2], oneBackground[2], 100 );
    } else if ( tracker.numberOfHands() > 1 ) {
        hueEase = Ease( background[0], manyBackground[0], 100 );
        saturationEase = Ease( background[1], manyBackground[1], 100 );
        brightnessEase = Ease( background[2], manyBackground[2], 100 );
    }
}

void SeedClusterApp::handOut( const ix::Hand & hand )
{
    if ( tracker.numberOfHands() == 0 ) {
        hueEase = Ease( background[0], defaultBackground[0], 100 );
        saturationEase = Ease( background[1], defaultBackground[1], 100 );
        brightnessEase = Ease( background[2], defaultBackground[2], 100 );
        cluster.releaseSeed();
    } else if ( tracker.numberOfHands() > 0 ) {
        hueEase = Ease( background[0], oneBackground[0], 100 );
        saturationEase = Ease( background[1], oneBackground[1], 100 );
        brightnessEase = Ease( background[2], oneBackground[2], 100 );
    }
}

void SeedClusterApp::handClose( const ix::Hand & hand )
{
    if ( tracker.numberOfHands() == 1 ) {
        if ( cluster.chooseSeed( Vec2i( hand.center.x, hand.center.y ) ) ) {

        } else {
            cluster.seed( Vec2i( hand.center.x, hand.center.y ), Vec3f( hand.hue, Rand::randFloat() * 0.3 + 0.6, Rand::randFloat() * 0.4 ) );
        }
    }
}

void SeedClusterApp::handDrag( const ix::Hand & hand )
{
    if ( cluster.isSeedChosen() ) {
        cluster.chosenSeed->seek( Vec2i( hand.center.x, hand.center.y ) );
    }
}

void SeedClusterApp::handOpen( const ix::Hand & hand )
{
    if ( tracker.numberOfHands() == 1 ) {
        cluster.releaseSeed();
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
            // cv::dilate( depth, depth, cv::Mat() );
            // cv::morphologyEx( depth, depth, cv::MORPH_CLOSE, cv::Mat(), cv::Point( -1, -1 ), 2 );
            tracker.detectHands( depth, 150, 280 );
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
    tracker.drawField();
    gl::popModelView();
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )
