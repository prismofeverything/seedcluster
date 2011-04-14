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
#include "Kinect.h"
#include "TileCluster.h"

using namespace ci;
using namespace ci::app;
using namespace std;


GLfloat no_mat[]			= { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[]		= { 0.6, 0.3, 0.4, 1.0 };
GLfloat mat_diffuse[]		= { 0.3, 0.5, 0.8, 1.0 };
GLfloat mat_specular[]		= { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_emission[]		= { 0.0, 0.1, 0.3, 0.0 };

GLfloat mat_shininess[]		= { 128.0 };
GLfloat no_shininess[]		= { 0.0 };


class SeedClusterApp : public AppBasic {
  public:
	
	
	bool DIFFUSE;
	bool AMBIENT;
	bool SPECULAR;
	bool EMISSIVE;
	float mDirectional;
	
	void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();

    Vec3f randomVec3f();
    Matrix44f randomMatrix44f( float scale = 1.0f, float offset = 0.0f );
    Color randomColor();

    void updateCamera();

	void keyDown( KeyEvent event );	
	void keyUp( KeyEvent event );	
	void mouseDown( MouseEvent event );	
	void mouseUp( MouseEvent event );	
	void mouseMove( MouseEvent event );	
	void mouseDrag( MouseEvent event );	

	Vec2f			mMousePos;
	bool mIsMouseDown;
	
	
    TileCluster cluster;
    Vec3f bloomColor;
    Matrix44f bloomTransform;

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

    // kinect
	Kinect kinect;
    bool kinectEnabled;
	gl::Texture kinectDepthTexture;
    std::shared_ptr<uint16_t> kinectDepth;
	float kinectTilt, kinectScale;
	float XOff, mYOff;
    int kinectWidth, kinectHeight;
    Vec3f kinectColor;

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
    
	DIFFUSE		= true;
	AMBIENT		= false;
	SPECULAR	= false;
	EMISSIVE	= false;
	
	background = Color( CM_HSV, Vec3f( 0.0f, 0.0f, 0.9f ) );
    bloomColor = Vec3f( Rand::randFloat(), 0.4f, 1.0f );
    bloomTransform = Matrix44f() + randomMatrix44f( 0.04f, -0.02f );

    mouseIsDown = false;
    keyIsDown = false;

    //rotation.w = 0.0f;
    rotation.w = -0.74f;
    eye = Vec3f( 0.0f, 0.0f, 3000.0f );
    towards = Vec3f::zero();
    up = Vec3f::yAxis();
	camera.setPerspective( fovea, getWindowAspectRatio(), near, far );

    kinectEnabled = Kinect::getNumDevices() > 0;
    if ( kinectEnabled ) {
        kinectTilt = 0.0f;
        kinect = Kinect( Kinect::Device() );
        kinectWidth = 640;
        kinectHeight = 480;
        kinectDepthTexture = gl::Texture( kinectWidth, kinectHeight );
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
    mIsMouseDown = true;
	mouseIsDown = true;
    bloomColor[2] = Rand::randFloat();

    // cluster.bloomPoint( mousePosition - centering, mouseVelocity, bloomColor );
    cluster.mouseDown( centering, mouseVelocity, bloomColor );
}

void SeedClusterApp::mouseUp( MouseEvent event )
{
    mouseIsDown = false;
	mIsMouseDown = false;
}

void SeedClusterApp::mouseMove( MouseEvent event )
{
    mouseVelocity = event.getPos() - mousePosition;
    mousePosition = event.getPos();
	
	mMousePos.x = event.getX() - getWindowWidth() * 0.5f;
	mMousePos.y = getWindowHeight() * 0.5f - event.getY();
	
}

void SeedClusterApp::mouseDrag( MouseEvent event )
{
    mouseMove( event );
    // system->mouseImpact( mousePosition, mouseVelocity, changeColor );
}

void SeedClusterApp::update() 
{
    eye[2] += eye[2] * 0.00015f;
    // towards[0] += 1.0f;
    updateCamera();
	
	if( mIsMouseDown ) // using small number instead of 0.0 because lights go black after a few seconds when going to 0.0f
		mDirectional -= ( mDirectional - 0.00001f ) * 0.1f;  
	else 
		mDirectional -= ( mDirectional - 1.0f ) * 0.1f;
	
    gl::setMatrices( camera );
    gl::rotate( rotation );
    cluster.update(); 
}

void SeedClusterApp::draw()
{

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	GLfloat light_position[] = { mMousePos.x, mMousePos.y, -175.0f, mDirectional };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
	
	gl::clear( background );
    cluster.draw();
}


CINDER_APP_BASIC( SeedClusterApp, RendererGl )
