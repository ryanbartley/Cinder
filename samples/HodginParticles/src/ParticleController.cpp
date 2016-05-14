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
		
		particleIt->update();
		
		if( mRenderParticles ) {
			// First capture alive particles
			particleVertexIt->position = particleIt->mCurrentLoc;
			particleVertexIt->color = particleIt->mColor;
			particleVertexIt->radius = particleIt->getHeatedRadius( mEmitterHeat );
			++particleVertexIt;
		}
		
		if( mRenderTrails ) {
			// Next capture the tail
			auto & locations = particleIt->mLoc;
			auto radius = particleIt->mRadius;
			auto age = particleIt->mAgePer;
			
			int numTailVerts = 0;
			float size = locations.size() - 1;
			ci::vec3 lastPoint = particleIt->mCurrentLoc;
			
			for( int i = 0, end = size; i < end; i++ ){
				float per = i / size;
				
				auto nextPoint = locations[i];
				
				auto dir = normalize( nextPoint - lastPoint );
				ci::vec3 perp0( dir.y, -dir.x, 0 );
				
				ci::vec3 off = perp0 * ( radius * ( 1.0f - per ) * 0.25f  );
				
				auto tailColor = ColorA( ( 1.0f - per ) * 0.75f, 0.15f, per * 0.5f, ( 1.0f - per ) * age * 0.25f );
				
				particleTailIt->position = lastPoint - off;
				particleTailIt->color = tailColor;
				++particleTailIt;
				++numTailVerts;
				
				particleTailIt->position = lastPoint + off;
				particleTailIt->color = tailColor;
				++particleTailIt;
				++numTailVerts;
				
				lastPoint = nextPoint;
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
	if( mRenderParticles )
		mParticleRender->renderParticles();
	if( mRenderTrails )
		mParticleRender->renderTrails();
}
