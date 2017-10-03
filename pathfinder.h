class PointArray {
protected:
	int count;
	int size;
	Point *data;

public:
	PointArray();
	~PointArray();
	
	void Init(int i);
	void AddData(int x, int y);
	void RemoveData(int x, int y);
	Point *GetFirstElement();
	void RemoveFirstElement();
	int GetCount();
	void SetCount(int i);
	void RemoveDataByNum(int i);
	Point *GetData(int i);
};

class Level;

class PathFinder {
protected:
	PointArray *path_openlist;
	PointArray *path_closelist;
	PointArray *path_pathlist;
	PointArray *path_found;
	
	PointList *pointList;

	int level_map[MAX_GRID_CELLS * MAX_GRID_PATH_CELLS][MAX_GRID_CELLS * MAX_GRID_PATH_CELLS];
	int path_los[MAX_GRID_CELLS * MAX_GRID_PATH_CELLS][MAX_GRID_CELLS * MAX_GRID_PATH_CELLS];
	float path_path[MAX_GRID_CELLS * MAX_GRID_PATH_CELLS][MAX_GRID_CELLS * MAX_GRID_PATH_CELLS];
	int path_usedpath[MAX_GRID_CELLS * MAX_GRID_PATH_CELLS][MAX_GRID_CELLS * MAX_GRID_PATH_CELLS];

	void FindLOS(int start_x, int start_y, float start_slope, float end_slope, int diff, int dir_x, int dir_y, int step_x, int step_y, int radius);

	Level *path_level;

public:
	PathFinder(Level *l);
	~PathFinder();

	void CalculateLos(vector pos, int radius);
	void CalculateViewLos(vector pos, int radius, float direction, float view);
	void CalculatePath(vector src, vector dst);
};

extern PathFinder *pathFinder;
