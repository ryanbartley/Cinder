//
//  ParticleRender.cpp
//  HodginParticlesRedux
//
//  Created by ryan bartley on 7/6/15.
//
//

#include "Render.h"
#include "ParticleController.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void ParticleRender::setup()
{
	mParticleBuffer = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(ParticleLayout) * MAX_PARTICLES, nullptr, GL_DYNAMIC_DRAW );
	auto particleGlsl = gl::GlslProg::create( gl::GlslProg::Format()
											 .vertex( loadAsset( "particle.vert" ) )
											 .fragment( loadAsset( "particle.frag" ) )
											 .attrib( geom::POSITION, "position" )
											 .attrib( geom::COLOR, "color" )
											 .attrib( geom::CUSTOM_0, "radius" ) );
	particleGlsl->uniform( "particleTex", 0 );
	auto particleLayout = geom::BufferLayout({
		geom::AttribInfo( geom::POSITION, geom::FLOAT, 3, sizeof(ParticleLayout), offsetof(ParticleLayout, position) ),
		geom::AttribInfo( geom::COLOR, geom::FLOAT, 4, sizeof(ParticleLayout), offsetof(ParticleLayout, color) ),
		geom::AttribInfo( geom::CUSTOM_0, geom::FLOAT, 1, sizeof(ParticleLayout), offsetof(ParticleLayout, radius) )
	});
	auto particleMesh = gl::VboMesh::create( MAX_PARTICLES, GL_POINTS, {{ particleLayout, mParticleBuffer }} );
	mParticleBatch = gl::Batch::create( particleMesh, particleGlsl );
	mParticleTexture = gl::Texture2d::create( loadImage( loadAsset( "particle.png" ) ) );
	
	std::vector<uint32_t> indices( MAX_PARTICLES * (MAX_TAIL_VERTICES_PER_PARTICLE + 1) );
	int index_multiplier = 0;
	for( int i = 0; i < indices.size(); i+=MAX_TAIL_VERTICES_PER_PARTICLE + 1 ) {
		for( int j = 0; j < MAX_TAIL_VERTICES_PER_PARTICLE; j++ ) {
			indices[i+j] = (index_multiplier * MAX_TAIL_VERTICES_PER_PARTICLE) + j;
		}
		indices[i+MAX_TAIL_VERTICES_PER_PARTICLE] = PRIMITIVE_RESTART_INDEX;
		index_multiplier++;
	}
	auto tailIndices = gl::Vbo::create( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW );
	mTailBuffer = gl::Vbo::create( GL_ARRAY_BUFFER, sizeof(ParticleTailLayout) * MAX_PARTICLES * MAX_TAIL_VERTICES_PER_PARTICLE, nullptr, GL_DYNAMIC_DRAW );
	auto tailLayout = geom::BufferLayout({
		geom::AttribInfo( geom::POSITION, geom::FLOAT, 3, sizeof(ParticleTailLayout), offsetof(ParticleTailLayout, position ) ),
		geom::AttribInfo( geom::COLOR, geom::FLOAT, 4, sizeof(ParticleTailLayout), offsetof(ParticleTailLayout, color ) )
	});
	auto tailMesh = gl::VboMesh::create( MAX_PARTICLES * MAX_TAIL_VERTICES_PER_PARTICLE, GL_TRIANGLE_STRIP, {{ tailLayout, mTailBuffer }}, indices.size(), GL_UNSIGNED_INT, tailIndices );
	mTailBatch = gl::Batch::create( tailMesh, gl::getStockShader( gl::ShaderDef().color() ) );
}

void ParticleRender::renderParticles()
{
	if( mNumActiveParticles == 0 ) return;
	
	gl::ScopedBlendAdditive scopeAdditive;
	gl::ScopedTextureBind	scopeTex( mParticleTexture, 0 );
	gl::ScopedState			scopePointSize( GL_VERTEX_PROGRAM_POINT_SIZE, true );
	
	mParticleBatch->draw( 0, mNumActiveParticles );
}


void ParticleRender::renderTrails()
{
	if( mNumActiveParticles == 0 ) return;
	
	gl::ScopedBlendAdditive scopeAdditive;
	gl::ScopedState			scopeState( GL_PRIMITIVE_RESTART, true );
	glPrimitiveRestartIndex( PRIMITIVE_RESTART_INDEX );
	
	mTailBatch->draw( 0, mNumActiveParticles * (MAX_TAIL_VERTICES_PER_PARTICLE + 1) );
}

void ParticleRender::bufferParticles( uint32_t numActiveParticles, const std::vector<ParticleLayout> &activeParticles )
{
	mNumActiveParticles = numActiveParticles;
	mParticleBuffer->bufferSubData( 0, sizeof(ParticleLayout) * mNumActiveParticles, activeParticles.data() );
}

void ParticleRender::bufferParticleTails( uint32_t numActiveParticles, const std::vector<ParticleTailLayout> &particleTails )
{
	mNumActiveParticles = numActiveParticles;
	mTailBuffer->bufferSubData( 0, sizeof(ParticleTailLayout) * MAX_TAIL_VERTICES_PER_PARTICLE * mNumActiveParticles, particleTails.data() );
}

void EmitterRender::setup()
{
	auto format			= gl::Texture2d::Format().wrap( GL_REPEAT );
	mDiffuseTexture		= gl::Texture::create( loadImage( loadAsset( "emitter.png" ) ), format );
	mNormalTexture		= gl::Texture::create( loadImage( loadAsset( "normal.png" ) ), format );
	mHeightTexture		= gl::Texture::create( loadImage( loadAsset( "bump.png" ) ), format );
	mSpecTexture		= gl::Texture::create( loadImage( loadAsset( "specExponent.png" ) ), format );
	
	try {
		mEmitterShader	= gl::GlslProg::create( gl::GlslProg::Format()
											   .vertex( loadAsset( "emitter_vert.glsl" ) )
											   .fragment( loadAsset( "emitter_frag.glsl" ) ) );
	}
	catch( gl::GlslProgExc &exc ) {
		std::cout << "GlslProg error, what: " << exc.what() << std::endl;
	}
	catch( ci::Exception &exc ) {
		std::cout << "Unable to load shader, what: " << exc.what() << std::endl;
	}
	
	mEmitterShader->uniform( "texDiffuse", 0 );
	mEmitterShader->uniform( "texNormal", 1 );
	mEmitterShader->uniform( "texHeight", 2 );
	mEmitterShader->uniform( "texSpec", 3 );
	
	auto sphereGeom		= geom::Sphere().subdivisions( 100 ).radius( 0.5f );
	mSphere				= gl::Batch::create( sphereGeom,  mEmitterShader );
}

void EmitterRender::renderEmitter( ParticleController &controller )
{
	static auto lightDirection = normalize( vec3( 0.0f, 0.25f, 1.0f ) );
	
	controller.mEmitterRadius = 50.0f - controller.mEmitterSpinSpeed * 30.0f + 70.0f * ( 1.0f - controller.mEmitterSpinSpeed );
	
	gl::ScopedBlendAlpha scopeBlend;
	gl::ScopedTextureBind scopeTex0( mDiffuseTexture, 0 );
	gl::ScopedTextureBind scopeTex1( mNormalTexture, 1 );
	gl::ScopedTextureBind scopeTex2( mHeightTexture, 2 );
	gl::ScopedTextureBind scopeTex3( mSpecTexture, 3 );
	
	mEmitterShader->uniform( "heat", controller.mEmitterHeat );
	mEmitterShader->uniform( "mouseVel", controller.mCurrentMouseVel * 0.025f );
	mEmitterShader->uniform( "spinSpeed", controller.mEmitterSpinSpeed );
	mEmitterShader->uniform( "counter", (float)getElapsedFrames() );
	mEmitterShader->uniform( "lightDir", lightDirection );
	mEmitterShader->uniform( "minHeight", 0.0f );
	
	{
		gl::ScopedDepth scopeDepth( true );
		gl::ScopedModelMatrix scopeModel;
		gl::translate( controller.mEmitterLoc );
		gl::scale( vec3( controller.mEmitterRadius * 2.0f ) );
		gl::rotate( toRadians( 180.0f ), vec3( 0, 1, 0 ) );
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
		mSphere->draw();
	}
	gl::ScopedDepth scopeDepth( true, false );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	for( int i=0; i<20; i++ ){
		mEmitterShader->uniform( "minHeight", (float)i/20.0f );
		auto localRadius = 52 + i * 0.1f;
		auto localAlpha = 0.1f;
		gl::ScopedModelMatrix scopeModel;
		gl::translate( controller.mEmitterLoc );
		controller.mEmitterRadius = localRadius - controller.mEmitterSpinSpeed * 30.0f + 70.0f * ( 1.0f - controller.mEmitterSpinSpeed );
		gl::scale( vec3( controller.mEmitterRadius * 2.0f ) );
		gl::color( ColorA( 1.0f, 1.0f, 1.0f, localAlpha ) );
		mSphere->draw();
	}
}

void RoomRender::setup( const std::vector<Constraint*> &constraints )
{
	
}

void RoomRender::renderRoom( ParticleController &controller )
{
	
}