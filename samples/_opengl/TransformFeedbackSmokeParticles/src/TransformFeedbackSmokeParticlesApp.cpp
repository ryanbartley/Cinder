#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"

#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"

#include "cinder/gl/TransformFeedbackObj.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// This example is based on samples from the 10th chapter of the book, OpenGL 4.0
// Shading Language Cookbook. For more in-depth discussion of what is going on
// here please refer to that reference.

const int nParticles			= 50000;

// These constants will stand for the indices in the Vao, and Transform Feedback
// buffers.
const int PositionIndex			= 0;
const int VelocityIndex			= 1;
const int StartTimeIndex		= 2;
const int InitVelIndex			= 3;

struct ParticleInfo {
	vec3	position;
	vec3	velocity;
	float	startTime;
	vec3	initVelocity;
};

class TransformFeedbackSmokeParticlesApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	void loadBuffers();
	void loadShaders();
	
  private:
	gl::VaoRef						mPVao[2];
	gl::TransformFeedbackObjRef		mPFeedbackObj[2];
	gl::VboRef						mAttribBuffer[2];
	
	gl::GlslProgRef					mPUpdateGlsl, mPRenderGlsl;
	
	Rand							mRand;
	CameraPersp						mCam;
	uint32_t						mDrawBuff;
	ci::Timer						mTimer;
};

void TransformFeedbackSmokeParticlesApp::setup()
{
	mTimer.start();
	
	mDrawBuff = 0;
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 1000.0f );
	mCam.lookAt( vec3( 0, 0, 10 ), vec3( 0, 0, 0 ) );
	
	loadShaders();
	loadBuffers();
}

void TransformFeedbackSmokeParticlesApp::loadShaders()
{
	try {
		// Create a vector of Transform Feedback "Varyings".
		// These strings tell OpenGL what to look for when capturing
		// Transform Feedback data. For instance, Position, Velocity,
		// and StartTime are variables in the updateSmoke.vert that we
		// write our calculations to.
		std::vector<std::string> transformFeedbackVaryings( 4 );
		transformFeedbackVaryings[PositionIndex] =			"Position";
		transformFeedbackVaryings[VelocityIndex] =			"Velocity";
		transformFeedbackVaryings[StartTimeIndex] =			"StartTime";
		// Since we're using an interleaved buffer, there's data that
		// won't be changing and we need to tell OpenGL that. On Desktop,
		// there exists two constructs for non-writeable, interleaved
		// data. The first is "gl_SkipComponents#" with the # representing
		// a number between 1 and 4. These are float values. The number
		// of floats we want to skip in this case is 3 because Initial
		// Velocity is a vec3.
#if ! defined( CINDER_GL_ES )
		transformFeedbackVaryings[InitVelIndex] =			"gl_SkipComponents3";
#else
		// On GL ES 3, this paradigm doesn't yet exist. Therefore we say
		// that we're going to write into the out version of InitialVelocity
		// but in our shader it's just going to be a copy.
		transformFeedbackVaryings[InitVelIndex] =			"InitVelocity";
#endif
		
		ci::gl::GlslProg::Format mUpdateParticleGlslFormat;
		// Notice that we don't offer a fragment shader. We don't need
		// one because we're not trying to write pixels to the screen
		// when updating the position, velocity, etc.
		mUpdateParticleGlslFormat
#if ! defined( CINDER_GL_ES )
			.vertex( loadAsset( "updateSmoke_osx.vert" ) )
#else
		// However on ES 3, we make a blank fragment shader.
			.vertex( loadAsset( "updateSmoke_ios.vert" ) )
			.fragment( "#version 300 es void main(){}" )
#endif
		// This option will be either GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS,
		// depending on the structure of our data, below. We're only using one vbo
		// for all of our data. Therefore, we're using GL_INTERLEAVED_ATTRIBS. It is
		// also the most performative way to use transform feedback in most cases.
			.feedbackFormat( GL_INTERLEAVED_ATTRIBS )
		// Pass the feedbackVaryings to glsl
			.feedbackVaryings( transformFeedbackVaryings )
			.attribLocation( "VertexPosition",			PositionIndex )
			.attribLocation( "VertexVelocity",			VelocityIndex )
			.attribLocation( "VertexStartTime",			StartTimeIndex )
			.attribLocation( "VertexInitialVelocity",	InitVelIndex );
		
		mPUpdateGlsl = ci::gl::GlslProg::create( mUpdateParticleGlslFormat );
	}
	catch( const ci::gl::GlslProgCompileExc &ex ) {
		console() << "PARTICLE UPDATE GLSL ERROR: " << ex.what() << std::endl;
	}

	mPUpdateGlsl->uniform( "Origin", vec3( 0.0f ) );
	mPUpdateGlsl->uniform( "Accel", vec3( 0.0f ) );
	mPUpdateGlsl->uniform( "ParticleLifetime", 3.0f );
	
	try {
		ci::gl::GlslProg::Format mRenderParticleGlslFormat;
		// This being the render glsl, we provide a fragment shader.
		mRenderParticleGlslFormat
#if ! defined( CINDER_GL_ES )
			.vertex( loadAsset( "renderSmoke_osx.vert" ) )
			.fragment( loadAsset( "renderSmoke_osx.frag" ) )
#else
			.vertex( loadAsset( "renderSmoke_ios.vert" ) )
			.fragment( loadAsset( "renderSmoke_ios.frag" ) )
#endif
			.attribLocation( "VertexPosition",			PositionIndex )
			.attribLocation( "VertexVelocity",			VelocityIndex )
			.attribLocation( "VertexStartTime",			StartTimeIndex )
			.attribLocation( "VertexInitialVelocity",	InitVelIndex );
		
		mPRenderGlsl = ci::gl::GlslProg::create( mRenderParticleGlslFormat );
	}
	catch( const ci::gl::GlslProgCompileExc &ex ) {
		console() << "PARTICLE RENDER GLSL ERROR: " << ex.what() << std::endl;
	}
	
	mPRenderGlsl->uniform( "MinParticleSize", 1.0f );
	mPRenderGlsl->uniform( "MaxParticleSize", 64.0f );
	mPRenderGlsl->uniform( "ParticleLifetime", 3.0f );
}

void TransformFeedbackSmokeParticlesApp::loadBuffers()
{
	std::vector<ParticleInfo> particles( nParticles );
	float time = 0.0f;
	float rate = 0.005f;
	for( auto & particle : particles ) {
		particle.position		= vec3();
		particle.initVelocity	= ci::randVec3f() * mix( 0.0f, 1.5f, mRand.nextFloat() );
		particle.velocity		= particle.initVelocity;
		particle.startTime		= time;
		time += rate;
	}
	
	mAttribBuffer[0] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(ParticleInfo), particles.data(), GL_STATIC_DRAW );
	mAttribBuffer[1] = gl::Vbo::create( GL_ARRAY_BUFFER, particles.size() * sizeof(ParticleInfo), particles.data(), GL_STATIC_DRAW );
	
	for( int i = 0; i < 2; i++ ) {
		// Initialize the Vao's holding the info for each buffer
		mPVao[i] = ci::gl::Vao::create();
		
		gl::ScopedVao scopeVao( mPVao[i] );
		{
			gl::ScopedBuffer scopeBuffer( mAttribBuffer[i] );
			
			ci::gl::enableVertexAttribArray( PositionIndex );
			ci::gl::enableVertexAttribArray( VelocityIndex );
			ci::gl::enableVertexAttribArray( StartTimeIndex );
			ci::gl::enableVertexAttribArray( InitVelIndex );
	
			ci::gl::vertexAttribPointer( PositionIndex,  3, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), 0 );
			ci::gl::vertexAttribPointer( VelocityIndex,  3, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (const GLvoid*)(sizeof(float) * 3) );
			ci::gl::vertexAttribPointer( StartTimeIndex, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (const GLvoid*)(sizeof(float) * 6) );
			ci::gl::vertexAttribPointer( InitVelIndex,   3, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (const GLvoid*)(sizeof(float) * 7) );
		}
		
		// Create a TransformFeedbackObj, which is similar to Vao
		// It's used to capture the output of a glsl and uses the
		// index of the feedback's varying variable names.
		mPFeedbackObj[i] = gl::TransformFeedbackObj::create();
		
		// Bind the TransformFeedbackObj and bind each corresponding buffer
		// to it's index.
		mPFeedbackObj[i]->bind();
		gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, mAttribBuffer[i] );
		mPFeedbackObj[i]->unbind();
	}
}

void TransformFeedbackSmokeParticlesApp::mouseDown( MouseEvent event )
{
	auto normPoint = vec2(event.getPos()) / vec2(getWindowSize());
	normPoint.y = 1 - normPoint.y;
	
	auto ray = mCam.generateRay( normPoint.x, normPoint.y, getWindowAspectRatio() );
	
	mPUpdateGlsl->uniform( "Origin", ray.calcPosition( 5.0f ) );
}

void TransformFeedbackSmokeParticlesApp::mouseDrag( MouseEvent event )
{
	auto normPoint = vec2(event.getPos()) / vec2(getWindowSize());
	normPoint.y = 1 - normPoint.y;
	
	auto ray = mCam.generateRay( normPoint.x, normPoint.y, getWindowAspectRatio() );
	
	mPUpdateGlsl->uniform( "Origin", ray.calcPosition( 10.0f ) );
}

void TransformFeedbackSmokeParticlesApp::update()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	// This equation just reliably swaps all concerned objects
	mDrawBuff = 1 - mDrawBuff;
	
	gl::ScopedGlslProg	glslScope( mPUpdateGlsl );
	// We use this vao for input to the Glsl, while using the opposite
	// for the TransformFeedbackObj.
	gl::ScopedVao		vaoScope( mPVao[mDrawBuff] );
	// Because we're not using a fragment shader, we need to
	// stop the rasterizer. This will make sure that OpenGL won't
	// move to the rasterization stage.
	gl::ScopedState		stateScope( GL_RASTERIZER_DISCARD, true );
	
	mTimer.stop();
	mPUpdateGlsl->uniform( "H", (float)mTimer.getSeconds() );
	mPUpdateGlsl->uniform( "Time", (float)getElapsedFrames() / 60.0f );
	
	// Opposite TransformFeedbackObj to catch the calculated values
	// In the opposite buffer
	mPFeedbackObj[1-mDrawBuff]->bind();
	
	// We begin Transform Feedback, using the same primitive that
	// we're "drawing". Using points for the particle system.
	gl::beginTransformFeedback( GL_POINTS );
	gl::drawArrays( GL_POINTS, 0, nParticles );
	gl::endTransformFeedback();
	
	mTimer.start();
}

void TransformFeedbackSmokeParticlesApp::draw()
{
	// clear out the window with black
	
	static float rotateRadians = 0.0f;
	rotateRadians += 0.01f;
	
	gl::ScopedVao			vaoScope( mPVao[1-mDrawBuff] );
	gl::ScopedGlslProg		glslScope( mPRenderGlsl );
	gl::ScopedBlend			blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	// On Desktop Gl, you must enable the point size.
#if ! defined( CINDER_GL_ES )
	gl::ScopedState			pointScope( GL_PROGRAM_POINT_SIZE, true );
#endif

	gl::ScopedMatrices scopeMat;
	gl::setMatrices( mCam );
	
	mPRenderGlsl->uniform( "Time", getElapsedFrames() / 60.0f );
	gl::setDefaultShaderVars();
	gl::drawArrays( GL_POINTS, 0, nParticles );
}

CINDER_APP( TransformFeedbackSmokeParticlesApp, RendererGl, []( App::Settings * settings ) {
																settings->setMultiTouchEnabled(false);
															})
