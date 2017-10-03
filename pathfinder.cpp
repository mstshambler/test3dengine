#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "common.h"
#include "gettimeofday.h"
#include "math.h"
#include "particles.h"
#include "light.h"
#include "render.h"
#include "texture.h"
#include "level.h"
#include "pathfinder.h"

PointArray::PointArray() {
	count = 0;
	data = NULL;
	size = 0;
}

PointArray::~PointArray() {
	delete data;
}

void PointArray::Init(int i) {
	if (i <= 0)
		return;

	size = i;
	if (data)
		delete data;

	data = new Point[i];
}

void PointArray::AddData(int x, int y) {
	if (count < size) {
		Point *p = data+count;

		p->x = x;
		p->y = y;

		count++;
	}
}

void PointArray::RemoveData(int x, int y) {
	int i;
	Point *p;

	for (i=0, p=data; i<count && (p->x != x || p->y != y) ; i++, p++);
	if (i<count) {
		memmove(data+i, data+(i+1), (count-i-1)*sizeof(Point));
		count--;
	}
}

Point *PointArray::GetFirstElement() {
	if (count > 0) {
		return data;
	}
	return NULL;
}

void PointArray::RemoveFirstElement() {
	if (count > 0) {
		memmove(data, data+1, (count-1)*sizeof(Point));
		count--;
	}
}

int PointArray::GetCount() {
	return count;
}

void PointArray::SetCount(int i) {
	count = i;
}

void PointArray::RemoveDataByNum(int i) {
	if (i>=count) return;
	memmove(data+i, data+(i+1), (count-i-1)*sizeof(Point));
	count--;
}

Point *PointArray::GetData(int i) {
	if (i>=count) return NULL;
	return data + i;
}


/*
\11|22/
8\1|2/3
88\|/33
---+---
77/|\44
7/6|5\4
/66|55\
*/

int los_calcvalues[8][4] = {
	{0, -1, 1, 0},
	{0, -1, -1, 0},
	{1, 0, 0, 1},
	{1, 0, 0, -1},
	{0, 1, -1, 0},
	{0, 1, 1, 0},
	{-1, 0, 0, -1},
	{-1, 0, 0, 1}
};

PathFinder::PathFinder(Level *l) {
	path_level = l;

	pointList = new PointList();

	path_openlist = new PointArray();
	path_closelist = new PointArray();
	path_pathlist = new PointArray();
	path_found = new PointArray();

	path_openlist->Init(MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS);
	path_closelist->Init(MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS);
	path_pathlist->Init(MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS);
	path_found->Init(MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS);

	memset(level_map, 0, sizeof(int)*MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS);
	// init pathing
	if (path_level != NULL) {
		Level::GridTile *gt;
		Level::GridTile *ct;
		int x, y;

		gt = path_level->GetTiles();
		for (y=0; y<MAX_GRID_CELLS; y++)
			for (x=0; x<MAX_GRID_CELLS; x++)
				ct = gt+y*MAX_GRID_CELLS+x;
				if ((gt+y*MAX_GRID_CELLS+x)->obstacle == 1) {
					int tx, ty;

					for (ty = 0; ty < MAX_GRID_PATH_CELLS; ty++)
						for (tx = 0; tx < MAX_GRID_PATH_CELLS; tx++)
							level_map[y*MAX_GRID_PATH_CELLS + ty][x*MAX_GRID_PATH_CELLS + tx] = 1;
				}

	}
}

PathFinder::~PathFinder() {
	pointList->clear(1);
	delete pointList;

	delete path_found;
	delete path_openlist;
	delete path_closelist;
	delete path_pathlist;
}


/*
 * calculates field of vision from point (START_X, START_Y) till grid end through each row or column (depends on DIR_X and DIR_Y)
 * START_X, START_Y - starting position
 * START_SLOPE, END_SLOPE - starting and ending slope for field of vision (initially should be 1 and 0)
 * DIFF - how much rows and columns we should skip starting from START_X, START_Y
 * DIR_X, DIR_Y - in which direction we skipping rows and columns (can be -1, 0 or 1)
 * STEP_X, STEP_Y - in which direction we checking current row or column (can be -1, 0 or 1)
 * RADIUS - radius for LOS
 * Warning: if (DIR_* == 0) then STEP_* != 0 and vice versa
 */
void PathFinder::FindLOS(int start_x, int start_y, float start_slope, float end_slope, int diff, int dir_x, int dir_y, int step_x, int step_y, int radius) {
	int x, y; // current position
	int calc_start, calc_end; // calculated starting and ending cells for current slopes and direction;
	int i; // cycle for running from CALC_START to CALC_END

	int error = 0; // error == 1 if we run into obstacle and have to abort our function
	int obstacle = 0; // obstacle == 0 if we're in clear cell and obstacle == 1 if we're in obstacle cell
	float new_start_slope, new_end_slope; // new slopes for another find_los call

	if (start_x+dir_x*diff<0 || start_x+dir_x*diff>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS || start_y+dir_y*diff<0 || start_y+dir_y*diff>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
		return;

	// While no obstacles
	while(error == 0) {
		float d_calc_start;
		float d_calc_end;
		int lightlevel;

		// Calculating some variables
		d_calc_start = (float)(start_slope*diff*abs(step_x)) + (float)(start_slope*diff*abs(step_y));
		d_calc_end = (float)(end_slope*diff*abs(step_x)) + (float)(end_slope*diff*abs(step_y));

		calc_start = (int)d_calc_start;
		calc_end = (int)d_calc_end;

		new_start_slope = start_slope;
		new_end_slope = end_slope;

		obstacle = -1;
		// going through current row/column
		for (i=calc_start;i>=calc_end;i--) {
			x = start_x+dir_x*diff-step_x*i;
			y = start_y+dir_y*diff-step_y*i;

			if (x<0 || x>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS || y<0 || y>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
				continue;

			if (pow((float)(start_x-x),2)+pow((float)(start_y-y),2)>pow((float)radius,2))
				continue;

			if (obstacle == -1) {
				obstacle = path_los[y][x];
				if (obstacle >= 2)
					obstacle = 0;
				if (obstacle == 1)
					error = 1;
			}

			//if we run into something new
			if (path_los[y][x] != obstacle && (path_los[y][x] == 1 || obstacle == 1)) {
				error = 1;

				obstacle = path_los[y][x];

				// if we hit obstacle now - start new find_los
				if (obstacle == 1) {
					float a, b;
					a = (float)(abs(start_x-x) + (float)(-abs(dir_x)+abs(step_x))*0.5);
					b = (float)(abs(start_y - y) + (float)(-abs(dir_y)+abs(step_y))*0.5);
					if (a <= b)
						new_end_slope = a / b;
					else
						new_end_slope = b / a;
					FindLOS(start_x, start_y, new_start_slope, new_end_slope, diff+1, dir_x, dir_y, step_x, step_y, radius);
				} else {
				// if we hit clear cell - define NEW_START_SLOPE
					float a, b;

					a = (float)(abs(start_x-x) + (float)(abs(dir_x)+abs(step_x))*0.5);
					b = (float)(abs(start_y - y) + (float)(abs(dir_y)+abs(step_y))*0.5);
					if (a <= b)
						new_start_slope =  a / b;
					else
						new_start_slope =  b / a;
				} // End of "if (obstacle)"
			} // end of "if (grid[y][x] != obstacle)"

			if (obstacle != 1) {
				lightlevel = abs(start_x - x) + abs(start_y - y) + 2;
				if (path_los[y][x] == 0 || path_los[y][x] > lightlevel)
					path_los[y][x] = lightlevel;
			}
			//outputGrid(x, y);
		} // end of "for (i=calc_start;i<=calc_end;i++)"

		// if we reached and of current row/column and error and no obstacle
		if (error && obstacle != 1 && obstacle >= 0) {
			FindLOS(start_x, start_y, new_start_slope, end_slope, diff+1, dir_x, dir_y, step_x, step_y, radius);
		}
		if (obstacle<0)
			return;

		diff++;

		if (start_x+dir_x*diff<0 || start_x+dir_x*diff>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS || start_y+dir_y*diff<0 || start_y+dir_y*diff>=MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
			return;
	} // end of "while(error == 0)"
}

/*
 * Calculates full LOS from point X,Y with radius RADIUS
 */
void PathFinder::CalculateLos(vector pos, int radius) {
	int i;
	int x, y;

	x = (int)pos[0];
	y = (int)pos[1];

// Copy current path map (should be filled with 0 and 1 only)
	memcpy(path_los, level_map, sizeof(int) * MAX_GRID_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_PATH_CELLS);

	if (x >= 0 && x<MAX_GRID_CELLS * MAX_GRID_PATH_CELLS &&
		y >= 0 && y<MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
		path_los[y][x] = 2;
	for (i=0; i<8; i++) {
		FindLOS(x, y, 1, 0, 1, los_calcvalues[i][0], los_calcvalues[i][1], los_calcvalues[i][2], los_calcvalues[i][3], radius);
	}

}

/*
 * Calculates view LOS from point X,Y with radius RADIUS, direction DIR and angle of view VIEW (no VIEW more than 90, plz)
 */
void PathFinder::CalculateViewLos(vector pos, int radius, float direction, float view) {
	float start_angle;
	float end_angle;
	float sector_slope[8][2];
	float current_angle;
	int i;
	int x, y;

	x = (int)pos[0];
	y = (int)pos[1];

// Copy current path map (should be filled with 0 and 1 only)
	memcpy(path_los, level_map, sizeof(int) * MAX_GRID_CELLS * MAX_GRID_CELLS * MAX_GRID_PATH_CELLS * MAX_GRID_PATH_CELLS);

	if (x >= 0 && x<MAX_GRID_CELLS * MAX_GRID_PATH_CELLS &&
		y >= 0 && y<MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
		path_los[y][x] = 2;

	for (i=0; i<8; i++) {
		sector_slope[i][0] = 1;
		sector_slope[i][1] = 0;
	}
	start_angle = direction - view;
	end_angle = direction + view;

	if (start_angle < 0)
		start_angle += 360;
	if (end_angle >= 360)
		end_angle -= 360;

	if (start_angle > end_angle)
		end_angle += 360;

	current_angle = 0;

// calculating slope for each sector
	for (i=0; i<8; i++) {
		float sa, ea;

		sa = current_angle - 45;
		if (sa < 0)
			sa += 360;
		if (sa < end_angle - 360)
			sa += 360;
		ea = current_angle;
		while(sa > ea)
			ea += 360;

		if (sa >= start_angle && ea <= end_angle) {
			// OK HERE
		} else if (ea < start_angle && end_angle - sa < 360) {
			sector_slope[i][0] = -1; // NO LOS here
		} else {
			float value;

			if (sa < start_angle) {
				value = (start_angle - sa) / 45.0f;
				if (value >= 0) {
					if (i%2 == 0)
						sector_slope[i][0] = 1 - value;
					else
						sector_slope[i][1] = value;
				}
			}
			if (ea > end_angle) {
				value = (ea - end_angle) / 45.0f;
				if (value >= 0) {
					if (i%2 == 1)
						sector_slope[i][0] = 1 - value;
					else
						sector_slope[i][1] = value;
				}
			}
		}
		current_angle+=45;
	}
	for (i=0; i<8; i++) {
		if (sector_slope[i][0] >= 0)
			FindLOS(x, y, sector_slope[i][0], sector_slope[i][1], 1, los_calcvalues[i][0], los_calcvalues[i][1], los_calcvalues[i][2], los_calcvalues[i][3], radius);
	}
}


void PathFinder::CalculatePath(vector src, vector dst) {
	int px, py;
	int tx, ty;
	int sx, sy, dx, dy;
	float end_dist = 9999;
	float dist = 0;
	Point *p;
	float sqrt2 = sqrt(2.0f);
	int bx, by;
	int num;
	Point startP, endP;

	dynamicReaderFree(pr, Point);

	sx = (int)src[0];
	sy = (int)src[1];
	dx = (int)dst[0];
	dy = (int)dst[1];

	path_openlist->SetCount(0);
	path_closelist->SetCount(0);
	path_pathlist->SetCount(0);
	path_found->SetCount(0);

	px = sx;
	py = sy;
	if (px < 0 || px >= MAX_GRID_CELLS * MAX_GRID_PATH_CELLS ||
			py < 0 || py >= MAX_GRID_CELLS * MAX_GRID_PATH_CELLS)
		return;

	memset(path_path, 0, sizeof(path_path));
	memset(path_usedpath, 0, sizeof(path_usedpath));

// Simple A* pathfinding without diagonal movements
	path_usedpath[py][px] = 1;
	path_openlist->AddData(px, py);
	// going until no open nodes
	while(path_openlist->GetCount() > 0) {
		// get current node
		p = path_openlist->GetFirstElement();

		px = p->x;
		py = p->y;

		if (px == dx && py == dy)
			end_dist = path_path[py][px];
		else if (end_dist > path_path[py][px]) {
			// searching all neighbours
			for (ty=-1;ty<=1;ty++)
				for (tx=-1;tx<=1;tx++) {
					if (tx != 0 || ty != 0) {
						if (tx == 0 || ty == 0)
							dist = path_path[py][px] + 1;
						else
							dist = path_path[py][px] + sqrt2;

						if (dist < end_dist && px + tx >= 0 && px + tx < MAX_GRID_CELLS * MAX_GRID_PATH_CELLS &&
								py + ty >= 0 && py + ty < MAX_GRID_CELLS * MAX_GRID_PATH_CELLS) {
							// if current node passable && (not visited || it's dist > calculated dist)
							if (level_map[py+ty][px+tx] == 0 && (path_usedpath[py+ty][px+tx] == 0 || path_path[py+ty][px+tx] > dist)) {
								// if node is not visited - add into openlist
								if (path_usedpath[py+ty][px+tx] == 0)
									path_openlist->AddData(px+tx, py+ty);
								// else if node is in closedlist - open it again
								else if (path_usedpath[py+ty][px+tx] == 2) {
									path_closelist->RemoveData(px+tx, py+ty);
									path_openlist->AddData(px+tx, py+ty);
								}
								// we do not have to do anything with node in openlist (only change distance)
								path_usedpath[py+ty][px+tx] = 1;
								path_path[py+ty][px+tx] = dist;
							}
						}
					}
				}
		}

		// add current node into closed list
		path_usedpath[py][px] = 2;
		path_closelist->AddData(px, py);
		path_openlist->RemoveFirstElement();		
	}	

	// Now we have to move backwards using found path
	px = (int)dx;
	py = (int)dy;
	while(px != sx || py != sy) {
		dist = path_path[py][px];
		bx = -2;
		by = -2;
		for (ty = -1; ty <= 1; ty++)
			for (tx = -1; tx <= 1; tx++) {
				if (px + tx >= 0 && px + tx < MAX_GRID_CELLS * MAX_GRID_PATH_CELLS &&
					py + ty >= 0 && py + ty < MAX_GRID_CELLS * MAX_GRID_PATH_CELLS &&
					(tx != 0 || ty != 0) && path_path[py+ty][px+tx] < dist && path_usedpath[py][px] > 0) {
					bx = px + tx;
					by = py + ty;
				}
			}
		if (bx >= 0) {
			path_found->AddData(bx, by);
			px = bx;
			py = by;
		} else {
			path_found->SetCount(0);
			break;
		}
	}

	// optimize path now
	if (path_found->GetCount() <= 2)
		return;

	pr->attach(pointList);

	num = path_found->GetCount() - 3;
	startP.x = sx;
	startP.y = sy;
	while(num >= 0 && path_found->GetCount() > 2) {
		p = path_found->GetData(num);

		pointList->clear(1);
		Math::BresenhamLine(startP, *p, pointList);
		p = pr->getFirstElement();
		while(p) { 
			if (level_map[p->y][p->x] == 1)
				break;
			p = pr->getNextElement();
		}
		if (p) {
			p = path_found->GetData(num + 1);
			startP.x = p->x;
			startP.y = p->y;
		} else {
			path_found->RemoveDataByNum(num + 1);
		}
		num--;
	}
}
