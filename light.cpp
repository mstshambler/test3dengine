#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "common.h"
#include "math.h"
#include "light.h"

LightSource::LightSource() {
	pos[0] = pos[1] = pos[2] = 0;
	radius = 0;
	color[0] = color[1] = color[2] = 0;
	changed = -1;
}

LightSource::~LightSource() {
}
