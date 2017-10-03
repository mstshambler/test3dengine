class Model {
public:

	Vertex *vertexes;
	int vertexes_num;

	Face *faces;
	int faces_num;

	int texnum;
	int vbo;

	Model();
	~Model();

	void Load(char *filename);
	void CreateVBO();
};
