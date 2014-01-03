#pragma once

#include <string>
#include <vector>
#include <map>

#include <windows.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glut.h>	
#include "SOIL.h"
#include "Log.h"

struct BuildHeader {
	// BILD
	int magicNumber;
	// 6
	int version;
	int boneCount;
	int frameCount;
};

struct BoneFrameInfo {
	int index;
	int duration;
	// bbox
	float x;
	float y;
	float width;
	float height;
	// vb start index
	int vStartIndex;
	// num verts
	int vCount;
};

struct BoneInfo {
	unsigned int id;
	int frameCount;
	typedef std::vector<BoneFrameInfo*> BoneFrameList;
	BoneFrameList infos;
};


struct Vertex {
	float x;
	float y;
	float z;
	float u;
	float v;
	float w;
};


struct AnimHeader {
	// ANIM
	int magicNumber;
	// 4
	int version;
	int elementCount;
	int frameCount;
	int eventCount;
};


struct AnimationFrameElement {
	int boneID;
	int frameID;
	int layerID;
	float a;
	float b;
	float c;
	float d;
	float tx;
	float ty;
	float tz;
};

struct AnimationFrame {
	float x;
	float y;
	float w;
	float h;
	int eventCount;
	int eventID;
	int elementCount;
	typedef std::vector<AnimationFrameElement*> ElementList;
	ElementList elements;
};

struct Animation {
	std::string name;
	char facing;
	int boneID;
	float frameRate;
	int frameCount;
	std::vector<AnimationFrame*> frames;
};

class Animator
{
public:
	Animator(void);
	~Animator(void);
	bool load(const char* buildFileName, const char* AnimFileName);
	bool loadBuildFile(const char* buildFileName);
	bool loadAnimFile(const char* AnimFileName);
	void update();
	void render();
	bool play(const char* name);
	BoneInfo* getBone(int id);
	int currentFrame;
private:
	BuildHeader buildHeader;
	AnimHeader animHeader;
	unsigned char* rptr;
	unsigned char* buffer;

	std::vector<std::string> textures;

	typedef std::vector<BoneInfo*> BoneList;
	BoneList bones;
	std::map<int, std::string> boneNames;
	std::map<int, std::string> layerNames;

	typedef std::vector<Vertex*> VertexList;
	VertexList vertexes;

	typedef std::vector<Animation*> AnimationList;
	AnimationList animations;

	std::string name;

	Animation* currentAnimation;
	
	GLuint	texture[1];			// Storage For One Texture ( NEW )
	CLog log;

	bool readBuildHeader();
	bool readAnimHeader();
	char* readString();
	int readInt();
	char readByte();
	unsigned int readUInt();
	float readFloat();
	void printf(float x, float y, const char * fmt, ...);
	void print(float x, float y, const char* string, float r = 1.0f, float g = 1.0f, float b = 1.0f);
	unsigned int strHash(std::string val);
};

