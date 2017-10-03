#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#ifdef WIN32
#include <GL/glew.h>
#endif

#include <GL/glut.h>

#include "common.h"
#include "gettimeofday.h"
#include "math.h"
#include "particles.h"

#include "render.h"
#include "texture.h"

Particle::Particle() {
	pos[0] = pos[1] = pos[2] = 0;
	startcolor[0] = startcolor[1] = startcolor[2] = startcolor[3] = 0;
	endcolor[0] = endcolor[1] = endcolor[2] = endcolor[3] = 0;
	move[0] = move[1] = move[2] = 0;
	movespeed = 0;
	spinspeed = 0;
	currentsize = 0;
	startsize = 0;
	endsize = 0;
	ttl = 0;
	remainingttl = 0;
}

Particle::~Particle() {
}



ParticleEmitter::ParticleEmitter() {
	pos[0] = pos[1] = pos[2] = 0;
	type = PARTICLE_EMITTER_FIRE;
	storedTime = 0;
}

ParticleEmitter::~ParticleEmitter() {
}




ParticleSystem::ParticleSystem() {
	particlesNum = 0;
	particleEmitterList = new ParticleEmitterList();
}

ParticleSystem::~ParticleSystem() {
	particleEmitterList->clear(0);
	delete particleEmitterList;
}

void ParticleSystem::Render() {
	float modelview[16];
	Particle *p;
	int i;
	Texture3DInfo *particleTexture;
	Texture3DSubInfo *ct;

	particleTexture = (Texture3DInfo *)texture->FindTexture((char *)"ParticleTextures");
	if (particleTexture) {
		ct = texture->FindTexture3DSub(particleTexture, (char*)"particle2.tga");
		if (ct) {
			vector look, up, right;
			vector cf, cu, cr;
			float pd;

			texture->Bind3D(particleTexture->texnum, 0);

			Math::AngleVectors(render->camera.viewangles, cf, cr, cu);

			i = 0;
			p = particles;
			while(p && i < particlesNum) {
				glPushMatrix();

				// Calculate matrix for billboarding
				Math::VectorSubtract(render->camera.pos, p->pos, look);
				Math::NormalizeVector(look);
				Math::VectorCrossProduct(cu, look, right);
				Math::VectorCrossProduct(look, right, up);

				modelview[0] = right[0];
				modelview[1] = right[1];
				modelview[2] = right[2];
				modelview[3] = 0;
				modelview[4] = up[0];
				modelview[5] = up[1];
				modelview[6] = up[2];
				modelview[7] = 0;
				modelview[8] = look[0];
				modelview[9] = look[1];
				modelview[10] = look[2];
				modelview[11] = 0;
				modelview[12] = p->pos[0];
				modelview[13] = p->pos[1];
				modelview[14] = p->pos[2];
				modelview[15] = 1;
				glMultMatrixf(modelview);

				// glTranslatef(p->shiftpos[0], p->shiftpos[1], p->shiftpos[2]);
				glRotatef(p->spin, 0, 0, 1);
			
				pd = (0.5f + ct->depth) / particleTexture->GetMaxDepth();
				// render particle
				glColor4fv(p->currentcolor);
				glBegin(GL_QUADS);
			
				glTexCoord3f(0, 0, pd);
				glVertex3f(-p->currentsize, p->currentsize, 0);
				glTexCoord3f(1.0, 0.0, pd);
				glVertex3f(p->currentsize, p->currentsize, 0);
				glTexCoord3f(1.0, 1.0, pd);
				glVertex3f(p->currentsize, -p->currentsize, 0);
				glTexCoord3f(0.0, 1.0, pd);
				glVertex3f(-p->currentsize, -p->currentsize, 0);
				glEnd();

				glPopMatrix();
		
				p++;
				i++;
			}
		}
	}
}

void ParticleSystem::DoTick(timeval difftime) {
	Particle *p;
	int i;
	int overall_msecs;
	float cur_move;
	float ttlchange;
	ParticleEmitter *pe;
	dynamicReaderFree(per, ParticleEmitter);

	per->attach(particleEmitterList);

	overall_msecs = difftime.tv_sec * 1000 + difftime.tv_usec / 1000;

	// Making new particles from emitters
	pe = per->getFirstElement();
	while(pe) {
		int launchTime = 1000;

		pe->storedTime += overall_msecs;
		if (pe->type == ParticleEmitter::ParticleEmitterType::PARTICLE_EMITTER_FIRE) { // launch particle each 100 msecs
			launchTime = 20;
		}
		while(pe->storedTime >= launchTime) {
			pe->storedTime -= launchTime;
				
			if (particlesNum < MAX_PARTICLES) {
				p = particles + particlesNum;

				if (pe->type == ParticleEmitter::ParticleEmitterType::PARTICLE_EMITTER_FIRE)
					CreateFireParticle(p, pe->pos);

				p->remainingttl = p->ttl + (overall_msecs - pe->storedTime);

				particlesNum++;
			}
		}
		pe = per->getNextElement();		
	}

	// moving particles
	p = particles;
	i = 0;
	while(p && i < particlesNum) {
		p->remainingttl -= overall_msecs;

		if (p->remainingttl <= 0) {
			memmove(p, p + 1, (particlesNum - 1 - (p - particles)) * sizeof(Particle));
			particlesNum--;
		} else {
			ttlchange = (float)(p->ttl - p->remainingttl) / (float)p->ttl;

			cur_move = overall_msecs / 1000.0f;
			p->pos[0] = p->pos[0] + p->move[0] * cur_move;
			p->pos[1] = p->pos[1] + p->move[1] * cur_move;
			p->pos[2] = p->pos[2] + p->move[2] * cur_move;

			cur_move = p->spinspeed * overall_msecs / 1000;
			p->spin = p->spin + cur_move;

			p->currentcolor[0] = p->startcolor[0] + (p->endcolor[0] - p->startcolor[0]) * ttlchange;
			p->currentcolor[1] = p->startcolor[1] + (p->endcolor[1] - p->startcolor[1]) * ttlchange;
			p->currentcolor[2] = p->startcolor[2] + (p->endcolor[2] - p->startcolor[2]) * ttlchange;
			p->currentcolor[3] = p->startcolor[3] + (p->endcolor[3] - p->startcolor[3]) * ttlchange;

			p->currentsize = p->startsize + (p->endsize - p->startsize) * ttlchange;
			p++;
			i++;
		}
	}
}

ParticleEmitter *ParticleSystem::CreateEmitter(vector pos, int type) {
	ParticleEmitter *pe;

	pe = new ParticleEmitter();
	Math::VectorCopy(pos, pe->pos);

	pe->type = type;

	particleEmitterList->addElement(pe);
	
	return pe;
}

void ParticleSystem::ModifyEmitter(ParticleEmitter *emitter, vector pos) {
	if (emitter) {
		Math::VectorCopy(pos, emitter->pos);
	}
}

void ParticleSystem::RemoveEmitter(ParticleEmitter *emitter) {
	if (emitter)
		particleEmitterList->removeElement(emitter, 1);
}

void ParticleSystem::CreateFireParticle(Particle *p, vector pos) {
	Math::VectorCopy(pos, p->pos);

	p->startcolor[0] = 1.0;
	p->startcolor[1] = 1.0;
	p->startcolor[2] = 1.0;
	p->startcolor[3] = 1.0;

	p->endcolor[0] = 0.0;
	p->endcolor[1] = 1.0;
	p->endcolor[2] = 1.0;
	p->endcolor[3] = 0.0;

	p->startsize = 0.5;
	p->endsize = 0.5;

	p->ttl = 1000 + (rand() % 500 - 250);
	p->remainingttl = p->ttl;

	p->spin = 0;

	p->move[0] = (float)(rand() % 201 - 100) / 100.0f * (float)(rand() % 10) / 10.0f;
	p->move[1] = (float)(rand() % 201 - 100) / 100.0f * (float)(rand() % 10) / 10.0f;
	p->move[2] = 4;

	p->spinspeed = 360;
}

