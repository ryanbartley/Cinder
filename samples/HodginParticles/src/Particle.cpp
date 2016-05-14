#include "Particle.h"
#include "cinder/gl/gl.h"


using namespace ci;

Particle::Particle( ci::vec3 aLoc, ci::vec3 aVel )
: mCurrentLoc( aLoc ), mLoc( 14, aLoc ), mColor(), mVel( aVel), mAcc( 0 ), mLen( mLoc.size() ),
	mInvLen( 1.0f / (float)mLen ), mAge( 0.0f ), mAgePer( 1.0f ),
	mLifeSpan( ci::Rand::randFloat( 5.0f, 70.0f ) + (ci::Rand::randFloat() < 0.15f ? 100.0f : 0.0f ) ),
	mRadius( ci::Rand::randFloat( 1.0f, 3.0f ) * (ci::Rand::randFloat() < 0.01f ? 4.0f : 1.0f) ),
	mMass( mRadius ), mInvMass( 1.0f / mMass ), mCharge( ci::Rand::randFloat( 0.35f, 0.75f ) ),
	mIsBouncing( false ), mIsDying( false ), mIsDead( false )
{

}







