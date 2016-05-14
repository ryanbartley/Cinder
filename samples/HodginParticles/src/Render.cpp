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
	
	std::vector<uint32_t> indices;
	indices.reserve( MAX_PARTICLES * (MAX_TAIL_VERTICES_PER_PARTICLE + 1) );
	int index_multiplier = 0;
	for( int i = 0; i < MAX_PARTICLES; i++ ) {
		for( int j = 0; j < MAX_TAIL_VERTICES_PER_PARTICLE; j++ ) {
			indices.push_back( (index_multiplier * MAX_TAIL_VERTICES_PER_PARTICLE) + j );
		}
		indices.push_back( PRIMITIVE_RESTART_INDEX );
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
	
	gl::ScopedDepthWrite scopeWrite( false );
	gl::ScopedDepthTest scopeDepth( true );
	gl::ScopedBlendAdditive scopeAdditive;
	gl::ScopedTextureBind	scopeTex( mParticleTexture, 0 );
	gl::ScopedState			scopePointSize( GL_VERTEX_PROGRAM_POINT_SIZE, true );
	
	mParticleBatch->draw( 0, mNumActiveParticles );
}


void ParticleRender::renderTrails()
{
	if( mNumActiveParticles == 0 ) return;
	
	gl::ScopedDepthWrite scopeWrite( false );
	gl::ScopedDepthTest scopeDepth( true );
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
	auto format			= gl::Texture2d::Format().wrapS( GL_REPEAT ).wrapT( GL_REPEAT );
	mDiffuseTex			= gl::Texture::create( loadImage( loadAsset( "diffuse.png" ) ),				format );
	mNormalTex			= gl::Texture::create( loadImage( loadAsset( "normal.png" ) ),				format );
	mAoTex				= gl::Texture::create( loadImage( loadAsset( "ambientOcclusion.png" ) ),	format );
	mHeightTex			= gl::Texture::create( loadImage( loadAsset( "height.png" ) ),				format );
	mReflOccTex			= gl::Texture::create( loadImage( loadAsset( "reflectiveOcclusion.png" ) ),	format );
	
	try{
		mEmitterGlsl	= gl::GlslProg::create( loadAsset( "emitter.vert" ), loadAsset( "emitter.frag" ) );
	}
	catch( gl::GlslProgCompileExc e ){
		std::cout << e.what() << std::endl;
	}
	mEmitterGlsl->uniform( "uDiffuseTex",	0 );
	mEmitterGlsl->uniform( "uNormalTex",	1 );
	mEmitterGlsl->uniform( "uAoTex",		2 );
	mEmitterGlsl->uniform( "uHeightTex",	3 );
	mEmitterGlsl->uniform( "uReflOccTex",	4 );
	
	auto sphereGeom		= geom::Sphere().subdivisions( 200 ).radius( 1.0f );
	mSphere				= gl::Batch::create( sphereGeom,  mEmitterGlsl );
}

void EmitterRender::renderEmitter( ParticleController &controller )
{
	gl::ScopedGlslProg		prog( mEmitterGlsl );
	gl::ScopedBlendAlpha	alpha;
	gl::ScopedDepth			depth( true );
	gl::ScopedTextureBind	tex0( mDiffuseTex,	0 );
	gl::ScopedTextureBind	tex1( mNormalTex,	1 );
	gl::ScopedTextureBind	tex2( mAoTex,		2 );
	gl::ScopedTextureBind	tex3( mHeightTex,	3 );
	gl::ScopedTextureBind	tex4( mReflOccTex,	4 );
	
	gl::ScopedModelMatrix	model;
	gl::multModelMatrix( controller.mEmitterMatrix );
	
	mEmitterGlsl->uniform( "uTime",			(float)getElapsedSeconds() * 0.1f );
	mEmitterGlsl->uniform( "uRotSpeed",		controller.mEmitterRotSpeed );
	mEmitterGlsl->uniform( "uSOffset",		(float)controller.mEasedPosition.x/(float)getWindowWidth() * 5.0f );
	mEmitterGlsl->uniform( "uTOffset",		(float)controller.mEasedPosition.y/(float)getWindowHeight() * 5.0f );
	mSphere->draw();
}

void RoomRender::setup( const std::vector<Constraint*> &constraints )
{
	
}

void RoomRender::renderRoom( ParticleController &controller )
{
	
}