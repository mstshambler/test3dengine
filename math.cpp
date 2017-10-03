#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "math.h"

// Point class
Point::Point() {
	x = y = 0;
}

Point::~Point() {
}


// PointList
void PointList::addPoint(int x, int y) {
	Point *p = new Point();

	p->x = x;
	p->y = y;
	this->addElement(p);
}



void Math::BresenhamLine(Point start, Point end, PointList *pointList) {
	int dir[2];
	int delta[2];
	int base = 0;
	int error;
	int pos[2][2];

	delta[0] = (end.x > start.x ? (dir[0] = 1, end.x - start.x) : (dir[0] = -1, start.x - end.x) ) << 1;
	delta[1] = (end.y > start.y ? (dir[1] = 1, end.y - start.y) : (dir[1] = -1, start.y - end.y) ) << 1;

	pos[0][0] = start.x;
	pos[0][1] = start.y;
	pos[1][0] = end.x;
	pos[1][1] = end.y;

	pointList->addPoint(start.x, start.y);

	if (delta[1] > delta[0])
		base = 1;

	error = delta[1-base] - (delta[base] >> 1);
	while(pos[0][base] != pos[1][base]) {
		if (error >= 0) {
			if (error || dir[base] > 0) {
				pos[0][1-base] += dir[1-base];
				error -= delta[base];
			}
		}

		pos[0][base] += dir[base];
		error += delta[1-base];

		pointList->addPoint(pos[0][0], pos[0][1]);
	}
}


void Math::BresenhamCircle(Point center, int radius, PointList *pointList) {
	int error = -radius;
	int x = radius;
	int y = 0;

	while(x >= y) {
		pointList->addPoint(center.x + x, center.y + y);
		if (x != 0)
			pointList->addPoint(center.x - x, center.y + y);
		if (y != 0)
			pointList->addPoint(center.x + x, center.y - y);
		if (x != 0 && y != 0)
			pointList->addPoint(center.x - x, center.y - y);

		if (x != y) {
			pointList->addPoint(center.x + y, center.y + x);
			if (y != 0)
				pointList->addPoint(center.x - y, center.y + x);
			if (x != 0)
				pointList->addPoint(center.x + y, center.y - x);
			if (x != 0 && y != 0)
				pointList->addPoint(center.x - y, center.y - x);
		}

		error += y;
		y++;
		error += y;

		if (error >= 0) {
			error -= x;
			x--;
			error -= x;
		}
	}
}

void Math::NormalizePlane(float frustum[6][4], int side) {
	float magnitude = (float)sqrt( frustum[side][PLANE_A] * frustum[side][PLANE_A] + 
		frustum[side][PLANE_B] * frustum[side][PLANE_B] + 
		frustum[side][PLANE_C] * frustum[side][PLANE_C] );
	frustum[side][PLANE_A] /= magnitude;
	frustum[side][PLANE_B] /= magnitude;
	frustum[side][PLANE_C] /= magnitude;
	frustum[side][PLANE_D] /= magnitude;
}

void Math::PlaneEquation(vector p0, vector p1, vector p2, float plane[4]) {
	plane[PLANE_A] = p0[1]*(p1[2] - p2[2]) + p1[1]*(p2[2] - p0[2]) + p2[1]*(p0[2] - p1[2]);
	plane[PLANE_B] = p0[2]*(p1[0] - p2[0]) + p1[2]*(p2[0] - p0[0]) + p2[2]*(p0[0] - p1[0]);
	plane[PLANE_C] = p0[0]*(p1[1] - p2[1]) + p1[0]*(p2[1] - p0[1]) + p2[0]*(p0[1] - p1[1]);
	plane[PLANE_D] = -(p0[0] * (p1[1]*p2[2] - p2[1]*p1[2]) + p1[0] * (p2[1]*p0[2] - p0[1]*p2[2]) + p2[0] * (p0[1]*p1[2] - p1[1]*p0[2]));
}

float Math::VectorLength(vector v) {
	int	i;
	float length;

	length = 0;
	for (i=0; i<3; i++)
		length += v[i]*v[i];
	length = sqrt(length);

	return length;
}

void Math::NormalizeVector(vector v) {
	float l;
	int i;

	l = VectorLength(v);
	v[0] = v[0] / l;
	v[1] = v[1] / l;
	v[2] = v[2] / l;
	for (i=0; i<3; i++)
		if (fabs(v[i]) < 0.0001)
			v[i] = 0.0;
}

void Math::VectorCopy(vector src, vector dst) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

void Math::AngleVectors (vector angles, vector forward, vector right, vector up) {
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	int YAW, PITCH, ROLL;
	YAW = 2;
	PITCH = 0;
	ROLL = 1;

	angle = angles[YAW] * (float)(M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (float)(M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * (float)(M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward[1] = cp*cy;
	forward[0] = cp*sy;
	forward[2] = sp;
/*
	right[1] = (sr*sp*cy-cr*sy);
	right[0] = -(sr*sp*sy-cr*cy);
	right[2] = -sr*cp;

// -1!
	up[1] = (cr*sp*cy+sr*sy);
	up[0] = -(cr*sp*-sy-sr*cy);
	up[2] = -cr*cp;
*/
	right[1] = (sr*sp*cy+cr*-sy);
	right[0] = (sr*sp*sy+cr*cy);
	right[2] = -1*sr*cp;
	
	up[1] = (cr*sp*cy+-sr*-sy);
	up[0] = (cr*sp*sy+-sr*cy);
	up[2] = -cr*cp;
}

float Math::DotProduct (vector v1, vector v2) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

float Math::CalcDistance(vector p1, vector p2) {
	return sqrt(pow(p1[0]-p2[0],2)+pow(p1[1]-p2[1],2)+pow(p1[2]-p2[2],2));
}

void Math::VectorSubtract (vector veca, vector vecb, vector out) {
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

void Math::VectorAdd (vector veca, vector vecb, vector out) {
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}

void Math::VectorCrossProduct (vector v1, vector v2, vector cross) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void Math::VectorMA (vector veca, vector vecb, float scale, vector out) {
	out[0] = veca[0] + scale*vecb[0];
	out[1] = veca[1] + scale*vecb[1];
	out[2] = veca[2] + scale*vecb[2];
}

void Math::VectorMS (vector veca, vector vecb, float scale, vector out) {
	out[0] = veca[0] - scale*vecb[0];
	out[1] = veca[1] - scale*vecb[1];
	out[2] = veca[2] - scale*vecb[2];
}

void Math::PlaneNormal(vector p0, vector p1, vector p2, vector normal) {
	vector v1, v2;

	VectorSubtract(p1, p0, v1);
	VectorSubtract(p2, p0, v2);

	VectorCrossProduct(v2, v1, normal);
	NormalizeVector(normal);
}

void Math::VectorMultiply(vector veca, vector vecb, vector out) {
	out[0] = veca[0] * vecb[0];
	out[1] = veca[1] * vecb[1];
	out[2] = veca[2] * vecb[2];
}

void Math::VectorMultiplyRev(vector veca, vector vecb, vector out) {
	out[0] = veca[0] * (-vecb[0]);
	out[1] = veca[1] * (-vecb[1]);
	out[2] = veca[2] * (-vecb[2]);
}

float Math::IntersectPlane(vector pos, vector move, vector planePos, vector planeNormal, float *v) {
	vector diff;

	Math::VectorSubtract(planePos, pos, diff);
	*v = Math::DotProduct(planeNormal, move);

	if (fabs(*v) < 0.0001)
		return -1;

	Math::VectorSubtract(planePos, pos, diff);
	return Math::DotProduct(planeNormal, diff) / *v;
}

float Math::IntersectRaySphere(vector pos, vector move, vector ipos, float radius) {
	vector diff;
	float c;
	float v;
	float d;

	VectorSubtract(ipos, pos, diff);
	c = VectorLength(diff);
	v = DotProduct(diff, move);
	d = pow(radius, 2) - (pow(c, 2) - pow(v, 2));

	// no intersection
	if (d < 0.0) 
		return -1.0;

	// Return the distance to the intersection
	return (v - sqrt(d));
}

int Math::PointInTriangle(vector p0, vector p1, vector p2, vector p) {
	double angles = 0;
	vector v1, v2, v3;

	VectorSubtract(p, p0, v1);
	VectorSubtract(p, p1, v2);
	VectorSubtract(p, p2, v3);

	NormalizeVector(v1);
	NormalizeVector(v2);
	NormalizeVector(v3);

	angles += acos(DotProduct(v1, v2));
	angles += acos(DotProduct(v2, v3));
	angles += acos(DotProduct(v3, v1));

	if (fabs(angles - 2*M_PI) <= 0.001)
		return 1;
	return 0;
}

int Math::PointInQuad(vector p0, vector p1, vector p2, vector p3, vector p) {
	float angles = 0;
	vector v1, v2, v3, v4;

	VectorSubtract(p, p0, v1);
	VectorSubtract(p, p1, v2);
	VectorSubtract(p, p2, v3);
	VectorSubtract(p, p3, v4);

	NormalizeVector(v1);
	NormalizeVector(v2);
	NormalizeVector(v3);
	NormalizeVector(v4);

	angles += acos(DotProduct(v1, v2));
	angles += acos(DotProduct(v2, v3));
	angles += acos(DotProduct(v3, v4));
	angles += acos(DotProduct(v4, v1));

	if (fabs(angles - 2*M_PI) <= 0.001)
		return 1;
	return 0;
}

int Math::PointInSphere(vector p, vector c, float radius) {
	double d;
	vector diff;

	VectorSubtract(p, c, diff);
	d = VectorLength(diff);
	if (d <= radius)
		return 1;
	return 0;
}

void Math::ClosestPointOnLine(vector p0, vector p1, vector p, vector res) {
	vector c, v;
	float d;
	float t;

	VectorSubtract(p, p0, c);
	VectorSubtract(p1, p0, v);
	d = VectorLength(v);

	NormalizeVector(v);
	t = DotProduct(v, c);

	// point leaves line on P0
	if (t < 0.0f)
		VectorCopy(p0, res);
	// point leaves line on P1
	else if (t > d)
		VectorCopy(p1, res);
	else {
	// Point somewhere between P0 and P1
		v[0] *= t;
		v[1] *= t;
		v[2] *= t;

		VectorAdd(p0, v, res);
	}
}


void Math::ClosestPointOnTriangle(vector p0, vector p1, vector p2, vector p, vector res) {
	vector v[3];
	float d[3];
	vector temp;
	int min;
	int i;

	ClosestPointOnLine(p0, p1, p, v[0]);
	ClosestPointOnLine(p1, p2, p, v[1]);
	ClosestPointOnLine(p2, p0, p, v[2]);

	for (i=0; i<3; i++) {
		VectorSubtract(p, v[i], temp);
		d[i] = VectorLength(temp);
	}
	min = 0;

	if (d[1] < d[min])
		min = 1;

	if (d[2] < d[min])
		min = 2;

	VectorCopy(v[min], res);
}

void Math::ClosestPointOnQuad(vector p0, vector p1, vector p2, vector p3, vector p, vector res) {
	vector v[4];
	float d[4];
	vector temp;
	int min;
	int i;

	ClosestPointOnLine(p0, p1, p, v[0]);
	ClosestPointOnLine(p1, p2, p, v[1]);
	ClosestPointOnLine(p2, p3, p, v[2]);
	ClosestPointOnLine(p3, p0, p, v[3]);

	for (i=0; i<4; i++) {
		VectorSubtract(p, v[i], temp);
		d[i] = VectorLength(temp);
	}
	min = 0;
	
	for (i=1; i<4; i++) {
		if (d[i] < d[min])
			min = i;
	}

	VectorCopy(v[min], res);
}

void Math::VectorInvert(vector v) {
	v[0] *= -1.0;
	v[1] *= -1.0;
	v[2] *= -1.0;
}

void Math::VectorScale(vector v, float s) {
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
}

float Math::CheckCollisionQuad(vector p0, vector p1, vector p2, vector p3, vector pos, vector move, vector newpos, float radius, vector intersect) {
	float v;
	vector planeNormal;
	float distance;
	vector entityPoint;
	vector normmove;

	Math::VectorCopy(move, normmove);
	Math::NormalizeVector(normmove);

	Math::PlaneNormal(p0, p1, p2, planeNormal);
	Math::VectorMS(pos, planeNormal, radius, entityPoint);

	distance = Math::IntersectPlane(entityPoint, normmove, p0, planeNormal, &v);
	if (v < -0.0001) {
		float moveDistance;
		float moveCurDistance;

		Math::VectorMA(entityPoint, normmove, distance, intersect);

		// intersection point is not on quad - have to check nearest points of quad
		if (Math::PointInQuad(p0, p1, p2, p3, intersect) == 0) {
			vector invertnormmove;

			Math::VectorCopy(normmove, invertnormmove);
			Math::VectorInvert(invertnormmove);

			Math::ClosestPointOnQuad(p0, p1, p2, p3, intersect, intersect);
			distance = Math::IntersectRaySphere(intersect, invertnormmove, pos, radius);
			if (distance > 0.0) {
				entityPoint[0] = intersect[0] + distance * invertnormmove[0];
				entityPoint[1] = intersect[1] + distance * invertnormmove[1];
				entityPoint[2] = intersect[2] + distance * invertnormmove[2];
			}
		}

		moveDistance = Math::CalcDistance(pos, newpos);
		moveCurDistance = Math::CalcDistance(entityPoint, intersect);

		if (moveCurDistance <= moveDistance) {
			return moveCurDistance;
		}
	}
	return -1.0;
}

void Math::MakeSlideAfterCollision(vector pos, vector move, vector newpos, vector intersect, float distance, vector newmove, vector nearpos) {
	vector slidePlanePos;
	vector slidePlaneNormal;
	vector collisionPos;
	vector collisionMove;
	float l;
	float v;
	vector slideDestPos;
	
	Math::VectorCopy(move, collisionMove);
	Math::VectorScale(collisionMove, (distance - 0.1f) / Math::VectorLength(collisionMove));
	Math::VectorAdd(pos, collisionMove, collisionPos);

	Math::VectorCopy(intersect, slidePlanePos);
	Math::VectorSubtract(collisionPos, intersect, slidePlaneNormal);

	l = Math::IntersectPlane(newpos, slidePlaneNormal, slidePlanePos, slidePlaneNormal, &v);
	Math::VectorMA(newpos, slidePlaneNormal, l, slideDestPos);

	Math::VectorSubtract(slideDestPos, intersect, newmove);
	Math::VectorCopy(collisionPos, nearpos);
}
