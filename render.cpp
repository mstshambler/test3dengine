#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef WIN32
#include <GL/glew.h>
#endif

#include <GL/glut.h>

#include "common.h"

#ifdef WIN32
#include "gettimeofday.h"
#endif

#include "math.h"
#include "render.h"
#include "texture.h"
#include "particles.h"
#include "light.h"
#include "level.h"

void printLog(GLuint obj) {
	int infologLength = 0;
	int maxLength;
	
	if(glIsShader(obj))
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	else
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);

	char *infoLog = (char *) malloc(maxLength*sizeof(char));
 
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
 
	if (infologLength > 0)
		debug(infoLog);
}

char *file2string(const char *path) {
	FILE *fd;
	long len, r;
	char *str;
 
	if (!(fd = fopen(path, "r")))
		return NULL;
 
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0, SEEK_SET);
 
	str = (char *) malloc(len * sizeof(char));
	r = fread(str, sizeof(char), len, fd);
	str[r - 1] = '\0';
	fclose(fd);
 
	return str;
}

// based on NeHe
bool IsExtensionSupported( char* szTargetExtension ) {
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return false;

	pszExtensions = glGetString( GL_EXTENSIONS );

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;) {
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return true;
		pszStart = pszTerminator;
	}
	return false;
}

Render::Render() {
	gl_vendor = NULL;
	gl_renderer = NULL;
	gl_version = NULL;
	gl_extensions = NULL;

	screen_width = 0;
	screen_height = 0;

	camera.pos[0] = camera.pos[1] = camera.pos[2] = 0;
	camera.viewangles[0] = camera.viewangles[1] = camera.viewangles[2] = 0;

	extension_twoSidedStencil = 0;
	extension_3dTextures = 0;
}

void Render::CalculateFrustum() {    
	float proj[16];
	float modl[16];
	float clip[16];

	// collect matrixes
	glGetFloatv( GL_PROJECTION_MATRIX, proj );
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	// combine both matrixes
	clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];
	clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];
	clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];
	clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];
	
	// calculating sides of frustum
	// RIGHT side
	m_Frustum[Math::FRUSTUM_RIGHT][Math::PLANE_A] = clip[ 3] - clip[ 0];
	m_Frustum[Math::FRUSTUM_RIGHT][Math::PLANE_B] = clip[ 7] - clip[ 4];
	m_Frustum[Math::FRUSTUM_RIGHT][Math::PLANE_C] = clip[11] - clip[ 8];
	m_Frustum[Math::FRUSTUM_RIGHT][Math::PLANE_D] = clip[15] - clip[12];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_RIGHT);

	// LEFT side of the frustum
	m_Frustum[Math::FRUSTUM_LEFT][Math::PLANE_A] = clip[ 3] + clip[ 0];
	m_Frustum[Math::FRUSTUM_LEFT][Math::PLANE_B] = clip[ 7] + clip[ 4];
	m_Frustum[Math::FRUSTUM_LEFT][Math::PLANE_C] = clip[11] + clip[ 8];
	m_Frustum[Math::FRUSTUM_LEFT][Math::PLANE_D] = clip[15] + clip[12];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_LEFT);

	// BOTTOM side of the frustum
	m_Frustum[Math::FRUSTUM_BOTTOM][Math::PLANE_A] = clip[ 3] + clip[ 1];
	m_Frustum[Math::FRUSTUM_BOTTOM][Math::PLANE_B] = clip[ 7] + clip[ 5];
	m_Frustum[Math::FRUSTUM_BOTTOM][Math::PLANE_C] = clip[11] + clip[ 9];
	m_Frustum[Math::FRUSTUM_BOTTOM][Math::PLANE_D] = clip[15] + clip[13];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_BOTTOM);

	// TOP side of the frustum
	m_Frustum[Math::FRUSTUM_TOP][Math::PLANE_A] = clip[ 3] - clip[ 1];
	m_Frustum[Math::FRUSTUM_TOP][Math::PLANE_B] = clip[ 7] - clip[ 5];
	m_Frustum[Math::FRUSTUM_TOP][Math::PLANE_C] = clip[11] - clip[ 9];
	m_Frustum[Math::FRUSTUM_TOP][Math::PLANE_D] = clip[15] - clip[13];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_TOP);

	// BACK side of the frustum
	m_Frustum[Math::FRUSTUM_BACK][Math::PLANE_A] = clip[ 3] - clip[ 2];
	m_Frustum[Math::FRUSTUM_BACK][Math::PLANE_B] = clip[ 7] - clip[ 6];
	m_Frustum[Math::FRUSTUM_BACK][Math::PLANE_C] = clip[11] - clip[10];
	m_Frustum[Math::FRUSTUM_BACK][Math::PLANE_D] = clip[15] - clip[14];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_BACK);

	// FRONT side of the frustum
	m_Frustum[Math::FRUSTUM_FRONT][Math::PLANE_A] = clip[ 3] + clip[ 2];
	m_Frustum[Math::FRUSTUM_FRONT][Math::PLANE_B] = clip[ 7] + clip[ 6];
	m_Frustum[Math::FRUSTUM_FRONT][Math::PLANE_C] = clip[11] + clip[10];
	m_Frustum[Math::FRUSTUM_FRONT][Math::PLANE_D] = clip[15] + clip[14];

	Math::NormalizePlane(m_Frustum, Math::FRUSTUM_FRONT);
}

bool Render::PointInFrustum(float x, float y, float z) {
	// Go through all the sides of the frustum
	for(int i = 0; i < 6; i++) {
		// Calculate the plane equation and check if the point is behind a side of the frustum
		if(m_Frustum[i][Math::PLANE_A] * x + m_Frustum[i][Math::PLANE_B] * y + m_Frustum[i][Math::PLANE_C] * z + m_Frustum[i][Math::PLANE_D] <= 0) {
			// The point was behind a side, so it ISN'T in the frustum
			return false;
		}
	}

	// The point was inside of the frustum (In front of ALL the sides of the frustum)
	return true;
}

int Render::GetScreenWidth() {
	return screen_width;
}

int Render::GetScreenHeight() {
	return screen_height;
}

void Render::Reshape(int width, int height) {
	vector empty;

	screen_width = width;
	screen_height = height;
	Frame(-1, -1, empty);
}

void Render::Init(int width, int height) {
	int		x, x2, y2, y;
	double screenaspect;

	// define screen resolution
	screen_width = width;
	screen_height = height;

	x = 0;
	x2 = width;
	y = height;
	y2 = 0;

	// get OpenGL parameters
	gl_vendor = strdup((char *)glGetString (GL_VENDOR));
	gl_renderer = strdup((char *)glGetString (GL_RENDERER));
	gl_version = strdup((char *)glGetString (GL_VERSION));
	gl_extensions = strdup((char *)glGetString (GL_EXTENSIONS));

	debug("%s", gl_extensions);
	if (IsExtensionSupported("GL_EXT_stencil_two_side"))
		extension_twoSidedStencil = 1;
	if (IsExtensionSupported("GL_EXT_texture3D"))
		extension_3dTextures = 1;

	// Set default OpenGL settings
	glClearColor( 0.0F, 0.0F, 0.0F, 0.0F );
	glClearStencil(0);
	glShadeModel( GL_SMOOTH );

	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );

	glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glRenderMode(GL_RENDER);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    screenaspect = (double)width/height;
	gluPerspective (90.0f/screenaspect,  screenaspect,  0,  4096);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glViewport (x, y2, width, height);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glDisable(GL_LIGHT0);

	// Shader loading
	{
		unsigned long vert_id, frag_id;
		char *vsrc = file2string("data/shaders/Lighting.vert");
		char *fsrc = file2string("data/shaders/Lighting.frag");

		vert_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_id, 1, (const GLchar **)&vsrc, NULL);
		glCompileShader(vert_id);
		printLog(vert_id);

		frag_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_id, 1, (const GLchar **)&fsrc, NULL);
		glCompileShader(frag_id);
		printLog(frag_id);

		free(vsrc); // because of malloc. TODO: rewrite to new/free
		free(fsrc);

		shaderLighting = glCreateProgram();
		glAttachShader(shaderLighting, vert_id);
		glAttachShader(shaderLighting, frag_id);
		glLinkProgram(shaderLighting);
		printLog(shaderLighting);


		vsrc = file2string("data/shaders/Water.vert");
		fsrc = file2string("data/shaders/Water.frag");

		vert_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vert_id, 1, (const GLchar **)&vsrc, NULL);
		glCompileShader(vert_id);
		printLog(vert_id);

		frag_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_id, 1, (const GLchar **)&fsrc, NULL);
		glCompileShader(frag_id);
		printLog(frag_id);

		free(vsrc); // because of malloc. TODO: rewrite to new/free
		free(fsrc);

		shaderWater = glCreateProgram();
		glAttachShader(shaderWater, vert_id);
		glAttachShader(shaderWater, frag_id);
		glLinkProgram(shaderWater);
		printLog(shaderWater);
	}
}

void Render::Begin(int width = -1, int height = -1) {
	double screenaspect;

	if (width == -1)
		width = screen_width;
	if (height == -1)
		height = screen_height;

	Render::Clear();

	glRenderMode(GL_RENDER);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glViewport(0, 0, width, height);
    screenaspect = (double)width/height;
	gluPerspective (90.0f/screenaspect,  screenaspect,  1,  4096);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();

	// Make Z look up (first person view)
	glRotatef(-90, 1, 0, 0);
//	glRotatef(90, 0, 0, 1);

	glRotatef (-camera.viewangles[0],  1, 0, 0);
	glRotatef (-camera.viewangles[1],  0, 1, 0);
	glRotatef (camera.viewangles[2],  0, 0, 1);

	glTranslatef (-camera.pos[0],  -camera.pos[1],  -camera.pos[2]);
/*
	{
		vector up;

		up[0] = 0.5; up[1] = 0.5; up[2] = 0;
		Math::NormalizeVector(up);
		gluLookAt(camera.pos[0], camera.pos[1], camera.pos[2], 24.0, -12.0, 0, up[0], up[1], up[2]);
	}
*/
	glColor4f(1.0, 1.0, 1.0, 1.0);
}


void Render::Set3DDepthMode() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_TEXTURE_3D);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void Render::Set3DPostLightingMode() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_3D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Render::Set3DTextureMode() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glEnable(GL_TEXTURE_3D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Render::Set3DParticlesMode() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	glEnable(GL_TEXTURE_3D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Render::End() {
	glRenderMode(GL_RENDER);
	glDisable(GL_DEPTH_TEST);
	glDisable (GL_ALPHA_TEST);

	glutSwapBuffers();
}

void Render::Set2DMode(int width = 800, int height = 600) {
	if (width == -1)
		width = screen_width;
	if (height == -1)
		height = screen_height;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity ();

	glDisable (GL_DEPTH_TEST);
	glDisable (GL_CULL_FACE);
	glEnable (GL_ALPHA_TEST);

	glColor4f (1,1,1,1);
}

void Render::End2DMode() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Render::Frame(int mousex, int mousey, vector worldpos) {
	static float someTime = 0;

	Begin();
	
	Set3DDepthMode();
	CalculateFrustum();
	level->Render(0);

	level->RenderLighting(); // disables DepthMask

	Set3DPostLightingMode();
/*
	{
		int timeUniform;
		int tex, tex2;
		int waterUniform;
		Texture3DInfo *geometryTexture;
		Texture3DSubInfo *waterTex = NULL;

		geometryTexture = (Texture3DInfo *)texture->FindTexture("GeometryTextures");
		if (geometryTexture) {
			waterTex = texture->FindTexture3DSub(geometryTexture, "water.tga");
		}

		glUseProgram(shaderWater);

		// TODO: move it into init
		tex = glGetUniformLocation(shaderWater, (char *)"tex");
		if (tex >= 0) {
			glUniform1i(tex, 0);
		}
		tex2 = glGetUniformLocation(shaderWater, (char *)"tex2");
		if (tex2 >= 0) {
			glUniform1i(tex2, 1);
		}
		waterUniform = glGetUniformLocation(shaderWater, (char *)"waterTex");
		if (waterUniform >= 0 && waterTex) {
			glUniform1f(waterUniform, (0.5f + waterTex->depth) / (float)geometryTexture->GetMaxDepth());
		}

		someTime += tick_diff.tv_sec + tick_diff.tv_usec / 1000000.0f;
		while (someTime >= 2.0f * M_PI) {
			someTime -= 2.0f * M_PI;
		}
		timeUniform = glGetUniformLocation(shaderWater, (char *)"time");

		if (timeUniform >= 0)
			glUniform1f(timeUniform, someTime);

		level->Render(2);

		glUseProgram(0);
	}
*/
	level->Render(2);

//	Set3DParticlesMode();
//	level->RenderParticles();
	
	Set3DTextureMode();
	// Render something maybe

	if (mousex >=0 && mousey >= 0)
		ReadCoordsUnderCursor(mousex, mousey, worldpos);

	Set2DMode();

	// TODO: 2D Rendering here (GUI & etc)

	End2DMode();

	End();
}

void Render::Shutdown(void) {
}

void Render::Clear (void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

int Render::GetShaderLighting() {
	return shaderLighting;
}

int Render::GetShaderWater() {
	return shaderWater;
}

int Render::GetExtensionTwoSidedStencil() {
	return extension_twoSidedStencil;
}

int Render::GetExtension3DTextures() {
	return extension_3dTextures;
}

void Render::ReadCoordsUnderCursor(int x, int y, vector worldpos) {
	GLfloat wy, wz;
	GLdouble objpos[3];
	GLdouble model_view[16];
	GLdouble projection[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glReadBuffer(GL_FRONT);
	wy = (float)viewport[3]-(float)y;
	glReadPixels(x, (int)wy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &wz);
	gluUnProject(x, wy, wz,
		model_view, projection, viewport,
		&objpos[0], &objpos[1], &objpos[2]);

	objpos[1]*=-1;
	glReadBuffer(GL_BACK);

	worldpos[0] = (float)objpos[0];
	worldpos[1] = (float)objpos[1];
	worldpos[2] = (float)objpos[2];
}
