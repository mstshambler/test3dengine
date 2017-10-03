class Point {
public:
	Point();
	~Point();

	int x, y;
};

class PointList : public List<Point> {
public:
	void addPoint(int x, int y);
};

union Vector {
	struct vectorValues {
		float x, y, z;
	} p;
	float v[3];
};

typedef float vector[3];

class Math {
public:
	enum FrustumSide
	{
		FRUSTUM_RIGHT	= 0,		// The RIGHT side of the frustum
		FRUSTUM_LEFT	= 1,		// The LEFT	 side of the frustum
		FRUSTUM_BOTTOM	= 2,		// The BOTTOM side of the frustum
		FRUSTUM_TOP		= 3,		// The TOP side of the frustum
		FRUSTUM_BACK	= 4,		// The BACK	side of the frustum
		FRUSTUM_FRONT	= 5			// The FRONT side of the frustum
	}; 

	enum PlaneData
	{
		PLANE_A = 0,				// The X value of the plane's normal
		PLANE_B = 1,				// The Y value of the plane's normal
		PLANE_C = 2,				// The Z value of the plane's normal
		PLANE_D = 3				// The distance the plane is from the origin
	};

	static void BresenhamLine(Point start, Point end, PointList *pointList);
	static void BresenhamCircle(Point center, int radius, PointList *pointList);
	
	static void NormalizeVector(vector v);
	static float VectorLength(vector v);
	static void VectorCopy(vector src, vector dst);
	static void VectorSubtract (vector veca, vector vecb, vector out);
	static void VectorAdd (vector veca, vector vecb, vector out);
	static void VectorCrossProduct (vector v1, vector v2, vector cross);
	static float DotProduct (vector v1, vector v2);
	static float CalcDistance(vector p1, vector p2);
	static void VectorMA (vector veca, vector vecb, float scale, vector out);
	static void VectorMS (vector veca, vector vecb, float scale, vector out);
	static void VectorMultiply(vector veca, vector vecb, vector out);
	static void VectorMultiplyRev(vector veca, vector vecb, vector out);
	static void VectorInvert(vector v);
	static void VectorScale(vector v, float s);

	static void AngleVectors(vector angles, vector forward, vector right, vector up);

	static void NormalizePlane(float frustum[6][4], int side);
	static void PlaneEquation(vector p0, vector p1, vector p2, float plane[4]);
	static void PlaneNormal(vector p0, vector p1, vector p2, vector normal);
	static float IntersectPlane(vector pos, vector move, vector planePos, vector planeNormal, float *v);
	static float IntersectRaySphere(vector pos, vector move, vector ipos, float radius);

	static int PointInTriangle(vector p0, vector p1, vector p2, vector p);
	static int PointInQuad(vector p0, vector p1, vector p2, vector p3, vector p);
	static int PointInSphere(vector p, vector c, float radius);
	static void ClosestPointOnLine(vector p0, vector p1, vector p, vector res);
	static void ClosestPointOnTriangle(vector p0, vector p1, vector p2, vector p, vector res);
	static void ClosestPointOnQuad(vector p0, vector p1, vector p2, vector p3, vector p, vector res);

	static float CheckCollisionQuad(vector p0, vector p1, vector p2, vector p3, vector pos, vector move, vector newpos, float radius, vector intersect);
	static void MakeSlideAfterCollision(vector pos, vector move, vector newpos, vector intersect, float distance, vector newmove, vector nearpos);
};
