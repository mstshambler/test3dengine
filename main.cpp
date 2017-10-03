#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef WIN32
#include <GL/glew.h>
#endif
#include <GL/freeglut.h>

#include "gettimeofday.h"

#include "common.h"
#include "math.h"
#include "game.h"
#include "render.h"
#include "texture.h"
#include "particles.h"
#include "light.h"
#include "level.h"
#include "pathfinder.h"

int hMainWindow; // Main Window

// keyboard
bool global_keys[256];
bool global_specialkeys[256];

// mouse
int global_mouse[3];
int global_mouselastpos[2];
int global_mouserotatepos[2];
int global_mousepos[2];

// cursor position in 3D
double global_objpos[3];

Game *game;
Render *render;
Texture *texture;
Level *level;
PathFinder *pathFinder;

/*
 * Order of initialization: Game, Render, Texture, everything else
*/

// KeyDown Event
static void key(unsigned char k, int x, int y) {
	global_keys[k] = true;
}

// KeyUp Event
static void keyup(unsigned char k, int x, int y) {
	global_keys[k] = false;
}

// Special KeyDown Event
static void specialkey(int key, int x, int y) {
	global_specialkeys[key] = true;
}

// Special KeyUp Event
static void specialkeyup(int key, int x, int y) {
	global_specialkeys[key] = false;
}

// Mouse Button Event
static void mousebutton(int button, int state, int x, int y) {
	// check for button
	switch(button) {
		// left mouse button
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				global_mouse[0] = 1;
			} else if (state == GLUT_UP) {
				global_mouse[0] = 2;
			}
		break;
		// right mouse button
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN) {
				global_mouse[1] = 1;

				global_mouselastpos[0] = x;
				global_mouselastpos[1] = y;
				glutSetCursor(GLUT_CURSOR_NONE);
				glutWarpPointer(render->GetScreenWidth() / 2, render->GetScreenHeight() / 2);
			} else if (state == GLUT_UP)
				global_mouse[1] = 2;
		break;
		// no middle/4/5/so on buttons
		default:
		break;
	}

	if (global_mouse[1] == 0) {
		// storing mouse position
		global_mousepos[0] = x;
		global_mousepos[1] = y;
	}

}

// Mouse Move Event
static void mousemotion(int x, int y) {
	if (global_mouse[1] == 1) {
		if (x != render->GetScreenWidth() / 2 || y != render->GetScreenHeight() / 2) {
			global_mouserotatepos[0] += x - render->GetScreenWidth() / 2;
			global_mouserotatepos[1] += y - render->GetScreenHeight() / 2;

			glutWarpPointer(render->GetScreenWidth() / 2, render->GetScreenHeight() / 2);
		}
	} else {
		global_mousepos[0] = x;
		global_mousepos[1] = y;
	}
}

// Idle function
static void idle(void) {
	double speed_move = 20.0;
	double speed_rotate = 60.0;
	double overall_msecs = 0.0;

	double cur_move = 0.0;
	double cur_rotate = 0.0;

	double mouse_speed = 0.17;

	vector move;
	vector newpos;

	vector forward, up, right;

	// current tick
	gettimeofday(&tick_ntime, NULL);

	getdifftime(tick_ntime, tick_otime, &tick_diff);

	// last tick
	tick_otime = tick_ntime;

	Math::AngleVectors(render->camera.viewangles, forward, right, up);

// make normalized vectors after nullifying Z values
	if (game->isFirstPerson) {
		forward[2] = 0;
		right[2] = 0;
	}
	Math::NormalizeVector(forward);
	Math::NormalizeVector(right);
	Math::NormalizeVector(up);

	// If "Esc" - quit
	if (global_keys[27]) {
		exit(0);
	}

	overall_msecs = tick_diff.tv_sec * 1000 + tick_diff.tv_usec / 1000;
	cur_move = speed_move * overall_msecs / 1000;
	cur_rotate = speed_rotate * overall_msecs / 1000;

	move[0] = move[1] = move[2] = 0;

	if (global_keys['q']) {
		render->camera.viewangles[1]-= (float)cur_rotate;
		while (render->camera.viewangles[1] < 0.0)
			render->camera.viewangles[1] += 360.0;
	}
	if (global_keys['e']) {
		render->camera.viewangles[1]+=(float)cur_rotate;
		while (render->camera.viewangles[1] >= 360.0)
			render->camera.viewangles[1] -= 360.0;
	}

	if (global_keys['w']) {
		move[0] += forward[0] * (float)cur_move;
		move[1] += forward[1] * (float)cur_move;
		if (!game->isFirstPerson)
			move[2] += forward[2] * (float)cur_move;
	}
	if (global_keys['s']) {
		move[0] -= forward[0] * (float)cur_move;
		move[1] -= forward[1] * (float)cur_move;
		if (!game->isFirstPerson)
			move[2] -= forward[2] * (float)cur_move;
	}

	if (global_keys['a']) {
		move[0] -= right[0] * (float)cur_move;
		move[1] -= right[1] * (float)cur_move;
		if (!game->isFirstPerson)
			move[2] -= right[2] * (float)cur_move;
	}
	if (global_keys['d']) {
		move[0] += right[0] * (float)cur_move;
		move[1] += right[1] * (float)cur_move;
		if (!game->isFirstPerson)
			move[2] += right[2] * (float)cur_move;
	}

	if (global_keys['r']) {
		/*
		move[0] -= up[0] * (float)cur_move;
		move[1] -= up[1] * (float)cur_move;
		move[2] -= up[2] * (float)cur_move;
		*/
		move[2] += (float)cur_move;
	}
	if (global_keys['f']) {
		/*
		move[0] += up[0] * (float)cur_move;
		move[1] += up[1] * (float)cur_move;
		move[2] += up[2] * (float)cur_move;
		*/
		move[2] -= (float)cur_move;
	}

// some basic collision
// TODO: make it much better
	level->CheckCollision(render->camera.pos, move, newpos, ENTITY_RADIUS, tick_diff);
	Math::VectorCopy(newpos, render->camera.pos);

	// left mouse button down
	if (global_mouse[0]==1) {
	}
	// left mouse button up
	if (global_mouse[0]==2) {
		global_mouse[0]=0;
	}

	// right mouse button down
	if (global_mouse[1]==1) {
		render->camera.viewangles[2] += global_mouserotatepos[0] * (float)mouse_speed;
		render->camera.viewangles[0] -= global_mouserotatepos[1] * (float)mouse_speed;

		while (render->camera.viewangles[1] < 0.0)
			render->camera.viewangles[1] += 360.0;
		while (render->camera.viewangles[1] >= 360.0)
			render->camera.viewangles[1] -= 360.0;

		global_mouserotatepos[0] = 0;
		global_mouserotatepos[1] = 0;
	}
	// right mouse button up
	if (global_mouse[1]==2) {
		global_mouse[1]=0;
		
		global_mouserotatepos[0] = 0;
		global_mouserotatepos[1] = 0;

		glutWarpPointer(global_mouselastpos[0], global_mouselastpos[1]);
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	}

	level->GetParticleSystem()->DoTick(tick_diff);

	// render
	glutPostRedisplay();
}

// Rendering
void display(void) {
	vector objpos;

	render->Frame(global_mousepos[0], global_mousepos[1], objpos);
}

void reshape(int w, int h) {
	render->Reshape(w, h);
}

// Entry point
int main(int argc, char** argv) {
	int i;

	// clear
	for (i=0;i<256;i++) {
		global_keys[i] = false;
		global_specialkeys[i] = false;
	}
	for (i=0;i<3;i++)
		global_mouse[i] = false;

	game = new Game();
	game->isFirstPerson = 1;

	render = new Render();

	// Screen Init
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_STENCIL);
	glutInitWindowSize (800, 600);
	glutInitWindowPosition (10, 10);

	glutCreateWindow ("Dungeon Engine Alpha Render 0.1b");

#ifdef WIN32
	{
		GLenum err = glewInit();
		if (GLEW_OK != err) {
			debug("Error: %s\n", glewGetErrorString(err));
			exit(0);
		}
	}
#endif

	render->Init(800, 600);

	texture = new Texture();
	texture->LoadTexturesList();

	level = new Level();
	pathFinder = new PathFinder(level);

	// Functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutKeyboardUpFunc(keyup);
	glutMouseFunc(mousebutton);
	glutMotionFunc(mousemotion);
	glutPassiveMotionFunc(mousemotion);
	glutSpecialFunc(specialkey);
	glutSpecialUpFunc(specialkeyup);
	glutIdleFunc(idle);

	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

	gettimeofday(&tick_otime, NULL);

	if (game->isFirstPerson) {
		render->camera.pos[0] = 1 * TILE_SIZE + TILE_SIZE/2;
		render->camera.pos[1] = -(1 * TILE_SIZE + TILE_SIZE/2);
		render->camera.pos[2] = ENTITY_HEIGHT;

		render->camera.viewangles[2] = 90;
	} else {
		render->camera.pos[0] = 1 * TILE_SIZE + TILE_SIZE/2;
		render->camera.pos[1] = -(1 * TILE_SIZE + TILE_SIZE/2);
		render->camera.pos[2] = ENTITY_HEIGHT * 10;

		render->camera.viewangles[0] = -90;
	}

	// Let's Rock'n'Roll
	glutMainLoop();
	return 0;
}
