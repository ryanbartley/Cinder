
#pragma once
#include "Particle.h"
#include "cinder/gl/Texture.h"
#include "cinder/Perlin.h"
#include <list>
#include <vector>

// Forward Declarations
class ParticleRender;
class EmitterRender;
class RoomRender;

struct Constraint {
	ci::vec3	mNormal;
	float		mMinValue, mMaxValue;
};

class ParticleController {
  public:
	
	ParticleController();
	void setup();
	void update( const ci::vec3 &mouseLoc, bool mouseIsDown );
	void updateEmitter( const ci::vec3 &mouseLoc, bool mouseIsDown );
	void updateParticles();
	void render();

	void addParticles( int amt, ci::vec3 loc, ci::vec3 vel, float heat, float radius );
	void createConstraints( ci::vec2 windowDim );
	
	inline void applyEmmiterConstraints();
	inline void applyParticleConstraints( Particle &particle );
	
	//! Applies a Perlin Noise force to particle.
	inline void applyPerlin( Particle &particle );
	//! Applies a Gravitational force to the particle.
	inline void applyGravity( Particle &particle );
	
	std::vector<Particle>		mParticles;
	std::vector<Constraint*>	mConstraints;
	
	// Emitter info
	ci::vec3			mEmitterLoc, mEmitterVel;
	float				mEmitterRadius, mEmitterRadiusDest,
						mEmitterHeat, mEmitterSpinSpeed;
	ci::vec3			mCurrentMouseVel, mCurrentMousePos;
	
	// Perlin info
	ci::Perlin	mPerlin				= ci::Perlin(3);
	float		mPerlinMagnitude	= 0.5f;
	// Gravity info
	ci::vec3	mGravitationalAxis	= ci::vec3( 0, 0.1, 0 );
	// Repulsion info
	float		mRepulsionMagnitude	= 10.0f;
	
	// Render Info
	EmitterRender*	mEmitterRender;
	ParticleRender*	mParticleRender;
	RoomRender*		mRoomRender;
	
	// Assorted Flags
	bool		mEnableConstraints, mEnableGravity,
				mEnablePerlin, mEnableRepulsion,
				mRenderParticles, mRenderTrails;
	bool		mIsTouchingConstraint;
};

void ParticleController::applyPerlin( Particle &particle )
{
	auto counter = ci::app::getElapsedFrames();
	// Perlin Calculation
	ci::vec3 noise = mPerlin.dfBm( particle.mLoc[0] * 0.01f +
								  ci::vec3( 0, 0, counter / 100.0f ) );
	noise = normalize( noise );
	noise *= mPerlinMagnitude;
	particle.mAcc += ci::vec3( noise.x, noise.y, noise.z );
}

void ParticleController::applyGravity( Particle &particle )
{
	// Gravity Calculation
	particle.mAcc += mGravitationalAxis * particle.mMass;
}

void ParticleController::applyEmmiterConstraints()
{
	for( auto c : mConstraints ) {
		if( c->mNormal.x > 0.0f ){
			auto tempX = ci::constrain( mEmitterLoc.x, c->mMinValue, c->mMaxValue );
			if( tempX != mEmitterLoc.x ) {
				mEmitterLoc.x = tempX;
				mIsTouchingConstraint = true;
			}
		}
		
		if( c->mNormal.y > 0.0f ){
			auto tempY = ci::constrain( mEmitterLoc.y, c->mMinValue, c->mMaxValue );
			if( tempY != mEmitterLoc.y ) {
				mEmitterLoc.y = tempY;
				mIsTouchingConstraint = true;
			}
		}
		
		if( c->mNormal.z > 0.0f ){
			auto tempZ = ci::constrain( mEmitterLoc.z, c->mMinValue, c->mMaxValue );
			if( tempZ != mEmitterLoc.z ) {
				mEmitterLoc.z = tempZ;
				mIsTouchingConstraint = true;
			}
		}
	}
}

void ParticleController::applyParticleConstraints( Particle &particle )
{
	for( auto c : mConstraints ) {
		float velMulti = ci::Rand::randFloat( -0.5f, -0.1f );
		auto & pLoc = particle.mLoc[0];
		if( c->mNormal.x > 0.0f ){
			auto tempX = ci::constrain( pLoc.x, c->mMinValue, c->mMaxValue );
			if( tempX != pLoc.x ) {
				pLoc.x = tempX;
				particle.mVel.x *= velMulti;
				particle.mIsBouncing = true;
			}
		}
		
		if( c->mNormal.y > 0.0f ){
			auto tempY = ci::constrain( pLoc.y, c->mMinValue, c->mMaxValue );
			if( tempY != pLoc.y ) {
				pLoc.y = tempY;
				particle.mVel.y *= velMulti;
				particle.mIsBouncing = true;
			}
		}
		
		if( c->mNormal.z > 0.0f ){
			auto tempZ = ci::constrain( pLoc.z, c->mMinValue, c->mMaxValue );
			if( tempZ != pLoc.z ) {
				pLoc.z = tempZ;
				particle.mVel.z *= velMulti;
				particle.mIsBouncing = true;
			}
		}
	}
}

