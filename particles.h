class Particle {
public:
	Particle();
	~Particle();

	vector pos;
	vector move;
	float movespeed;

	float spin;
	float spinspeed;

	color startcolor;
	color endcolor;
	color currentcolor;

	float startsize;
	float endsize;
	float currentsize;

	int ttl;
	int remainingttl;
};

class ParticleEmitter {
public:
	ParticleEmitter();
	~ParticleEmitter();

	enum ParticleEmitterType {
		PARTICLE_EMITTER_FIRE
	};

	vector pos;
	int type;
	int storedTime;
};

class ParticleEmitterList : public List<ParticleEmitter> {
};

class ParticleSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void DoTick(timeval difftime);
	void Render();

	ParticleEmitter *CreateEmitter(vector pos, int type);
	void ModifyEmitter(ParticleEmitter *emitter, vector pos);
	void RemoveEmitter(ParticleEmitter *emitter);
	
protected:
	Particle particles[MAX_PARTICLES];
	int particlesNum;
	ParticleEmitterList *particleEmitterList;

	void CreateFireParticle(Particle *p, vector pos);
};

