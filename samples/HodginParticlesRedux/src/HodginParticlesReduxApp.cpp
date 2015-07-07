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

class HodginParticlesReduxApp : public App {
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

	vec3			mMouseLoc;
	vec3			mMousePastLoc;
	vec3			mMouseVel;
	
	bool			mIsMouseDown;
	
	ParticleController	mParticleController;
	params::InterfaceGlRef mParams;
};

void HodginParticlesReduxApp::setup()
{
	mIsMouseDown		= false;
	
	mParticleController.setup();
	setupParams();
	
	gl::clearColor( ColorA( 0.0025f, 0.0025f, 0.0025f, 1 ) );
}


void HodginParticlesReduxApp::resize()
{
	mParticleController.createConstraints( vec2( getWindowWidth(), getWindowHeight() ) );
}

void HodginParticlesReduxApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' || event.getChar() == 'F' ){
		setFullScreen( ! isFullScreen() );
	}
}

void HodginParticlesReduxApp::mouseMove( MouseEvent event )
{
	mMouseLoc = vec3( event.getX(), event.getY(), 0.0f );
}

void HodginParticlesReduxApp::mouseDrag( MouseEvent event )
{
	mouseMove( event );
}

void HodginParticlesReduxApp::mouseDown( MouseEvent event )
{
	mIsMouseDown = true;
}

void HodginParticlesReduxApp::mouseUp( MouseEvent event )
{
	mIsMouseDown = false;
}

void HodginParticlesReduxApp::update()
{
	mMousePastLoc -= ( mMousePastLoc - mMouseLoc ) * 0.01f;
	mMouseVel -= ( mMouseVel - ( mMousePastLoc - mMouseLoc ) ) * 0.01f;
	
	mParticleController.mCurrentMouseVel = mMouseVel;
	mParticleController.update( mMouseLoc, mIsMouseDown );
}


void HodginParticlesReduxApp::draw()
{
	gl::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	gl::setMatricesWindowPersp( getWindowSize() );
	
	mParticleController.render();
	
	mParams->draw();

	getWindow()->setTitle( to_string( getAverageFps() ) );
}

void HodginParticlesReduxApp::setupParams()
{
	mParams = params::InterfaceGl::create( "Hodgin Particles Redux", ci::ivec2( 400 ) );
	
	mParams->addParam( "Enable Perlin", &mParticleController.mEnablePerlin );
	mParams->addParam( "Enable Gravity", &mParticleController.mEnableGravity );
	mParams->addParam( "Enable Constraints", &mParticleController.mEnableConstraints );
	mParams->addParam( "Enable Repulsion", &mParticleController.mEnableRepulsion );
	mParams->addParam( "Gravity Vector", &mParticleController.mGravitationalAxis );
	mParams->addParam( "Render Particles", &mParticleController.mRenderParticles );
	mParams->addParam( "Render Trails", &mParticleController.mRenderTrails );
}


CINDER_APP( HodginParticlesReduxApp, RendererGl, []( App::Settings *settings ) {
	settings->setWindowSize( 1280, 720 );
	settings->setFrameRate( 60.0f );
	settings->setFullScreen( false );
})