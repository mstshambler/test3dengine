struct Face {
	int vertexes[3];
	vector normal;
	vector tc[3];
};

struct Vertex {
	vector pos;
};

struct Face3 {
	vector vertexes[3];
	vector normal;
	vector tc[3];
};

struct Face4 {
	vector vertexes[4];
	vector normal;
	vector tc[4];
};

class Render {
protected:
	char *gl_vendor;
	char *gl_renderer;
	char *gl_version;
	char *gl_extensions;

	int screen_width;
	int screen_height;

	float m_Frustum[6][4]; // frustum

	int shaderLighting;
	int shaderWater;

	int extension_twoSidedStencil;
	int extension_3dTextures;

public:
	Render();
	~Render();

	struct camera_pos {
		vector pos;
		vector viewangles;
	};
	struct camera_pos camera;

	void Init(int width, int height);
	void Set3DDepthMode();
	void Set3DTextureMode();
	void Set3DParticlesMode();
	void Set3DPostLightingMode();
	void Begin(int width, int height);
	void End();
	void Set2DMode(int width, int height);
	void End2DMode();
	void Frame(int mousex, int mousey, vector worldpos);
	void Shutdown();
	void Clear ();

	int GetScreenWidth();
	int GetScreenHeight();
	void Reshape(int width, int height);

	float *getFrustum();
	void CalculateFrustum();
	bool PointInFrustum(float x, float y, float z);

	int GetShaderLighting();
	int GetShaderWater();

	int GetExtensionTwoSidedStencil();
	int GetExtension3DTextures();

	void ReadCoordsUnderCursor(int x, int y, vector worldpos);
};

extern Render *render;
