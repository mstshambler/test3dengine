class LightSource {
public:
	LightSource();
	~LightSource();

	float pos[3];
	int radius; // not used right now
	float color[3];
	int changed;
	
	unsigned int lightVbo;
	unsigned int lightVboCount;

};

class LightSourceList : public List<LightSource> {
};
