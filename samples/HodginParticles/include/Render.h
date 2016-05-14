//
//  ParticleRender.h
//  HodginParticlesRedux
//
//  Created by ryan bartley on 7/6/15.
//
//

#include "cinder/gl/gl.h"

const uint32_t MAX_PARTICLES = 10000;
const uint32_t MAX_TAIL_VERTICES_PER_PARTICLE = 26;
const uint32_t PRIMITIVE_RESTART_INDEX = std::numeric_limits<uint32_t>::max();

struct ParticleLayout {
	ci::vec3 position;
	ci::vec4 color;
	float radius;
};

struct ParticleTailLayout {
	ci::vec3 position;
	ci::vec4 color;
};

class ParticleRender {
public:
	ParticleRender() = default;
	~ParticleRender() = default;
	
	void setup();
	void renderParticles();
	void renderTrails();
	
	void bufferParticles( uint32_t numActiveParticles, const std::vector<ParticleLayout> &activeParticles );
	void bufferParticleTails( uint32_t numActiveParticles, const std::vector<ParticleTailLayout> &particleTails );
	
	float mParticleScreenSize;
	
private:
	ci::gl::BatchRef	mParticleBatch;
	ci::gl::VboRef		mParticleBuffer;
	ci::gl::TextureRef	mParticleTexture;
	uint32_t			mNumActiveParticles;
	
	ci::gl::BatchRef	mTailBatch;
	ci::gl::VboRef		mTailBuffer;
};

// Forward Declaration
class ParticleController;

class EmitterRender {
public:
	EmitterRender() = default;
	~EmitterRender() = default;
	
	void setup();
	void renderEmitter( ParticleController &controller );
	
private:
	ci::gl::BatchRef		mSphere;
	ci::gl::GlslProgRef		mEmitterGlsl;
	ci::gl::TextureRef		mDiffuseTex;
	ci::gl::TextureRef		mNormalTex;
	ci::gl::TextureRef		mAoTex;
	ci::gl::TextureRef		mHeightTex;
	ci::gl::TextureRef		mReflOccTex;
};

// Forward Declaration.
class Constraint;

class RoomRender {
public:
	RoomRender() = default;
	~RoomRender() = default;
	
	void setup( const std::vector<Constraint*> &constraints );
	void renderRoom( ParticleController &controller );
	
private:
	ci::gl::BatchRef	mRoom;
};