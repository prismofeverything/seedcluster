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
#include "OpenCV/cv.h"
#include "Fingertips.h"
#include "Kinect.h"
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

    void updateCamera();

	void keyDown( KeyEvent event );	
	void keyUp( KeyEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseMove( MouseEvent event );	
	void mouseDrag( MouseEvent event );	

    TileCluster cluster;
    Vec3f bloomColor;

    // viewport
    CameraPersp camera;
    Quatf rotation;
    Vec3f eye, towards, up;
    Color background;
    Vec3f changeColor;
    float fovea;
    float near;
    float far;

    int width;
    int height;

    // opencv
    cv::Mat1f depthz;
//    std::shared_array<float> depthz;

    // kinect
	Kinect kinect;
    bool kinectEnabled;
	gl::Texture kinectDepthTexture;
    std::shared_ptr<uint16_t> kinectDepth;
	float kinectTilt, kinectScale;
	float XOff, mYOff;
    int kinectWidth, kinectHeight;
    Vec3f kinectColor;

    ix::Fingertips fingertips;

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

void SeedClusterApp::prepareSettings( Settings *settings )
{
    Rand::randomize();

    width = 800;
    height = width * 0.866;
	settings->setWindowSize( width, height );
	settings->setFrameRate( 40.0f );
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
    background = Color( CM_HSV, Vec3f( 0.0f, 0.0f, 0.9f ) );
    bloomColor = Vec3f( Rand::randFloat(), 0.4f, 1.0f );

    mouseIsDown = false;
    keyIsDown = false;

    rotation.w = 0.0f;
    eye = Vec3f( 0.0f, 0.0f, 1000.0f );
    towards = Vec3f::zero();
    up = Vec3f::yAxis();
	camera.setPerspective( fovea, getWindowAspectRatio(), near, far );

    depthz = cv::Mat1f( 480, 640 );

    kinectEnabled = Kinect::getNumDevices() > 0;
    if ( kinectEnabled ) {
        kinectTilt = 0.0f;
        kinect = Kinect( Kinect::Device() );
        kinectWidth = 640;
        kinectHeight = 480;
        // kinectDepthTexture = gl::Texture( kinectWidth, kinectHeight );
        kinectColor = Vec3f( 0.0f, 0.0f, 1.0f );
    }

    gl::enableAlphaBlending();
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

void SeedClusterApp::update()
{
    if ( kinectEnabled ) {
        if( kinect.checkNewDepthFrame() ) {
            // kinectDepthTexture = kinect.getDepthImage();
            // kinectDepthTexture.setFlipped( true );
            kinectDepth = kinect.getDepthData();
            fingertips.unproject( kinectDepth.get(), NULL, NULL, (float *) depthz.data );
            fingertips.detectFingertips( depthz, 0.0f, 100.0f );
        }

        if( kinectTilt != kinect.getTilt() ) {
            kinect.setTilt( kinectTilt );
        }
    }

    // eye[2] += eye[2] * 0.0035f;
    // towards[0] += 1.0f;
    updateCamera();

    gl::setMatrices( camera );
    gl::rotate( rotation );
    cluster.update();
}

void SeedClusterApp::draw()
{
	gl::clear( background );
    cluster.draw();
    Vec3f center;

    glColor4f( 1.0f, 0.2f, 0.5f, 1.0f );
    fingertips.drawContours();
    fingertips.drawFingertips();
    fingertips.drawField();

    // for ( std::vector<cv::Point2i>::iterator it = fingertips.fingertips.begin(); it < fingertips.fingertips.end(); it++ ) {
    //     center = Vec3f( it->x - 320, it->y - 240, 0.0f );
    //     // towards += center * 0.01;
    //     gl::drawSphere( center, 50.0f );
    // }
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )
