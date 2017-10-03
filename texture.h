class TextureAtlasSubInfo {
public:
	TextureAtlasSubInfo();
	~TextureAtlasSubInfo();
	char name[256];
	float x0, y0, x1, y1;
};

class TextureAtlasSubInfoList : public List<TextureAtlasSubInfo> {
};

class TextureInfo {
public:
	TextureInfo();
	~TextureInfo();

	char name[256];
	int texnum;

	int width;
	int height;

	enum TextureType {
		TEXTURE_COMMON,
		TEXTURE_ATLAS
	};

	int textureType;
};

class Texture3DSubInfo {
public:
	Texture3DSubInfo();
	~Texture3DSubInfo();
	char name[256];
	int depth;
};

class Texture3DSubInfoList : public List<Texture3DSubInfo> {
};

class TextureAtlasInfo : public TextureInfo {
protected:
	int *grid;

public:
	TextureAtlasInfo();
	~TextureAtlasInfo();

	int GenerateTextureAtlas(int w, int h, int t);
	int FindPlaceForSub(int w, int h, int *x, int *y);

	TextureAtlasSubInfoList *textureAtlasSubInfoList;
};

class Texture3DInfo : public TextureInfo {
protected:
	int depth_max;
	int depth_current;

public:
	Texture3DInfo();
	~Texture3DInfo();

	int GenerateTexture3D(int w, int h, int depth, int t);
	int GetMaxDepth();
	int GetCurrentDepth();
	void SetCurrentDepth(int i);

	Texture3DSubInfoList *texture3DSubInfoList;
};

class TextureInfoList : public List<TextureInfo> {
};

class Texture {
public:
	Texture();
	~Texture();

	void LoadTexturesList(void);
	void LoadTextureIntoAtlas(TextureAtlasInfo *ti, char *filename, char *name);
	int LoadTexture(char *filename, char *name);
	void LoadTexture3D(Texture3DInfo *ti, char *filename, char *name);
	TextureInfo *FindTexture(char *name);
	TextureInfo *FindTexture(int t);
	TextureAtlasSubInfo *FindTextureAtlasSub(TextureAtlasInfo *ti, char *name);
	Texture3DSubInfo *FindTexture3DSub(Texture3DInfo *ti, char *name);
	void Bind(int texnum, int unit);
	void Bind3D(int texnum, int unit);

protected:
	TextureInfoList *textureInfoList;

	int texnum_max;
	int texnum_current[4];

	typedef struct _TargaHeader {
		unsigned char id_length, colormap_type, image_type;
		unsigned short colormap_index, colormap_length;
		unsigned char colormap_size;
		unsigned short x_origin, y_origin, width, height;
		unsigned char pixel_size, attributes;
	} TargaHeader;

	TargaHeader targa_header;
	int image_width, image_height, image_pixelsize;

	int fgetLittleShort(FILE *f);
	int fgetLittleLong(FILE *f);
	byte *LoadTGA(FILE *fin);
};

extern Texture *texture;
