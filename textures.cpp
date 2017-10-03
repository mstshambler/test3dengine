#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef WIN32
#include <GL/glew.h>
#endif

#include <GL/glut.h>

#include <string.h>
#include <time.h>

#include "common.h"
#include "texture.h"

#define TEXTURE_ATLAS_BLOCK 32

TextureAtlasSubInfo::TextureAtlasSubInfo() {
	name[0] = 0;
	x0 = y0 = x1 = y1 = 0.0;
}

TextureAtlasSubInfo::~TextureAtlasSubInfo() {
}




TextureInfo::TextureInfo() {
	name[0] = 0;
	texnum = 0;
}

TextureInfo::~TextureInfo() {
}




TextureAtlasInfo::TextureAtlasInfo() {
	textureAtlasSubInfoList = new TextureAtlasSubInfoList();
	grid = NULL;
}

TextureAtlasInfo::~TextureAtlasInfo() {
	delete grid;

	textureAtlasSubInfoList->clear(1);
	delete textureAtlasSubInfoList;
}

int TextureAtlasInfo::GenerateTextureAtlas(int w, int h, int t) {
	int size;

	glBindTexture(GL_TEXTURE_2D, t);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	texnum = t;
	width = w;
	height = h;

	size = w / TEXTURE_ATLAS_BLOCK * h / TEXTURE_ATLAS_BLOCK;
	grid = new int[size];
	memset(grid, 0, sizeof(int) * size);
	return t + 1;
}

int TextureAtlasInfo::FindPlaceForSub(int w, int h, int *x, int *y) {
	int px, py;
	int found = 0;
	int sw, sh;
	int lw, lh;

	sw = width / TEXTURE_ATLAS_BLOCK;
	sh = height / TEXTURE_ATLAS_BLOCK;

	lw = w / TEXTURE_ATLAS_BLOCK;
	lh = h / TEXTURE_ATLAS_BLOCK;

	if (w % TEXTURE_ATLAS_BLOCK != 0)
		lw++;
	if (h % TEXTURE_ATLAS_BLOCK != 0)
		lh++;

	for (py = 0; py <= sh - lh && found == 0; py++) {
		for (px = 0; px <= sw - lw && found == 0; px++) {
			if (*(grid + py * sw + px) == 0) {
				int tx, ty;

				found = 1;
				for (ty = py; ty < py + lh && found == 1; ty++) {
					for (tx = px; tx < px + lw && found == 1; tx++) {
						if (*(grid + ty * sw + tx) != 0)
							found = 0;
					}
				}

				if (found) {
					if (x)
						*x = px * TEXTURE_ATLAS_BLOCK;
					if (y)
						*y = py * TEXTURE_ATLAS_BLOCK;

					for (ty = py; ty < py + lh; ty++) {
						for (tx = px; tx < px + lw; tx++) {
							*(grid + ty * sw + tx) = 1;
						}
					}
				}
			}
		}
	}

	return found;
}




Texture3DSubInfo::Texture3DSubInfo() {
	name[0] = 0;
	depth = 0;
}

Texture3DSubInfo::~Texture3DSubInfo() {
}




Texture3DInfo::Texture3DInfo() {
	depth_max = 0;
	depth_current = 0;

	texture3DSubInfoList = new Texture3DSubInfoList();
}

Texture3DInfo::~Texture3DInfo() {
	texture3DSubInfoList->clear(1);
	delete texture3DSubInfoList;
}

int Texture3DInfo::GetMaxDepth() {
	return depth_max;
}

int Texture3DInfo::GetCurrentDepth() {
	return depth_current;
}

void Texture3DInfo::SetCurrentDepth(int i) {
	depth_current = i;
}

int Texture3DInfo::GenerateTexture3D(int w, int h, int depth, int t) {
	glBindTexture(GL_TEXTURE_3D, t);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage3D(GL_TEXTURE_3D, 0, 4, w, h, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	texnum = t;
	width = w;
	height = h;
	depth_max = depth;

	return t + 1;
}




Texture::Texture() {
	image_width=0;
	image_height=0;
	image_pixelsize=0;
	texnum_current[0] = texnum_current[1] = texnum_current[2] = texnum_current[3] = 0;

	texnum_max = 1;

	textureInfoList = new TextureInfoList();
}

Texture::~Texture() {
	textureInfoList->clear(1);
	delete textureInfoList;
}

int Texture::fgetLittleShort (FILE *f) {
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

int Texture::fgetLittleLong (FILE *f) {
	byte	b1, b2, b3, b4;

	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	b4 = fgetc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}

byte* Texture::LoadTGA (FILE *fin) {
	int				columns, rows;
	long numPixels;
	byte			*image_rgba;

	targa_header.id_length = fgetc(fin);
	targa_header.colormap_type = fgetc(fin);
	targa_header.image_type = fgetc(fin);

	targa_header.colormap_index = fgetLittleShort(fin);
	targa_header.colormap_length = fgetLittleShort(fin);
	targa_header.colormap_size = fgetc(fin);
	targa_header.x_origin = fgetLittleShort(fin);
	targa_header.y_origin = fgetLittleShort(fin);
	targa_header.width = fgetLittleShort(fin);
	targa_header.height = fgetLittleShort(fin);
	targa_header.pixel_size = fgetc(fin);
	targa_header.attributes = fgetc(fin);

	if (targa_header.image_type!=2
		&& targa_header.image_type!=10)
		return NULL;

	if (targa_header.colormap_type !=0
		|| (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		return NULL;

	// Uncompressed RGB/RGBA only
	if (targa_header.image_type != 2)
		return NULL;

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (targa_header.pixel_size == 24)
		image_rgba = (unsigned char *)malloc (numPixels*3);
	else 
		image_rgba = (unsigned char *)malloc (numPixels*4);

	if (targa_header.id_length != 0)
		fseek(fin, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment

	fread(image_rgba, 1, rows*columns*(targa_header.pixel_size == 24 ? 3 : 4), fin);

	image_width = columns;
	image_height = rows;
	image_pixelsize = targa_header.pixel_size;

	return image_rgba;
}


void Texture::Bind(int texnum, int unit) {
	if (texnum_current[unit] == texnum)
		return;

	texnum_current[unit] = texnum;
	glBindTexture(GL_TEXTURE_2D, texnum);
}

void Texture::Bind3D(int texnum, int unit) {
	if (texnum_current[unit] == texnum)
		return;

	texnum_current[unit] = texnum;
	glBindTexture(GL_TEXTURE_3D, texnum);
}

TextureInfo *Texture::FindTexture(char *name) {
	dynamicReaderFree(tr, TextureInfo);

	tr->attach(textureInfoList);

	for (TextureInfo *ti = tr->getFirstElement(); ti; ti = tr->getNextElement())
		if (!strcasecmp(ti->name, name))
			return ti;

	return NULL;
}

TextureInfo *Texture::FindTexture(int t) {
	dynamicReaderFree(tr, TextureInfo);

	tr->attach(textureInfoList);

	for (TextureInfo *ti = tr->getFirstElement(); ti; ti = tr->getNextElement())
		if (ti->texnum == t)
			return ti;

	return NULL;
}

TextureAtlasSubInfo *Texture::FindTextureAtlasSub(TextureAtlasInfo *ti, char *name) {
	dynamicReaderFree(tr, TextureAtlasSubInfo);

	if (ti == NULL)
		return NULL;

	tr->attach(ti->textureAtlasSubInfoList);

	for (TextureAtlasSubInfo *tai = tr->getFirstElement(); tai; tai = tr->getNextElement())
		if (!strcasecmp(tai->name, name))
			return tai;

	return 0;
}

Texture3DSubInfo *Texture::FindTexture3DSub(Texture3DInfo *ti, char *name) {
	dynamicReaderFree(tr, Texture3DSubInfo);

	if (ti == NULL)
		return NULL;

	tr->attach(ti->texture3DSubInfoList);

	for (Texture3DSubInfo *tai = tr->getFirstElement(); tai; tai = tr->getNextElement())
		if (!strcasecmp(tai->name, name))
			return tai;

	return 0;
}

int Texture::LoadTexture(char *filename, char *name) {
	FILE *fp;
	byte *bp;
	TextureInfo *texnum;

	if ((texnum = FindTexture(name)) != NULL)
		return texnum->texnum;

	fp = fopen(filename,"rb");
	if (fp) {
		TextureInfo *ti;

		bp = LoadTGA(fp);

		Bind(texnum_max, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		texnum_max++;
		ti = new TextureInfo();

		strcpy(ti->name, name);
		ti->texnum = texnum_max-1;
		textureInfoList->addElement(ti);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (image_pixelsize == 24)
			glTexImage2D(GL_TEXTURE_2D, 0, 3, this->image_width, this->image_height, 0, GL_BGR, GL_UNSIGNED_BYTE, bp);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, 4, this->image_width, this->image_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bp);

		fclose(fp);
		return texnum_max-1;
	}
	return 0;
}

void Texture::LoadTextureIntoAtlas(TextureAtlasInfo *ti, char *filename, char *name) {
	FILE *fp;
	byte *bp;

	if (!ti)
		return;

	fp = fopen(filename,"rb");
	if (fp) {
		TextureAtlasSubInfo *tai;
		int col, row;

		bp = LoadTGA(fp);
		if (ti->FindPlaceForSub(this->image_width, this->image_height, &col, &row)) {
			tai = new TextureAtlasSubInfo();

			texture->Bind(ti->texnum, 0);

			strcpy(tai->name, name);
			tai->x0 = (float)(col) / (float)ti->width;
			tai->y0 = (float)(row) / (float)ti->width;
			tai->x1 = (float)(col + this->image_width) / (float)ti->width;
			tai->y1 = (float)(row + this->image_height) / (float)ti->width;

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			
			if (image_pixelsize == 24)
				glTexSubImage2D(GL_TEXTURE_2D, 0, col, row, this->image_width, this->image_height, GL_BGR, GL_UNSIGNED_BYTE, bp);
			else
				glTexSubImage2D(GL_TEXTURE_2D, 0, col, row, this->image_width, this->image_height, GL_BGRA, GL_UNSIGNED_BYTE, bp);

			ti->textureAtlasSubInfoList->addElement(tai);
		}
		fclose(fp);
	}
}

void Texture::LoadTexture3D(Texture3DInfo *ti, char *filename, char *name) {
	FILE *fp;
	byte *bp;

	if (!ti)
		return;

	if (ti->GetMaxDepth() <= ti->GetCurrentDepth())
		return;

	fp = fopen(filename,"rb");
	if (fp) {
		Texture3DSubInfo *t3i;

		bp = LoadTGA(fp);
		int depth;

		depth = ti->GetCurrentDepth();
		ti->SetCurrentDepth(depth + 1);

		texture->Bind3D(ti->texnum, 0);

		t3i = new Texture3DSubInfo();

		strcpy(t3i->name, name);
		t3i->depth = depth;

		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			
		if (image_pixelsize == 24)
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, depth, this->image_width, this->image_height, 1, GL_BGR, GL_UNSIGNED_BYTE, bp);
		else
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, depth, this->image_width, this->image_height, 1, GL_BGRA, GL_UNSIGNED_BYTE, bp);

		ti->texture3DSubInfoList->addElement(t3i);

		fclose(fp);
	}
}

void Texture::LoadTexturesList(void) {
//	TextureInfo *ti;
/*
	TextureAtlasInfo *tai;

	tai = new TextureAtlasInfo();
	strcpy(tai->name, "GeometryTextures");
	texnum_max = tai->GenerateTextureAtlas(2048, 2048, texnum_max);
	textureInfoList->addElement(tai);

	LoadTextureIntoAtlas(tai, (char *)"data/textures/floor.tga", (char *)"floor.tga");
	LoadTextureIntoAtlas(tai, (char *)"data/textures/wall.tga", (char *)"wall.tga");
	LoadTextureIntoAtlas(tai, (char *)"data/textures/ceil.tga", (char *)"ceil.tga");

	tai = new TextureAtlasInfo();
	strcpy(tai->name, "ParticleTextures");
	texnum_max = tai->GenerateTextureAtlas(256, 256, texnum_max);
	textureInfoList->addElement(tai);

	LoadTextureIntoAtlas(tai, (char *)"data/textures/particle.tga", (char*)"particle.tga");
	LoadTextureIntoAtlas(tai, (char *)"data/textures/particle2.tga", (char*)"particle2.tga");
*/
	Texture3DInfo *t3i;
	t3i = new Texture3DInfo();
	strcpy(t3i->name, "GeometryTextures");
	texnum_max = t3i->GenerateTexture3D(32, 32, 64, texnum_max);
	textureInfoList->addElement(t3i);

	LoadTexture3D(t3i, (char *)"data/textures/geometry/water.tga", (char *)"water.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/grass.tga", (char *)"grass.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/dirt.tga", (char *)"dirt.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/stone.tga", (char *)"stone.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/wood.tga", (char *)"wood.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/sand.tga", (char *)"sand.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/gravel.tga", (char *)"gravel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/snow.tga", (char *)"snow.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/blackstone.tga", (char *)"blackstone.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/cobblestone.tga", (char *)"cobblestone.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/stonepanel.tga", (char *)"stonepanel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/woodpanel.tga", (char *)"woodpanel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/sandwall.tga", (char *)"sandwall.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/sandpanel.tga", (char *)"sandpanel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/bricks.tga", (char *)"bricks.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/blackstonepanel.tga", (char *)"blackstonepanel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/gold.tga", (char *)"gold.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/iron.tga", (char *)"iron.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/coal.tga", (char *)"coal.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/redstone.tga", (char *)"redstone.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/diamonds.tga", (char *)"diamonds.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/bluestone.tga", (char *)"bluestone.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/goldpanel.tga", (char *)"goldpanel.tga");
	LoadTexture3D(t3i, (char *)"data/textures/geometry/bluestonepanel.tga", (char *)"bluestonepanel.tga");
	
	t3i = new Texture3DInfo();
	strcpy(t3i->name, "ParticleTextures");
	texnum_max = t3i->GenerateTexture3D(32, 32, 64, texnum_max);
	textureInfoList->addElement(t3i);

	LoadTexture3D(t3i, (char *)"data/textures/particle.tga", (char*)"particle.tga");
	LoadTexture3D(t3i, (char *)"data/textures/particle2.tga", (char*)"particle2.tga");

	LoadTexture((char *)"data/textures/bumpmap.tga", (char *)"bumpmap.tga");
}
