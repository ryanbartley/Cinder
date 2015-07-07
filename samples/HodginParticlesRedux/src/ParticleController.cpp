#include "ParticleController.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include <climits>
#include "glm/gtc/noise.hpp"
#include "Render.h"

using namespace ci;
using namespace ci::app;
using std::list;
using std::vector;
using std::cout;


ParticleController::ParticleController()
: mEnablePerlin( true ), mEnableGravity( false ), mEnableConstraints( true ),
	mEnableRepulsion( false ), mIsTouchingConstraint( false ), mParticleRender( new ParticleRender ),
	mEmitterRender( new EmitterRender ), mRoomRender( new RoomRender ), mRenderParticles( true ), mRenderTrails( true )
{
}

void ParticleController::setup()
{
	mEmitterRender->setup();
	mParticleRender->setup();
}

void ParticleController::createConstraints( vec2 windowDim )
{
	mConstraints.clear();
	//mConstraints.push_back( new Constraint( vec3( 1, 0, 0 ), 0.0f, windowDim.x ) );
	mConstraints.push_back( new Constraint{ vec3( 0, 1, 0 ), -1000.0f, windowDim.y * 0.625f } );
}

void ParticleController::update( const vec3 &mouseLoc, bool mouseIsDown )
{
	updateEmitter( mouseLoc, mouseIsDown );
	updateParticles();
}

void ParticleController::updateEmitter( const vec3 &mouseLoc, bool mouseIsDown )
{
	vec3 mouseVel( mouseLoc.x - mEmitterLoc.x, mouseLoc.y - mEmitterLoc.y, 0 );
	mEmitterVel -= ( mEmitterVel - mouseVel ) * 0.1f;
	
	mEmitterLoc += mEmitterVel;
	mEmitterVel *= 0.9f;
	
	if( mIsTouchingConstraint && mouseIsDown )
		mEmitterVel += Rand::randVec3() * 3.0f * mEmitterHeat;
	
	if( mouseIsDown  ) {
		mEmitterSpinSpeed -= ( mEmitterSpinSpeed - 1.0f ) * 0.04f;
		mEmitterHeat -= ( mEmitterHeat - mEmitterSpinSpeed ) * 0.1f;
		mIsTouchingConstraint = false;
	} else {
		mEmitterSpinSpeed -= ( mEmitterSpinSpeed - 0.0f ) * 0.05f;
		mEmitterHeat -= ( mEmitterHeat - mEmitterSpinSpeed ) * 0.01f;
	}
	
	if( mEnableConstraints ) {
		applyEmmiterConstraints();
	}
	
	// add particles
	if( mouseIsDown && mEmitterHeat > 0.5f ){
		int depth = mouseLoc.y - 380;
		float per = depth/340.0f;
		vec3 vel = mEmitterVel * per;
		vel.y *= 0.02f;
		int numParticlesToSpawn = ( mEmitterHeat - 0.5f ) * 250;
		if( Rand::randFloat() < 0.02f )
			numParticlesToSpawn *= 5;
		addParticles( numParticlesToSpawn, mEmitterLoc, mEmitterVel * per, mEmitterHeat, mEmitterRadius );
	}
	
	// add particles
	if( mouseLoc.y > 380 && mEnableConstraints && mEmitterHeat > 0.5f ){
		int depth = ( mouseLoc.y - 380 ) / 8;
		float per = depth/340.0f;
		vec3 vel = mEmitterVel * per;
		vel.y *= 0.02f;
		addParticles( depth * mEmitterHeat * 1, mEmitterLoc + vec3( 0.0f, per * mEmitterLoc.y, 0.0f ), vel, mEmitterHeat, mEmitterRadius );
	}
	
}

void ParticleController::updateParticles()
{
	static std::vector<ParticleLayout> particleVertices( MAX_PARTICLES );
	static std::vector<ParticleTailLayout> particleTailVertices( MAX_PARTICLES * MAX_TAIL_VERTICES_PER_PARTICLE );
	auto particleVertexIt = particleVertices.begin();
	auto particleTailIt = particleTailVertices.begin();
	uint32_t numAliveParticles = 0;
	
	// efficient way to remove dead particles from a vector
	mParticles.erase( std::remove_if( mParticles.begin(), mParticles.end(),
									 []( const Particle &particle ){
										 return particle.mIsDead;
									 }),
					 mParticles.end() );
	
	for( auto particleIt = mParticles.begin(); particleIt != mParticles.end(); ++particleIt ) {
		
		if( particleIt->mIsBouncing ){
			if( Rand::randFloat() < 0.025f && !particleIt->mIsDying ){
				mParticles.push_back( Particle( particleIt->mLoc[0], vec3() ) );
				mParticles.back().mIsDying = true;
				particleIt->mIsDying = true;
				//particleIt->mVel += Rand::randVec3() * Rand::randFloat( 2.0f, 3.0f );
			}
		}
		
		if( mEnablePerlin )
			applyPerlin( *particleIt );
		if( mEnableGravity )
			applyGravity( *particleIt );
		if( mEnableConstraints ) {
			applyParticleConstraints( *particleIt );
		}
		
		particleIt->move();
		particleIt->update();
		particleIt->finish();
		
		if( mRenderParticles ) {
			// First capture alive particles
			auto particleVert = particleVertexIt++;
			particleVert->position = particleIt->mLoc[0];
			particleVert->color = particleIt->mColor;
			particleVert->radius = particleIt->getHeatedRadius( mEmitterHeat );
		}
		
		if( mRenderTrails ) {
			// Next capture the tail
			auto & locations = particleIt->mLoc;
			auto radius = particleIt->mRadius;
			auto age = particleIt->mAgePer;
			
			int numTailVerts = 0;
			for( int i = 0; i < (locations.size()) - 2 && numTailVerts <= MAX_TAIL_VERTICES_PER_PARTICLE; i++ ){
				float per	= i / (float)(locations.size()-1);
				
				ci::vec3 perp0	= ci::vec3( locations[i].x, locations[i].y, 0.0f ) -
				ci::vec3( locations[i+1].x, locations[i+1].y, 0.0f );
				ci::vec3 perp1	= cross( perp0, ci::vec3( 0, 0, 1 ) );
				ci::vec3 perp2	= cross( perp0, perp1 );
				perp1			= normalize( cross( perp0, perp2 ) );
				
				ci::vec3 off	= perp1 * ( radius * ( 1.0f - per ) * 0.25f  );
				
				auto tailColor = ColorA( ( 1.0f - per ) * 0.75f, 0.15f, per * 0.5f, ( 1.0f - per ) * age * 0.25f );
				
				auto tailVert0 = particleTailIt++;
				tailVert0->position = locations[i] - off;
				tailVert0->color = tailColor;
				++numTailVerts;
				
				auto tailVert1 = particleTailIt++;
				tailVert1->position = locations[i] + off;
				tailVert1->color = tailColor;
				numTailVerts += 2;
			}
		}
		
		++numAliveParticles;
	}
	
	if( mRenderParticles )
		mParticleRender->bufferParticles( numAliveParticles, particleVertices );
	if( mRenderTrails )
		mParticleRender->bufferParticleTails( numAliveParticles, particleTailVertices );
}

void ParticleController::addParticles( int amt, vec3 loc, vec3 vel, float heat, float radius )
{
	for( int i = 0; i < amt; i++ ) {
		if( mParticles.size() > MAX_PARTICLES ) break;
		
		vec3 lOffset = Rand::randVec3();
		vec3 l = loc + lOffset * radius * 0.25f;
		vec3 v = -vel + lOffset * Rand::randFloat( 6.0f, 10.5f ) * ( heat + 0.75f ) + Rand::randVec3() * Rand::randFloat( 1.0f, 2.0f );
		v.y *= 0.65f;
		mParticles.emplace_back( l, v );
	}
}

void ParticleController::render()
{
	mEmitterRender->renderEmitter( *this );
	gl::ScopedDepth scopeDepth( true, false );
	if( mRenderParticles )
		mParticleRender->renderParticles();
	if( mRenderTrails )
		mParticleRender->renderTrails();
}
