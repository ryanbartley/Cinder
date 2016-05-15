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
using namespace std;

const static float RADIUS_MIN = 10.0f;
const static float RADIUS_MAX = 30.0f;

ParticleController::Particle::Particle( ci::vec3 aLoc, ci::vec3 aVel )
: mCurrentLoc( aLoc ), mColor(), mVel( aVel), mAcc( 0 ), mAgePer( 1.0f ),
mLifeSpanInc( 1.0f / (ci::Rand::randFloat( 5.0f, 70.0f ) + (ci::Rand::randFloat() < 0.15f ? 100.0f : 0.0f )) ),
mRadius( ci::Rand::randFloat( 1.0f, 3.0f ) * (ci::Rand::randFloat() < 0.01f ? 4.0f : 1.0f) ),
mMass( mRadius ), mInvMass( 1.0f / mMass ), mIsBouncing( false ), mIsDying( false )
{
	mLoc.fill( mCurrentLoc );
}

ParticleController::ParticleController()
: mEnablePerlin( true ), mEnableGravity( false ), mEnableConstraints( true ),
	mEnableRepulsion( false ), mIsTouchingConstraint( false ), mParticleRender( new ParticleRender ),
	mEmitterRender( new EmitterRender ), mRoomRender( new RoomRender ),
	mRenderParticles( true ), mRenderTrails( true )
{
	mEmitterPos		= vec3( 0.0f );
	mEmitterVel		= vec3( 0.0f );
	mEmitterAcc		= vec3( 0.0f );
	mEmitterAxis	= vec3( 0.0f, 1.0f, 0.0f );
	
	mEmitterRadius		= RADIUS_MAX;
	mEmitterRotSpeed	= 0.0f;
	mEmitterRotAngle	= 0.0f;
	
	int i = 0;
	for( auto & color : tailColors ) {
		auto per = static_cast<float>(i++) / tailColors.size();
		auto flippedPer = 1.0f - per;
		color = ColorA( flippedPer * 0.75f, 0.15f, per * 0.5f, flippedPer * 0.25f );
	}
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

void ParticleController::update( const Ray &ray, bool mouseDown )
{
	updateEmitter( ray, mouseDown );
	updateParticles();
}

void ParticleController::updateEmitter( const Ray &ray, bool mouseDown )
{
	if( mouseDown ){
		mEmitterRotSpeed	-= ( mEmitterRotSpeed - 1.0f ) * 0.05f;
		mEmitterRadius		-= ( mEmitterRadius - RADIUS_MIN ) * 0.05f;
	} else {
		mEmitterRotSpeed	-= ( mEmitterRotSpeed - 0.01f ) * 0.05f;
		mEmitterRadius		-= ( mEmitterRadius - RADIUS_MAX ) * 0.05f;
	}
	
	mEmitterHeat -= ( mEmitterHeat - mEmitterRotSpeed ) * 0.1f;
	
	float rayPlaneDist;
	ray.calcPlaneIntersection( vec3( 0.0f ), vec3( 0.0f, 0.0f, -1.0f ), &rayPlaneDist );
	vec3 newPos	= ray.getOrigin() + ray.getDirection() * rayPlaneDist;
	
	mEmitterPos -= ( mEmitterPos - newPos ) * 0.2f;
	
	mEmitterRotAngle += mEmitterRotSpeed;
	
	mEmitterMatrix = mat4();
	mEmitterMatrix = translate( mEmitterMatrix, mEmitterPos );
	mEmitterMatrix = scale( mEmitterMatrix, vec3( mEmitterRadius ) );
	mEmitterMatrix = rotate( mEmitterMatrix, mEmitterRotAngle, mEmitterAxis );
	
	if( mEnableConstraints ) {
		applyEmmiterConstraints();
	}
	
	// add particles
	if( mouseDown && mEmitterHeat > 0.5f ){
		int depth = newPos.y - 380;
		float per = depth/340.0f;
		vec3 vel = mEmitterVel * per;
		vel.y *= 0.02f;
		int numParticlesToSpawn = ( mEmitterHeat - 0.5f ) * 250;
		if( Rand::randFloat() < 0.02f )
			numParticlesToSpawn *= 5;
		addParticles( numParticlesToSpawn, mEmitterPos, mEmitterVel * per, mEmitterHeat, mEmitterRadius );
	}
	
	// add particles
	if( newPos.y > 380 && mEnableConstraints && mEmitterHeat > 0.5f ){
		int depth = ( newPos.y - 380 ) / 8;
		float per = depth/340.0f;
		vec3 vel = mEmitterVel * per;
		vel.y *= 0.02f;
		addParticles( depth * mEmitterHeat * 1, mEmitterPos + vec3( 0.0f, per * mEmitterPos.y, 0.0f ), vel, mEmitterHeat, mEmitterRadius );
	}
	
}

void ParticleController::updateParticles()
{
	// efficient way to remove dead particles from a vector
	mParticles.erase( std::remove_if( mParticles.begin(), mParticles.end(),
									 []( const Particle &particle ){
										 return particle.mAgePer <= 0.0f;
									 }),
					 mParticles.end() );
	
	ParticleLayout* particleVertexIt = nullptr; //particleVertices.begin();
	if( mRenderParticles )
		particleVertexIt = mParticleRender->mapParticles( mParticles.size() );
	ParticleTailLayout* particleTailIt = nullptr;// particleTailVertices.begin();
	if( mRenderTrails )
		particleTailIt = mParticleRender->mapParticleTails( mParticles.size() );
	
	uint32_t numAliveParticles = 0;
	
	for( auto particleIt = mParticles.begin(), end = mParticles.end(); particleIt != end; ++particleIt ) {
		
		if( particleIt->mIsBouncing ){
			if( Rand::randFloat() < 0.025f && !particleIt->mIsDying ){
				mParticles.emplace_back( particleIt->mLoc[0], vec3() );
				mParticles.back().mIsDying = true;
				particleIt->mIsDying = true;
				//particleIt->mVel += Rand::randVec3() * Rand::randFloat( 2.0f, 3.0f );
			}
		}
		
		if( mEnablePerlin ) {
			applyPerlin( *particleIt );
		}
		if( mEnableGravity )
			applyGravity( *particleIt );
		if( mEnableConstraints ) {
			applyParticleConstraints( *particleIt );
		}
		
		particleIt->mVel += particleIt->mAcc;
		auto& loc = particleIt->mLoc;
		for( int i=PARTICLE_HISTORY_LEN - 1; i>0; i-- ) {
			loc[i] = loc[i-1];
		}
		loc[0] = particleIt->mCurrentLoc;
		
		particleIt->mCurrentLoc += particleIt->mVel;
		
		particleIt->mVel *= 0.975f;
		particleIt->mAcc = ci::vec3();
		
		particleIt->mAgePer -= particleIt->mLifeSpanInc;
		
		if( particleIt->mIsDying && particleIt->mVel == ci::vec3() ) {
			particleIt->mColor = ci::ColorA( particleIt->mAgePer * 0.5f, particleIt->mAgePer * 0.35f, 1.0f - particleIt->mAgePer, particleIt->mAgePer + ci::Rand::randFloat( 0.25f ) );
		} else {
			particleIt->mColor = ci::ColorA( particleIt->mAgePer, particleIt->mAgePer * 0.75f, 1.0f - particleIt->mAgePer + 0.15f, particleIt->mAgePer + ci::Rand::randFloat( 0.5f ) );
		}
		
		particleIt->mIsBouncing = false;
		
		if( mRenderParticles ) {
			// First capture alive particles
			particleVertexIt->position = particleIt->mCurrentLoc;
			particleVertexIt->color = particleIt->mColor;
			float radius = particleIt->mRadius;
			if( particleIt->mAgePer == particleIt->mLifeSpanInc && ! particleIt->mIsDying && mEmitterHeat > 0.3f )
				radius *= 20.0f * mEmitterHeat;
			particleVertexIt->radius = radius * 2.0f;
			++particleVertexIt;
		}
		
		if( mRenderTrails ) {
			// Next capture the tail
			auto radius = particleIt->mRadius;
			auto age = particleIt->mAgePer;
			
			float size = loc.size() - 1;
			ci::vec3 lastPoint = particleIt->mCurrentLoc;
			auto increment = 1 / size;
			
			for( int i = 0, end = size; i < end; i++ ){
				float flippedPer = 1.0f - (i * increment);
				
				auto nextPoint = loc[i];
				
				auto dir = normalize( nextPoint - lastPoint );
				ci::vec3 perp0( dir.y, -dir.x, 0 );
				ci::vec3 off = perp0 * ( radius * flippedPer * 0.25f  );
				
				particleTailIt->position = lastPoint - off;
				particleTailIt->color = tailColors[i];
				particleTailIt->color.a *= age;
				++particleTailIt;
				
				particleTailIt->position = lastPoint + off;
				particleTailIt->color = tailColors[i];
				particleTailIt->color.a *= age;
				++particleTailIt;
				
				lastPoint = nextPoint;
			}
		}
		++numAliveParticles;
	}
	
	if( mRenderParticles )
		mParticleRender->unmapParticles();//mParticleRender->bufferParticles( numAliveParticles, particleVertices );
	if( mRenderTrails )
		mParticleRender->unmapParticleTails();//mParticleRender->bufferParticleTails( numAliveParticles, particleTailVertices );
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
	if( mRenderParticles )
		mParticleRender->renderParticles();
	if( mRenderTrails )
		mParticleRender->renderTrails();
}
