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
#include "light.h"
#include "render.h"
#include "texture.h"
#include "level.h"

// removed normals for a while
#define VALUES_PER_VERT 6
#define NORMALS_OFFSET 3
#define TEXCOORDS_OFFSET 3

char scheme[10][10] = {
	"...#....#",
	".L.#.R#.#",
	"##.#..#G#",
	".#.#..#.#",
	".#.....##",
	".#.#.W#.#",
	"##.#.WL.#",
	".B.####.#",
	".###....#",
	"#########"
};

Level::Level() {
	int i;
	int x, y;

	changed = -1;

	lightSources = new LightSourceList;

	particleSystem = new ParticleSystem();

	for (y=0; y<MAX_GRID_CELLS; y++)
		for (x=0; x<MAX_GRID_CELLS; x++) {
			map_tiles[y][x].obstacle = 0;
			map_tiles[y][x].levelType[GRIDLEVEL_BASE] = GRIDTILE_BLACKSTONE;
			map_tiles[y][x].levelType[GRIDLEVEL_FLOOR] = GRIDTILE_BLUESTONEPANEL;
			map_tiles[y][x].levelType[GRIDLEVEL_WALK] = GRIDTILE_EMPTY;
			map_tiles[y][x].levelType[GRIDLEVEL_AIR] = GRIDTILE_EMPTY;
			map_tiles[y][x].height = (float)(rand() % 100) / 100.0f;
		}

	for (i=0; i<MAX_GRID_CELLS; i++) {
		map_tiles[i][0].obstacle = 1;
		map_tiles[0][i].obstacle = 1;
		map_tiles[i][MAX_GRID_CELLS - 1].obstacle = 1;
		map_tiles[MAX_GRID_CELLS - 1][i].obstacle = 1;

		map_tiles[i][0].levelType[GRIDLEVEL_WALK] = GRIDTILE_STONE;
		map_tiles[0][i].levelType[GRIDLEVEL_WALK] = GRIDTILE_STONE;
		map_tiles[i][MAX_GRID_CELLS - 1].levelType[GRIDLEVEL_WALK] = GRIDTILE_STONE;
		map_tiles[MAX_GRID_CELLS - 1][i].levelType[GRIDLEVEL_WALK] = GRIDTILE_STONE;
	}

	for (y=0; y<10; y++)
		for(x=0; x<9; x++)
			if (scheme[y][x] == '#') {
				map_tiles[y+1][x+1].obstacle = 1;
				map_tiles[y+1][x+1].levelType[GRIDLEVEL_WALK] = GRIDTILE_STONE;
			}

	for (y=0; y<10; y++)
		for(x=0; x<9; x++) {
			if (scheme[y][x] == 'L' || scheme[y][x] == 'R' || scheme[y][x] == 'G' || scheme[y][x] == 'B') {
				LightSource *ls;

				ls = new LightSource();
				ls->pos[0] = (float)((x+1) * TILE_SIZE  + TILE_SIZE / 2);
				ls->pos[1] = (float)-((y+1) * TILE_SIZE  + TILE_SIZE / 2);
				ls->pos[2] = (float)TILE_SIZE / 2;
				ls->radius = 0; // not used

				if (scheme[y][x] == 'L') {
					ls->color[0] = ls->color[1] = ls->color[2] = 1;
				} else if (scheme[y][x] == 'R') {
					ls->color[0] = 1;
					ls->color[1] = ls->color[2] = 0;
				} else if (scheme[y][x] == 'G') {
					ls->color[1] = 1;
					ls->color[0] = ls->color[2] = 0;
				} else if (scheme[y][x] == 'B') {
					ls->color[0] = ls->color[1] = 0;
					ls->color[2] = 1;
				}

				lightSources->addElement(ls);
			}
		}
/*
	{
		vector pos;

		pos[0] = 48;
		pos[1] = -24;
		pos[2] = 4;
		particleSystem->CreateEmitter(pos, ParticleEmitter::PARTICLE_EMITTER_FIRE);
	}
*/

	geometryTextures = (Texture3DInfo *)texture->FindTexture((char *)"GeometryTextures");
}

Level::~Level() {
	lightSources->clear(1);
	delete particleSystem;	
}

void Level::SetChanged() {
	changed = 1;
}

LightSourceList *Level::GetLightSources() {
	return lightSources;
}

Level::GridTile *Level::GetTiles() {
	return map_tiles[0];
}

// TODO: use infMatrix
// TODO: use normal Infinity

void Level::PrepareShadowVolume(Face4 *face, LightSource *ls, float *vbo, unsigned int *count) {
	vector v[4];
	vector p[4];
	int i;
	float LIGHT_INFINITY = TILE_SIZE * 15;
	float *curVert;

	for (i=0; i<4; i++) {
		Math::VectorCopy(face->vertexes[i], p[i]);
		Math::VectorSubtract(p[i], ls->pos, v[i]);
	}

	for (i=0; i<4; i++) {
		Math::NormalizeVector(v[i]);
		v[i][0] *= LIGHT_INFINITY;
		v[i][1] *= LIGHT_INFINITY;
		v[i][2] *= LIGHT_INFINITY;
		v[i][0] += ls->pos[0];
		v[i][1] += ls->pos[1];
		v[i][2] += ls->pos[2];
	}

	curVert = vbo + (*count)*3;

	*curVert++ = p[0][0]; *curVert++ = p[0][1]; *curVert++ = p[0][2];
	*curVert++ = p[1][0]; *curVert++ = p[1][1]; *curVert++ = p[1][2];
	*curVert++ = p[2][0]; *curVert++ = p[2][1]; *curVert++ = p[2][2];
	*curVert++ = p[3][0]; *curVert++ = p[3][1]; *curVert++ = p[3][2];

	*curVert++ = v[3][0]; *curVert++ = v[3][1]; *curVert++ = v[3][2];
	*curVert++ = v[2][0]; *curVert++ = v[2][1]; *curVert++ = v[2][2];
	*curVert++ = v[1][0]; *curVert++ = v[1][1]; *curVert++ = v[1][2];
	*curVert++ = v[0][0]; *curVert++ = v[0][1]; *curVert++ = v[0][2];

	// walls
	for (i=0; i<4; i++) {
		*curVert++ = p[i][0]; *curVert++ = p[i][1]; *curVert++ = p[i][2];
		*curVert++ = v[i][0]; *curVert++ = v[i][1]; *curVert++ = v[i][2];
		*curVert++ = v[(i+1)%4][0]; *curVert++ = v[(i+1)%4][1]; *curVert++ = v[(i+1)%4][2];
		*curVert++ = p[(i+1)%4][0]; *curVert++ = p[(i+1)%4][1]; *curVert++ = p[(i+1)%4][2];
	}
	*count += 24;
}

void Level::PrepareShadowVolume(Face3 *face, LightSource *ls, float *vbo, unsigned int *count) {
	vector v[3];
	vector p[3];
	int i;
	float LIGHT_INFINITY = TILE_SIZE * 15;
	float *curVert;

	for (i=0; i<3; i++) {
		Math::VectorCopy(face->vertexes[i], p[i]);
		Math::VectorSubtract(p[i], ls->pos, v[i]);
	}

	for (i=0; i<3; i++) {
		Math::NormalizeVector(v[i]);
		v[i][0] *= LIGHT_INFINITY;
		v[i][1] *= LIGHT_INFINITY;
		v[i][2] *= LIGHT_INFINITY;
		v[i][0] += ls->pos[0];
		v[i][1] += ls->pos[1];
		v[i][2] += ls->pos[2];
	}

	curVert = vbo + (*count)*3;

	*curVert++ = p[0][0]; *curVert++ = p[0][1]; *curVert++ = p[0][2];
	*curVert++ = p[1][0]; *curVert++ = p[1][1]; *curVert++ = p[1][2];
	*curVert++ = p[2][0]; *curVert++ = p[2][1]; *curVert++ = p[2][2];

	*curVert++ = v[2][0]; *curVert++ = v[2][1]; *curVert++ = v[2][2];
	*curVert++ = v[1][0]; *curVert++ = v[1][1]; *curVert++ = v[1][2];
	*curVert++ = v[0][0]; *curVert++ = v[0][1]; *curVert++ = v[0][2];

	// walls
	for (i=0; i<3; i++) {
		*curVert++ = p[i][0]; *curVert++ = p[i][1]; *curVert++ = p[i][2];
		*curVert++ = v[i][0]; *curVert++ = v[i][1]; *curVert++ = v[i][2];
		*curVert++ = v[(i+1)%3][0]; *curVert++ = v[(i+1)%3][1]; *curVert++ = v[(i+1)%3][2];
		*curVert++ = p[(i+1)%3][0]; *curVert++ = p[(i+1)%3][1]; *curVert++ = p[(i+1)%3][2];
	}
	*count += 18;
}

void Level::RenderLighting() {
	LightSource *ls;
	dynamicReaderFree(lsr, LightSource);
	int lightTypeUniform;

	glUseProgram(render->GetShaderLighting());

	lightTypeUniform = glGetUniformLocation(render->GetShaderLighting(), (char *)"lightType");

	glDepthFunc(GL_EQUAL);
	glCullFace(GL_BACK);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (lightTypeUniform != -1)
		glUniform1i(lightTypeUniform, 0);
	{
		float lightAmbient[4];
		float lightDiffuse[4];
		float lightPosition[4];
		
		lightPosition[0] = 0;
		lightPosition[1] = 0;
		lightPosition[2] = 0;
		lightPosition[3] = 1;

		lightAmbient[0] = lightAmbient[1] = lightAmbient[2] = 0.1f;
		lightDiffuse[0] = lightDiffuse[1] = lightDiffuse[2] = 0;
		lightDiffuse[3] = lightAmbient[3] = 1;

		glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
		glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	}
		
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	Render(1);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT1);

	// Light sources
	lsr->attach(lightSources);

	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_3D);
		
	ls = lsr->getFirstElement();
	while(ls) {
		// TODO: Add scisors
		glDisable(GL_BLEND);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		if (render->GetExtensionTwoSidedStencil() == 0)
			glStencilFunc(GL_ALWAYS, 0, 0);
		else {
			glDisable(GL_CULL_FACE);
			glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			glActiveStencilFaceEXT(GL_BACK);
			glStencilOp(GL_KEEP, GL_INCR_WRAP_EXT, GL_KEEP);
			glStencilMask(0xff);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glActiveStencilFaceEXT(GL_FRONT);
			glStencilOp(GL_KEEP, GL_DECR_WRAP_EXT, GL_KEEP);
			glStencilMask(0xff);
			glStencilFunc(GL_ALWAYS, 0, 0);
		}

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, 100.0f);

		glEnableClientState( GL_VERTEX_ARRAY );
		glBindBuffer(GL_ARRAY_BUFFER, ls->lightVbo);
		glVertexPointer(3, GL_FLOAT, sizeof(float)*3, (void *)0);
		if (render->GetExtensionTwoSidedStencil() == 0) {
			glCullFace(GL_FRONT);
			glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);

			glDrawArrays(GL_QUADS, 0, ls->lightVboCount);
			glCullFace(GL_BACK);
			glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		}
		glDrawArrays(GL_QUADS, 0, ls->lightVboCount);

		glDisableClientState( GL_VERTEX_ARRAY );					// Disable Vertex Arrays
		glDisable(GL_POLYGON_OFFSET_FILL);

		if (render->GetExtensionTwoSidedStencil() == 1) {
			glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
			glEnable(GL_CULL_FACE);
		}

		glDepthFunc(GL_EQUAL);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_EQUAL, 0x0, 0xff);

		if (lightTypeUniform != -1)
			glUniform1i(lightTypeUniform, 0);
		{
			float lightPosition[4];
			float lightBlack[4];
		
			lightPosition[0] = ls->pos[0];
			lightPosition[1] = ls->pos[1];
			lightPosition[2] = ls->pos[2];
			lightPosition[3] = 1;

			lightBlack[0] = lightBlack[1] = lightBlack[2] = lightBlack[3] = 0.0;

			glLightfv(GL_LIGHT1, GL_AMBIENT, lightBlack);
			glLightfv(GL_LIGHT1, GL_DIFFUSE, ls->color);
			glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
			glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
			glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0);
			glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0);
		}
		
		// shader here
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHTING);
		Render(1);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT1);

		ls = lsr->getNextElement();
	}

	glDisable(GL_STENCIL_TEST);
	glUseProgram(0);
}

void Level::CreateVBOs() {
	int x, y, level, type;
	float *verts;
	unsigned int cur_vert;

	LightSource *ls;
	dynamicReaderFree(lsr, LightSource);

	if (changed == -1 || changed == 1) {
		if (geometryTextures == NULL)
			return;

		geometryVboCount = 0;

// count how much vertexes we require for level geometry
		for (y=0; y<MAX_GRID_CELLS; y++)
			for (x=0; x<MAX_GRID_CELLS; x++)
				for (level = GRIDLEVEL_BASE; level <= GRIDLEVEL_AIR; level++)
					for (type = GRIDFACE_TOP; type <= GRIDFACE_WEST; type++) {
						if (CheckFaceForTile(x, y, type, level))
							geometryVboCount += 4;
					}

// define vertexes
		verts = new float[geometryVboCount * VALUES_PER_VERT];
		memset(verts, 0, geometryVboCount * VALUES_PER_VERT);
		cur_vert = 0;

		for (y=0; y<MAX_GRID_CELLS; y++)
			for (x=0; x<MAX_GRID_CELLS; x++)
				for (level = GRIDLEVEL_BASE; level <= GRIDLEVEL_AIR; level++)
					for (type = GRIDFACE_TOP; type <= GRIDFACE_WEST; type++) {
						Face4 face;
						int i;

						if (CheckFaceForTile(x, y, type, level)) {
							GetFaceForTile(x, y, type, level, &face);
							for (i=0; i<4; i++) {
								verts[cur_vert * VALUES_PER_VERT] = face.vertexes[i][0];
								verts[cur_vert * VALUES_PER_VERT + 1] = face.vertexes[i][1];
								verts[cur_vert * VALUES_PER_VERT + 2] = face.vertexes[i][2];
								verts[cur_vert * VALUES_PER_VERT + TEXCOORDS_OFFSET] = face.tc[i][0];
								verts[cur_vert * VALUES_PER_VERT + TEXCOORDS_OFFSET + 1] = face.tc[i][1];
								verts[cur_vert * VALUES_PER_VERT + TEXCOORDS_OFFSET + 2] = face.tc[i][2];
								cur_vert++;
							}
						}
					}

		if (changed == -1)
			glGenBuffers(1, &geometryVbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometryVbo);
		if (changed == 1)
			glBufferData(GL_ARRAY_BUFFER, NULL, NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, geometryVboCount * VALUES_PER_VERT * sizeof(float), verts, GL_STATIC_DRAW);

		delete []verts;
	}

// light shadow volumes creation
	lsr->attach(lightSources);
	ls = lsr->getFirstElement();
	while(ls) {
		if (ls->changed == -1 || ls->changed == 1 || changed == 1 || changed == -1) {
			ls->lightVboCount = 0;

// count how much vertexes we require for current light shadow volumes
			for (y=0; y<MAX_GRID_CELLS; y++)
				for (x=0; x<MAX_GRID_CELLS; x++)
					if (sqrt(pow(x*TILE_SIZE+TILE_SIZE/2 - ls->pos[0], 2)+pow(y*TILE_SIZE+TILE_SIZE/2 + ls->pos[1], 2)) < TILE_SIZE * 8)
						for (level = GRIDLEVEL_BASE; level <= GRIDLEVEL_AIR; level++)
							for (type = GRIDFACE_TOP; type <= GRIDFACE_WEST; type++) {
								if (CheckFaceForTile(x, y, type, level)) {
									float side = 0;
									float plane[4];
									Face4 face;

									GetFaceForTile(x, y, type, level, &face);
						
									Math::PlaneEquation(face.vertexes[0], face.vertexes[1], face.vertexes[2], plane);
									side = plane[0]*ls->pos[0]+plane[1]*ls->pos[1]+plane[2]*ls->pos[2]+plane[3];
									if (side < 0) {
										ls->lightVboCount += 24;
									}
								}
							}

// utilize vertexes
			verts = new float[ls->lightVboCount * 3];
			cur_vert = 0;

			for (y=0; y<MAX_GRID_CELLS; y++)
				for (x=0; x<MAX_GRID_CELLS; x++)
					if (sqrt(pow(x*TILE_SIZE+TILE_SIZE/2 - ls->pos[0], 2)+pow(y*TILE_SIZE+TILE_SIZE/2 + ls->pos[1], 2)) < TILE_SIZE * 8)
						for (level = GRIDLEVEL_BASE; level <= GRIDLEVEL_AIR; level++)
							for (type = GRIDFACE_TOP; type <= GRIDFACE_WEST; type++) {
								if (CheckFaceForTile(x, y, type, level)) {
									float side = 0;
									float plane[4];
									Face4 face;

									GetFaceForTile(x, y, type, level, &face);
						
									Math::PlaneEquation(face.vertexes[0], face.vertexes[1], face.vertexes[2], plane);
									side = plane[0]*ls->pos[0]+plane[1]*ls->pos[1]+plane[2]*ls->pos[2]+plane[3];
									if (side < 0) {
										PrepareShadowVolume(&face, ls, verts, &cur_vert);
									}
								}
							}

			if (ls->changed == -1)
				glGenBuffers(1, &ls->lightVbo);
			glBindBuffer(GL_ARRAY_BUFFER, ls->lightVbo);
			if (changed == 1)
				glBufferData(GL_ARRAY_BUFFER, NULL, NULL, GL_STATIC_DRAW);
			glBufferData(GL_ARRAY_BUFFER, ls->lightVboCount * 3 * sizeof(float), verts, GL_STATIC_DRAW);

			ls->changed = 0;

			delete []verts;
		}

		ls = lsr->getNextElement();
	}
}

void Level::Render(int mode) {
	TextureInfo *geometryTexture;
//	TextureInfo *bumpmapTexture;

	// TODO: If mode == 0 precalculate viewLos from current camera, skeletal animation and index buffers of geometry (to render only visible parts of VBO)
	if (mode == 0) {
		if (changed == -1 || changed == 1) {
			CreateVBOs();
		}
	}
	if (mode == 2)
		changed = 0;

	glEnableClientState( GL_VERTEX_ARRAY );
	if (mode == 2) {
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
// disabled normals for a while
//		glEnableClientState( GL_NORMAL_ARRAY );
	}
	glColor4f(1,1,1,1);

	glBindBuffer(GL_ARRAY_BUFFER, geometryVbo);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*VALUES_PER_VERT, (void *)0); // offset to Vertex in buffer
	if (mode == 2) {
		geometryTexture = texture->FindTexture((char *)"GeometryTextures");
		// bumpmapTexture = texture->FindTexture((char *)"bumpmap.tga");

		if (geometryTexture) {
//			glActiveTexture(GL_TEXTURE0);
//			glClientActiveTexture(GL_TEXTURE0);
			texture->Bind3D(geometryTexture->texnum, 0);

			glTexCoordPointer(3, GL_FLOAT, sizeof(float)*VALUES_PER_VERT, (void *)(TEXCOORDS_OFFSET * sizeof(float)) ); // offset to Texs in buffer
		}
/*
		if (bumpmapTexture) {
			glActiveTexture(GL_TEXTURE1);
			glClientActiveTexture(GL_TEXTURE1);
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glBindBuffer(GL_ARRAY_BUFFER, geometryVbo);
			glEnable(GL_TEXTURE_2D);
			texture->Bind(bumpmapTexture->texnum, 1);
			glTexCoordPointer(2, GL_FLOAT, sizeof(float)*VALUES_PER_VERT, (void *)24); // offset to Texs in buffer
		}
*/
// disabled normals for a while
//		glNormalPointer(GL_FLOAT, sizeof(float)*VALUES_PER_VERT, (void *)(void *)(NORMALS_OFFSET * sizeof(float))); // offset to Norms in buffer
	}
	glDrawArrays(GL_QUADS, 0, geometryVboCount);

	glDisableClientState( GL_VERTEX_ARRAY );					// Disable Vertex Arrays
	if (mode == 2) {
		/*
		if (bumpmapTexture) {
			glDisable(GL_TEXTURE_2D);
			glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			glActiveTexture(GL_TEXTURE0);
			glClientActiveTexture(GL_TEXTURE0);
		}
		*/
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );				// Disable Texture Coord Arrays
// disabled normals for a while
//		glDisableClientState( GL_NORMAL_ARRAY );
	}
}

void Level::RenderParticles() {
	particleSystem->Render();
}

bool Level::CheckCollision(vector initpos, vector move, vector newpos, float radius, timeval difftime) {
	int x, y;
	int noCollision = 0;
	int type, level;
	int zx, zy;
	vector pos;
	vector bestIntersect;
	float bestDistance;

	Math::VectorCopy(initpos, pos);

	while(noCollision == 0) {
		noCollision = 1;
		bestDistance = 10000.0;

		Math::VectorAdd(pos, move, newpos);

		for (y=(int)(-newpos[1]-radius)/TILE_SIZE; y<=(int)(-newpos[1]+radius)/TILE_SIZE; y++)
			for (x=(int)(newpos[0]-radius)/TILE_SIZE; x<=(int)(newpos[0]+radius)/TILE_SIZE; x++)
				for (level = GRIDLEVEL_BASE; level <= GRIDLEVEL_AIR; level++)
					for (type = GRIDFACE_TOP; type <= GRIDFACE_WEST; type++) {
						vector intersect;
						float distance;
						Face4 face;

						if (CheckFaceForTile(x, y, type, level)) {
							GetFaceForTile(x, y, type, level, &face);

							distance = Math::CheckCollisionQuad(face.vertexes[0], face.vertexes[1], face.vertexes[2], face.vertexes[3], pos, move, newpos, radius, intersect);
							if (distance >= 0.0 && distance < bestDistance) {
								Math::VectorCopy(intersect, bestIntersect);
								bestDistance = distance;
								zx = x;
								zy = y;
							}
						}
					}

		if (bestDistance < 10000) {
			vector newmove;
			vector nearpos;

			noCollision = 0;
			Math::MakeSlideAfterCollision(pos, move, newpos, bestIntersect, bestDistance, newmove, nearpos);

			if (fabs(newmove[0]) < 0.001 && fabs(newmove[1]) < 0.001 && fabs(newmove[2]) < 0.001)
				noCollision = 1;

			Math::VectorCopy(nearpos, pos);
			Math::VectorCopy(newmove, move);
		}
	}

	Math::VectorAdd(pos, move, newpos);
	return false;
}

void Level::GetFaceForTile(int x, int y, int type, int level, Face4 *face) {
	float sx[2], sy[2], sz[4];
	float tc[2];

	if (x < 0 || y < 0 || x >= MAX_GRID_CELLS || y >= MAX_GRID_CELLS || level < 0 || level > GRIDLEVEL_AIR)		
		return;

	sx[0] = (float)x*TILE_SIZE;
	sx[1] = sx[0] + TILE_SIZE;
	sy[0] = (float)-y*TILE_SIZE;
	sy[1] = sy[0] - TILE_SIZE;

	tc[0] = 1.0f;
	tc[1] = (float)(TILE_HEIGHT / TILE_SIZE);

	if (geometryTextures && map_tiles[y][x].levelType[level] > GRIDTILE_EMPTY)
		face->tc[0][2] = face->tc[1][2] = face->tc[2][2] = face->tc[3][2] = (float)(0.5f + (map_tiles[y][x].levelType[level] - 1)) / (float)geometryTextures->GetMaxDepth();
	else {
		face->tc[0][2] = face->tc[1][2] = face->tc[2][2] = face->tc[3][2] = 0;
	}

	level -= 2;

	sz[0] = (float)level * TILE_HEIGHT + map_tiles[y][x].height;
	if (x < MAX_GRID_CELLS - 1)
		sz[1] = (float)level * TILE_HEIGHT + map_tiles[y][x+1].height;
	else
		sz[1] = sz[0];
	if (y < MAX_GRID_CELLS - 1 && x < MAX_GRID_CELLS - 1)
		sz[2] = (float)level * TILE_HEIGHT + map_tiles[y+1][x+1].height;
	else
		sz[2] = sz[0];
	if (y < MAX_GRID_CELLS - 1)
		sz[3] = (float)level * TILE_HEIGHT + map_tiles[y+1][x].height;
	else
		sz[3] = sz[0];

	switch(type) {
		// top
		case GRIDFACE_TOP:
			face->vertexes[0][0] = sx[0]; face->vertexes[0][1] = sy[0]; face->vertexes[0][2] = sz[0] + TILE_HEIGHT;
			face->vertexes[1][0] = sx[1]; face->vertexes[1][1] = sy[0]; face->vertexes[1][2] = sz[1] + TILE_HEIGHT;
			face->vertexes[2][0] = sx[1]; face->vertexes[2][1] = sy[1]; face->vertexes[2][2] = sz[2] + TILE_HEIGHT;
			face->vertexes[3][0] = sx[0]; face->vertexes[3][1] = sy[1]; face->vertexes[3][2] = sz[3] + TILE_HEIGHT;
			face->normal[0] = 0; face->normal[1] = 0; face->normal[2] = 1;
			face->tc[0][0] = 0.0f; face->tc[0][1] = 0.0f;
			face->tc[1][0] = 1.0f; face->tc[1][1] = 0.0f;
			face->tc[2][0] = 1.0f; face->tc[2][1] = tc[0];
			face->tc[3][0] = 0.0f; face->tc[3][1] = tc[0];
		break;
		// ceil
		case GRIDFACE_BOTTOM:
			face->vertexes[0][0] = sx[1]; face->vertexes[0][1] = sy[0]; face->vertexes[0][2] = sz[1];
			face->vertexes[1][0] = sx[0]; face->vertexes[1][1] = sy[0]; face->vertexes[1][2] = sz[0];
			face->vertexes[2][0] = sx[0]; face->vertexes[2][1] = sy[1]; face->vertexes[2][2] = sz[3];
			face->vertexes[3][0] = sx[1]; face->vertexes[3][1] = sy[1]; face->vertexes[3][2] = sz[2];
			face->normal[0] = 0; face->normal[1] = 0; face->normal[2] = -1;
			face->tc[0][0] = 1.0f; face->tc[0][1] = tc[0];
			face->tc[1][0] = 0.0f; face->tc[1][1] = tc[0];
			face->tc[2][0] = 0.0f; face->tc[2][1] = 0.0f;
			face->tc[3][0] = 1.0f; face->tc[3][1] = 0.0f;
		break;
		// north
		case GRIDFACE_NORTH:
			face->vertexes[0][0] = sx[0]; face->vertexes[0][1] = sy[0]; face->vertexes[0][2] = sz[0] + TILE_HEIGHT;
			face->vertexes[1][0] = sx[0]; face->vertexes[1][1] = sy[0]; face->vertexes[1][2] = sz[0];
			face->vertexes[2][0] = sx[1]; face->vertexes[2][1] = sy[0]; face->vertexes[2][2] = sz[1];
			face->vertexes[3][0] = sx[1]; face->vertexes[3][1] = sy[0]; face->vertexes[3][2] = sz[1] + TILE_HEIGHT;
			face->normal[0] = 0; face->normal[1] = 1; face->normal[2] = 0;
			face->tc[0][0] = 1.0f; face->tc[0][1] = 0.0f;
			face->tc[1][0] = 1.0f; face->tc[1][1] = tc[1];
			face->tc[2][0] = 0.0f; face->tc[2][1] = tc[1];
			face->tc[3][0] = 0.0f; face->tc[3][1] = 0.0f;
		break;
		// east
		case GRIDFACE_EAST:
			face->vertexes[0][0] = sx[1]; face->vertexes[0][1] = sy[0]; face->vertexes[0][2] = sz[1] + TILE_HEIGHT;
			face->vertexes[1][0] = sx[1]; face->vertexes[1][1] = sy[0]; face->vertexes[1][2] = sz[1];
			face->vertexes[2][0] = sx[1]; face->vertexes[2][1] = sy[1]; face->vertexes[2][2] = sz[2];
			face->vertexes[3][0] = sx[1]; face->vertexes[3][1] = sy[1]; face->vertexes[3][2] = sz[2] + TILE_HEIGHT;
			face->normal[0] = 1; face->normal[1] = 0; face->normal[2] = 0;
			face->tc[0][0] = 1.0f; face->tc[0][1] = 0.0f;
			face->tc[1][0] = 1.0f; face->tc[1][1] = tc[1];
			face->tc[2][0] = 0.0f; face->tc[2][1] = tc[1];
			face->tc[3][0] = 0.0f; face->tc[3][1] = 0.0f;
		break;
		// south
		case GRIDFACE_SOUTH:
			face->vertexes[0][0] = sx[0]; face->vertexes[0][1] = sy[1]; face->vertexes[0][2] = sz[3];
			face->vertexes[1][0] = sx[0]; face->vertexes[1][1] = sy[1]; face->vertexes[1][2] = sz[3] + TILE_HEIGHT;
			face->vertexes[2][0] = sx[1]; face->vertexes[2][1] = sy[1]; face->vertexes[2][2] = sz[2] + TILE_HEIGHT;
			face->vertexes[3][0] = sx[1]; face->vertexes[3][1] = sy[1]; face->vertexes[3][2] = sz[2];
			face->normal[0] = 0; face->normal[1] = -1; face->normal[2] = 0;
			face->tc[0][0] = 0.0f; face->tc[0][1] = tc[1];
			face->tc[1][0] = 0.0f; face->tc[1][1] = 0.0f;
			face->tc[2][0] = 1.0f; face->tc[2][1] = 0.0f;
			face->tc[3][0] = 1.0f; face->tc[3][1] = tc[1];
		break;
		// west
		case GRIDFACE_WEST:
			face->vertexes[0][0] = sx[0]; face->vertexes[0][1] = sy[0]; face->vertexes[0][2] = sz[0];
			face->vertexes[1][0] = sx[0]; face->vertexes[1][1] = sy[0]; face->vertexes[1][2] = sz[0] + TILE_HEIGHT;
			face->vertexes[2][0] = sx[0]; face->vertexes[2][1] = sy[1]; face->vertexes[2][2] = sz[3] + TILE_HEIGHT;
			face->vertexes[3][0] = sx[0]; face->vertexes[3][1] = sy[1]; face->vertexes[3][2] = sz[3];
			face->normal[0] = -1; face->normal[1] = 0; face->normal[2] = 0;
			face->tc[0][0] = 0.0f; face->tc[0][1] = tc[1];
			face->tc[1][0] = 0.0f; face->tc[1][1] = 0.0f;
			face->tc[2][0] = 1.0f; face->tc[2][1] = 0.0f;
			face->tc[3][0] = 1.0f; face->tc[3][1] = tc[1];
		break;
		default:
		break;
	}
}

int Level::CheckFaceForTile(int x, int y, int type, int level) {
	int ntype = GRIDTILE_EMPTY;

	if (x < 0 || y < 0 || x >= MAX_GRID_CELLS || y >= MAX_GRID_CELLS || level < 0 || level > GRIDLEVEL_AIR)
		return 0;


	if (map_tiles[y][x].levelType[level] == GRIDTILE_EMPTY)
		return 0;

	switch(type) {
		case GRIDFACE_TOP:
			if (level == GRIDLEVEL_AIR)
				return 1;
			ntype = map_tiles[y][x].levelType[level+1];
		break;
		case GRIDFACE_BOTTOM:
			if (level == GRIDLEVEL_BASE)
				return 1;
			ntype = map_tiles[y][x].levelType[level-1];
		break;
		case GRIDFACE_NORTH:
			if (y > 0)
				ntype = map_tiles[y-1][x].levelType[level];
		break;
		case GRIDFACE_EAST:
			if (x < MAX_GRID_CELLS - 1)
				ntype = map_tiles[y][x+1].levelType[level];
		break;
		case GRIDFACE_SOUTH:
			if (y < MAX_GRID_CELLS - 1)
				ntype = map_tiles[y+1][x].levelType[level];
		break;
		case GRIDFACE_WEST:
			if (x > 0)
				ntype = map_tiles[y][x-1].levelType[level];
		break;
		default: return 0;
	}
	if (ntype == GRIDTILE_EMPTY || ( (ntype == GRIDTILE_WATER) ^ (map_tiles[y][x].levelType[level] == GRIDTILE_WATER) ) )
		return 1;
	return 0;
}

ParticleSystem *Level::GetParticleSystem() {
	return particleSystem;
}
