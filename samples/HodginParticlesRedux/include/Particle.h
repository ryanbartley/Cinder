
#pragma once

#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Rand.h"

#include <vector>
#include <queue>

class Particle {
 public:
	Particle( ci::vec3 aLoc, ci::vec3 aVel );
	
	inline void move();
	inline void update();
	inline void finish() { mIsBouncing = false; }
	
	inline float getHeatedRadius( float heat )
	{
		float radius = mRadius;
		if( mAge == 1 && ! mIsDying && heat > 0.3f )
			radius *= 20.0f * heat;
		return radius * 2.0f;
	}
	
	std::vector<ci::vec3> mLoc;
	ci::ColorA	mColor;
	ci::vec3	mVel;
	ci::vec3	mAcc;
	int			mLen;
	float		mInvLen;

	float		mAge;
	float		mLifeSpan;
	float		mAgePer;
	
	float		mRadius;
	float		mMass;
	float		mInvMass;
	float		mCharge;
	
	bool		mIsBouncing;
	bool		mIsDying;
	bool		mIsDead;
};

void Particle::move()
{
	if( mVel != ci::vec3() )
		mVel += mAcc;
	
	for( int i=mLen-1; i>0; i-- ) {
		mLoc[i] = mLoc[i-1];
	}
	
	mLoc[0] += mVel;
}


void Particle::update()
{
	mVel *= 0.975f;
	mAcc = ci::vec3();
	
	mAge ++;
	if( mAge > mLifeSpan ){
		mIsDead = true;
	} else {
		mAgePer = 1.0f - mAge/mLifeSpan;
	}
	
	if( mIsDying && mVel == ci::vec3() ){
		mColor = ci::ColorA( mAgePer * 0.5f, mAgePer * 0.35f, 1.0f - mAgePer, mAgePer + ci::Rand::randFloat( 0.25f ) );
	} else {
		mColor = ci::ColorA( mAgePer, mAgePer * 0.75f, 1.0f - mAgePer + 0.15f, mAgePer + ci::Rand::randFloat( 0.5f ) );
	}
}
