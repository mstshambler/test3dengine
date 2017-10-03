class Level {
public:
	enum GridTileTypes {
		GRIDTILE_EMPTY,
		GRIDTILE_WATER,
		GRIDTILE_GRASS,
		GRIDTILE_DIRT,
		GRIDTILE_STONE,
		GRIDTILE_WOOD,
		GRIDTILE_SAND,
		GRIDTILE_GRAVEL,
		GRIDTILE_SNOW,
		GRIDTILE_BLACKSTONE,
		GRIDTILE_COBBLESTONE,
		GRIDTILE_STONEPANEL,
		GRIDTILE_WOODPANEL,
		GRIDTILE_SANDWALL,
		GRIDTILE_SANDPANEL,
		GRIDTILE_BRICKS,
		GRIDTILE_BLACKSTONEPANEL,
		GRIDTILE_GOLD,
		GRIDTILE_IRON,
		GRIDTILE_COAL,
		GRIDTILE_REDSTONE,
		GRIDTILE_DIAMONDS,
		GRIDTILE_BLUESTONE,
		GRIDTILE_GOLDPANEL,
		GRIDTILE_BLUESTONEPANEL,
		GRIDTILE_LAST
	};

	enum GridTileLevels {
		GRIDLEVEL_BASE,
		GRIDLEVEL_FLOOR,
		GRIDLEVEL_WALK,
		GRIDLEVEL_AIR,
	};

	struct GridTile {
		int obstacle;
		int levelType[GRIDLEVEL_AIR + 1];
		float height;
	};

	Level();
	~Level();

	void Render(int mode); // 0 - depth only, 1 - normal, 2 - light
	void RenderLighting();
	void RenderParticles();

	bool CheckCollision(vector initpos, vector move, vector newpos, float radius, timeval difftime);

	GridTile *GetTiles();
	LightSourceList *GetLightSources();

	void SetChanged();

	ParticleSystem *GetParticleSystem();

protected:
	GridTile map_tiles[MAX_GRID_CELLS][MAX_GRID_CELLS];

	void PrepareShadowVolume(Face4 *face, LightSource *ls, float *vbo, unsigned int *count);
	void PrepareShadowVolume(Face3 *face, LightSource *ls, float *vbo, unsigned int *count);

	int changed;
	LightSourceList *lightSources;

	void CreateVBOs();

	Texture3DInfo *geometryTextures;

	unsigned int geometryVbo;
	unsigned int geometryVboCount;
	// for future support for indexes
	unsigned int *geometryIndex;
	unsigned int geometryIndexCount;

	enum GridTileFaceType {
		GRIDFACE_TOP,
		GRIDFACE_BOTTOM,
		GRIDFACE_NORTH,
		GRIDFACE_EAST,
		GRIDFACE_SOUTH,
		GRIDFACE_WEST
	};

	int CheckFaceForTile(int x, int y, int type, int level);
	void GetFaceForTile(int x, int y, int type, int level, Face4 *face);

	// TODO: Move particle system into extern
	ParticleSystem *particleSystem;
};

extern Level *level;
