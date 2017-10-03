#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#include "gettimeofday.h"
#include "common.h"
#include "math.h"
#include "game.h"

Game::Game() {
	isFirstPerson = 0;
}

Game::~Game() {
}
