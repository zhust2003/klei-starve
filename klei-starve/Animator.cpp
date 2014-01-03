#include "Animator.h"
//#include <cstdio>
#define M_PI       3.14159265358979323846

Animator::Animator(void) : rptr(0),
						   buffer(0),
						   log(),
						   currentAnimation(NULL),
						   currentFrame(0)
{
	log.Init("log.html");
}


Animator::~Animator(void)
{
}

bool Animator::load(const char* fileName, const char* animFileName) {
	return loadBuildFile(fileName) && loadAnimFile(animFileName);
}

bool Animator::loadBuildFile(const char* fileName) {
	log.Write(COLOR_WHITE, "开始解析Build.bin文件");

	int size = 0;
	FILE * f;
	
	if(! (f = fopen(fileName, "rb")))
	{
		log.Write(COLOR_WHITE, "Could not open build file %s", fileName);
		return false;
	}

	//check file size and read it all into the buffer
	int iStart = ftell(f);
	fseek(f, 0, SEEK_END);
	int iEnd = ftell(f);
	fseek(f, 0, SEEK_SET);
	size = iEnd - iStart;

	//Allocate memory for whole file
	buffer = new unsigned char[size];
	rptr = buffer;

	if(! buffer)
	{
		log.Write(COLOR_WHITE, "Could not allocate memory for %s", fileName);
		return false;
	}

	//Load file into buffer
	if(fread(buffer, 1, size, f) != (unsigned)size)
	{
		log.Write(COLOR_WHITE, "Could not read from %s", fileName);
		delete [] buffer;
		return false;
	}

	//close the file, we don't need it anymore
	fclose(f);

	if (! readBuildHeader()) {
		log.Write(COLOR_WHITE, "%s is not a valid bin file", fileName);
		delete [] buffer;
		return false;
	}

	// 读取动画名称
	name = readString();
	

	// 资源名称
	int textureLen = readInt();

	
	for (int i = 0; i < textureLen; ++i) {
		std::string t = readString();
		textures.push_back(t);
	}
	
	// 加载每个骨骼的动画
	for (int i = 0; i < buildHeader.boneCount; ++i) {
		BoneInfo* b = new BoneInfo();
		b->id = readUInt();
		b->frameCount = readInt();
		for (int j = 0; j < b->frameCount; ++j) {
			BoneFrameInfo* f = new BoneFrameInfo();
			f->index = readInt();
			f->duration = readInt();
			f->x = readFloat();
			f->y = readFloat();
			f->width = readFloat();
			f->height = readFloat();
			f->vStartIndex = readInt();
			f->vCount = readInt();
			b->infos.push_back(f);
		}
		bones.push_back(b);
	}

	// 顶点信息
	int ncount = readInt();
	for (int i = 0; i < ncount; ++i) {
		Vertex* ni = new Vertex();
		ni->x = readFloat();
		ni->y = readFloat();
		ni->z = readFloat();
		ni->u = readFloat();
		ni->v = readFloat();
		ni->w = readFloat();
		//log.Write(COLOR_WHITE, "vertex info %f, %f, %f, %f, %f, %f \n", ni->x, ni->y, ni->z, ni->u, ni->v, ni->w);
		vertexes.push_back(ni);
	}

	// 骨骼ID对应骨骼名称
	int count = readInt();
	for (int i = 0; i < count; ++i) {
		int id = readInt();
		std::string name = readString();
		boneNames[id] = name;
	}

	// 打印骨骼动画
	BoneList::iterator i = bones.begin();
	for (; i != bones.end(); ++i) {
		BoneInfo* b = *i;
		log.Write(COLOR_WHITE, "部位名称 %s 帧数 %d\n", boneNames[b->id].c_str(), b->frameCount);
		BoneInfo::BoneFrameList::iterator j = b->infos.begin();
		for (; j != b->infos.end(); ++j) {
			BoneFrameInfo* f = *j;
			log.Write(COLOR_WHITE, "frameID %d, duration %d, x %f, y %f, 开始帧数 %d, 持续帧数 %d, w %f, h %f \n", f->index, f->duration, f->x, f->y, f->vStartIndex, f->vCount, f->width, f->height);
		}
	}

	delete[] buffer;

	log.Write(COLOR_WHITE, "部位数量 %d 总部位状态数 %d 总顶点数 %d", buildHeader.boneCount, buildHeader.frameCount, ncount);

	
	/* load an image file directly as a new OpenGL texture */
	texture[0] = SOIL_load_OGL_texture
		(
		"assets/beefalo/beefalo.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

	if(texture[0] == 0)
		return false;


	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	log.Write(COLOR_WHITE, "结构解析完成");
	log.Write(COLOR_WHITE, "-------------------------------------------------");

	return true;
}

bool Animator::loadAnimFile(const char* fileName) {
	log.Write(COLOR_WHITE, "开始解析Anim.bin文件");

	int size = 0;
	FILE * f;
	
	if(! (f = fopen(fileName, "rb")))
	{
		log.Write(COLOR_WHITE, "Could not open build file %s", fileName);
		return false;
	}

	//check file size and read it all into the buffer
	int iStart = ftell(f);
	fseek(f, 0, SEEK_END);
	int iEnd = ftell(f);
	fseek(f, 0, SEEK_SET);
	size = iEnd - iStart;

	//Allocate memory for whole file
	buffer = new unsigned char[size];
	rptr = buffer;

	if(! buffer)
	{
		log.Write(COLOR_WHITE, "Could not allocate memory for %s", fileName);
		return false;
	}

	//Load file into buffer
	if(fread(buffer, 1, size, f) != (unsigned)size)
	{
		log.Write(COLOR_WHITE, "Could not read from %s", fileName);
		delete [] buffer;
		return false;
	}

	//close the file, we don't need it anymore
	fclose(f);

	// 开始读取
	if (! readAnimHeader()) {
		log.Write(COLOR_WHITE, "%s is not a valid bin file", fileName);
		delete [] buffer;
		return false;
	}

	// 动画总数
	int animationCount = readInt();
	int i = 0;
	log.Write(COLOR_WHITE, "动画总数 %d", animationCount);
	for (; i < animationCount; ++i) {
		Animation* a = new Animation();
		a->name = readString();
		a->facing = readByte();
		a->boneID = readInt();
		a->frameRate = readFloat();
		a->frameCount = readInt();
		//log.Write(COLOR_WHITE, "动画 %s，帧数 %d，帧率 %f，朝向 %d", a->name.c_str(), a->frameCount, a->frameRate, a->facing);
		for (int j = 0; j < a->frameCount; ++j) {
			AnimationFrame* f = new AnimationFrame();
			//log.Write(COLOR_WHITE, "帧数 %d", j);
			f->x = readFloat();
			f->y = readFloat();
			f->w = readFloat();
			f->h = readFloat();
			f->eventCount = readInt();
			for (int eventIndex = 0; eventIndex < f->eventCount; ++eventIndex) {
				int eventID = readInt();
			}
			f->elementCount = readInt();
			for (int elementIndex = 0; elementIndex < f->elementCount; ++elementIndex) {
				AnimationFrameElement* e = new AnimationFrameElement();
				e->boneID = readInt();
				e->frameID = readInt();
				e->layerID = readInt();
				e->a = readFloat();
				e->b = readFloat();
				e->c = readFloat();
				e->d = readFloat();
				e->tx = readFloat();
				e->ty = readFloat();
				e->tz = readFloat();
				//if (boneNames.find(e->boneID) != boneNames.end()) {
					//log.Write(COLOR_WHITE, "帧元素 部位ID %s 帧ID %d x %f y %f z %f a %f b %f c %f d %f", boneNames[e->boneID].c_str(), e->frameID, e->tx, e->ty, e->tz, e->a, e->b, e->c, e->d);
				f->elements.push_back(e);
				//}
			}

			a->frames.push_back(f);
		}
		animations.push_back(a);
	}

	// layer hash to name
	int layerCount = readInt();
	for (int i = 0; i < layerCount; ++i) {
		int id = readInt();
		std::string name = readString();
		layerNames[id] = name;
	}

	
	// 打印骨骼动画
	for (AnimationList::iterator i = animations.begin(); i != animations.end(); ++i) {
		Animation* a = *i;
		//if (a->name == "walk_loop") {
		log.Write(COLOR_WHITE, "动画 %s，帧数 %d，帧率 %f，朝向 %d", a->name.c_str(), a->frameCount, a->frameRate, a->facing);
		//}
		int count = 0;
		for (std::vector<AnimationFrame*>::iterator j = a->frames.begin(); j != a->frames.end(); ++j) {
			AnimationFrame* f = *j;
			//if (a->name == "walk_loop") {
			//log.Write(COLOR_WHITE, "帧数 %d", count++);
			//}
			for (AnimationFrame::ElementList::iterator k = f->elements.begin(); k != f->elements.end(); ++k) {
				AnimationFrameElement* e = *k;
				if (boneNames.find(e->boneID) != boneNames.end()) {
					//if (a->name == "walk_loop") {
					//log.Write(COLOR_WHITE, "帧元素 部位ID %s 帧ID %d 图层 %s x %f y %f z %f a %f b %f c %f d %f", boneNames[e->boneID].c_str(), e->frameID, layerNames[e->layerID].c_str(), e->tx, e->ty, e->tz, e->a, e->b, e->c, e->d);
					//}
				} else {
					//log.Write(COLOR_WHITE, "帧元素 帧ID %d 图层 %s x %f y %f z %f a %f b %f c %f d %f",e->frameID, layerNames[e->layerID].c_str(), e->tx, e->ty, e->tz, e->a, e->b, e->c, e->d);
				}
			}
		}
	}


	log.Write(COLOR_WHITE, "动画解析完成");
}


bool Animator::play(const char* name) {
	AnimationList::const_iterator i = animations.begin();
	for (; i != animations.end(); ++i) {
		Animation* a = *i;
		if (a->name == name) {
			currentAnimation = a;
			return true;
		}
	}

	return false;
}


void Animator::update() {
	if (! currentAnimation) {
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLoadIdentity();
	glTranslatef(320.0f, 300.0, 0.0f);
	glScalef(1.0f / 3.0f, 1.0f / 3.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	currentFrame = currentFrame % currentAnimation->frameCount;
	AnimationFrame* f = currentAnimation->frames[currentFrame];
	AnimationFrame::ElementList::reverse_iterator i = f->elements.rbegin();
	int test = 0;
	for (; i != f->elements.rend(); ++i) {
		AnimationFrameElement* e = *i;
		
		BoneInfo* b = getBone(e->boneID);
		if (b) {
			BoneFrameInfo* f = b->infos[e->frameID];

			// 遍历顶点范围
			glPushMatrix();
			// tx ty
			glTranslatef(e->tx, e->ty, 0);

			// sx sy
			float sx = sqrtf(e->a * e->a + e->c * e->c);
			float sy = sqrtf(e->b * e->b + e->d * e->d);
			
			// http://www.infogroupindia.com/blog/posts/589/action-script-3-2d-matrix-scale-and-rotation-extraction
			// 阿三写的居然对了
			// 不过完全没看懂
			// 通过tan得到的角度，-pi / 2 ~ pi / 2
			float sign = atanf(-e->c / e->a);
			// 通过cos得到的角度，0 ~ pi
			float rad = acosf(e->a / sx);


			// 通过cos跟tan的值才能完全判断出角度
			float deg = rad * 180.0f / M_PI;
			
			// 修正错误的角度值
			if (deg > 90 && sign > 0) {
				// 应该在第三象限
				deg = 360 - deg;
			} else if (deg < 90 && sign < 0) {
				// 应该在第四象限
				deg = 360 - deg;
			}

			// 误差值 0.001
			// 蜘蛛女王还是有些问题，很奇怪
			if (((deg > 270 && deg < 360) || (deg > 0 && deg < 90)) && e->a < -0.001 ||
				deg > 90 && deg < 270 && e->a > 0.001) {
				sx *= -1.0f;
			}
			if (((deg > 270 && deg < 360) || (deg > 0 && deg < 90)) && e->d < -0.001 ||
				deg > 90 && deg < 270 && e->d > 0.001) {
				sy *= -1.0f;
				//log.Write(COLOR_WHITE, "渲染到上下翻转帧元素 部位ID %s 帧ID %d x %f y %f z %f a %f b %f c %f d %f", boneNames[e->boneID].c_str(), e->frameID, e->tx, e->ty, e->tz, e->a, e->b, e->c, e->d);
			}
			glScalef(sx, sy, 1.0);

			glRotatef(deg, 0.0f, 0.0f, 1.0f);
			

			int nowIndex = f->vStartIndex;
			int endIndex = f->vStartIndex + f->vCount;
			glBegin(GL_POLYGON);
			for (; nowIndex < endIndex; ++nowIndex) {
				Vertex* v1 = vertexes[nowIndex];
				glTexCoord2f(v1->u, v1->v); glVertex3f(v1->x, v1->y, v1->z);
			}
			glEnd();
			glPopMatrix();
		}
	}
	//currentFrame++;
	//log.Write(COLOR_WHITE, "当前帧数 %d %d", currentFrame, currentAnimation->frameCount);
}

BoneInfo* Animator::getBone(int id) {
	BoneList::const_iterator i = bones.begin();
	for(; i != bones.end(); ++i) {
		BoneInfo* b = *i;
		if (b->id == id) {
			return b;
		}
	}
	return NULL;
}

void Animator::render() {
	glLoadIdentity();		
	glTranslatef(320.0f,240.0f,0.0f);
	glScalef(1.0f / 3.0f, 1.0f / 3.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	// 渲染第一个骨骼的第一帧
	BoneList::iterator iterator = bones.begin();
	BoneInfo* b;
	for (; iterator != bones.end(); ++iterator) {
		b = *iterator;
		if (boneNames[b->id] == "hair") {
			break;
		}
	}
	BoneFrameInfo* f = b->infos.front();

	int i = f->vStartIndex;
	int endIndex = f->vStartIndex + f->vCount;

	glBegin(GL_POLYGON);
	for (; i < endIndex; ++i) {
		Vertex* v1 = vertexes[i];
		glTexCoord2f(v1->u, v1->v); glVertex3f(v1->x, v1->y, v1->z);
	}
	glEnd();
}

void Animator::printf(float x, float y, const char * fmt, ...) {
	char final[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf(final, fmt, va);
	va_end(va);
	print(x, y, final);
}

void Animator::print(float x, float y, const char* string, float r, float g, float b) {
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

unsigned int Animator::strHash(std::string val)
{
    unsigned int hash = 0;
    for(unsigned int i = 0; i < val.length(); i++)
	{
		char c = tolower(val[i]);
		hash = (c + (hash << 6) + (hash << 16) - hash) & 0xFFFFFFFFL;
	}
	return hash;
}

bool Animator::readBuildHeader() {
	memcpy(&buildHeader, rptr, sizeof(BuildHeader));
	rptr += sizeof(BuildHeader);

	if (buildHeader.magicNumber != 0x444C4942 && buildHeader.version != 6) {
		return false;
	}

	return true;
}

bool Animator::readAnimHeader() {
	memcpy(&animHeader, rptr, sizeof(AnimHeader));
	rptr += sizeof(AnimHeader);

	if (animHeader.magicNumber != 0x4D494E41 && animHeader.version != 4) {
		return false;
	}

	return true;
}

char Animator::readByte() {
	char c;
	memcpy(&c, rptr, 1);
	rptr += 1;

	return c;
}
int Animator::readInt() {
	int i;
	memcpy(&i, rptr, 4);
	rptr += 4;
	return i;
}
unsigned int Animator::readUInt() {
	unsigned int i;
	memcpy(&i, rptr, 4);
	rptr += 4;
	return i;
}
float Animator::readFloat() {
	float i;
	memcpy(&i, rptr, 4);
	rptr += 4;
	return i;
}
char* Animator::readString() {
	int len;
	memcpy(&len, rptr, 4);
	rptr += 4;
	char* str = new char[len + 1];
	memcpy(str, rptr, len);
	str[len] = 0;
	rptr += len;
	return str;
}