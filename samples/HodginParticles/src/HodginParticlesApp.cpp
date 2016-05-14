#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/params/Params.h"

#include "ParticleController.h"
#include "Resources.h"

using namespace std;
using namespace ci;
using namespace ci::app;

#include <list>
#include <sstream>
using std::list;

class HodginParticlesApp : public App {
 public:
	void resize() override;
	void setup() override;
	void update() override;
	void keyDown( KeyEvent event ) override;
	void mouseMove( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;
	void draw() override;
	
	void setupParams();
	
	CameraPersp			mCamera;

	vec2				mMousePos;
	vec2				mEasedMousePos;
	bool				mMouseDown;
	
	ParticleController	mParticleController;
	params::InterfaceGlRef mParams;
};

void HodginParticlesApp::setup()
{
	mMouseDown			= false;
	mMousePos			= getWindowCenter();
	mEasedMousePos		= mMousePos;
	
	mCamera				= CameraPersp();
	mCamera.setPerspective( 50.0f, getWindowAspectRatio(), 1.0f, 1000.0f );
	mCamera.lookAt( vec3( 0.0f, 150.0f, -400.0f ), vec3( 0.0f, 150.0f, 0.0f ) );
	
	mParticleController.setup();
	setupParams();
	
	gl::clearColor( Color( 0.1f, 0.1f, 0.1f ) );
}


void HodginParticlesApp::resize()
{
	mParticleController.createConstraints( vec2( getWindowWidth(), getWindowHeight() ) );
}

void HodginParticlesApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' || event.getChar() == 'F' ){
		setFullScreen( ! isFullScreen() );
	}
}

void HodginParticlesApp::mouseMove( MouseEvent event )
{
	mMousePos = event.getPos();
}

void HodginParticlesApp::mouseDrag( MouseEvent event )
{
	mouseMove( event );
}

void HodginParticlesApp::mouseDown( MouseEvent event )
{
	mMouseDown = true;
}

void HodginParticlesApp::mouseUp( MouseEvent event )
{
	mMouseDown = false;
}

void HodginParticlesApp::update()
{
	Ray ray = mCamera.generateRay( mMousePos, getWindowSize() );
	mParticleController.update( ray, mMouseDown );
	
	mEasedMousePos -= ( mEasedMousePos - mMousePos ) * 0.15f;
}


void HodginParticlesApp::draw()
{
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	gl::setMatrices( mCamera );
	
	mParticleController.render();
	
	mParams->draw();

	getWindow()->setTitle( to_string( getAverageFps() ) );
}

void HodginParticlesApp::setupParams()
{
	mParams = params::InterfaceGl::create( "Hodgin Particles ", ci::ivec2( 400 ) );
	
	mParams->addParam( "Enable Perlin", &mParticleController.mEnablePerlin );
	mParams->addParam( "Enable Gravity", &mParticleController.mEnableGravity );
	mParams->addParam( "Enable Constraints", &mParticleController.mEnableConstraints );
	mParams->addParam( "Enable Repulsion", &mParticleController.mEnableRepulsion );
	mParams->addParam( "Gravity Vector", &mParticleController.mGravitationalAxis );
	mParams->addParam( "Render Particles", &mParticleController.mRenderParticles );
	mParams->addParam( "Render Trails", &mParticleController.mRenderTrails );
}


CINDER_APP( HodginParticlesApp, RendererGl, []( App::Settings *settings ) {
	settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
})