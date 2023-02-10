#include"../include/glwindowing.h"
#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/glmodelloader.h"
#include"../include/vmath.h"
#include"../include/shapes.h"
#include<iostream>
#include<fstream>
#include<sstream>

using namespace std;
using namespace vmath;

glmodel_dl teddyeyes;
glprogram_dl modelProgram;
glprogram_dl fsquadProgram;
glprogram_dl furProgram;

struct GLTimer {
	double delta;
	double last;	
} gltime = {0.0f, 0.0f};

struct GLCamera {
	vec3 position;
	vec3 front;
	vec3 up;
	GLfloat yaw;
	GLfloat pitch;
} glcamera = {
	vec3(0.0f, 3.0f, 7.0f),
	vec3(cos(radians(-90.0f)) * cos(radians(-25.0f)), sin(radians(-25.0f)), sin(radians(-90.0f)) * cos(radians(-25.0f))),
	vec3(0.0f, 1.0f, 0.0f),
	-90.0f,
	-25.0f
};

GLuint fboScene;
GLuint texSceneColor;
GLuint texChocolateDay;
GLuint rboScene;

GLuint texFur;
GLuint texGrass;
GLuint texColor;
GLuint texColorG;
GLuint vaoTeddyBody;
GLuint vaoFloor;
int countTeddyBody;
int countFloor;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

bool startMix = false;

typedef struct furvertexdata_t {
	vec3 position;
	vec3 normal;
	vec2 texcoord;
	float layer;
} furvertexdata_t;

void setupProgram() {
	glshader_dl vertexShader, fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/draw.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/draw.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&modelProgram, "Draw", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/fsquad.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/fsquad.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&fsquadProgram, "FSQuad", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/fur.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/fur.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&furProgram, "Fur", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);
}

#define LAYER1 11
#define LAYER2 40

void initModelVerticesForFur(string path, vector<furvertexdata_t> &vertices) {
	vector<vec3> vertexPosition;
	vector<vec3> vertexNormal;
	vector<vec2> vertexTexCoord;
	vector<uvec3> facePosition;
	vector<uvec3> faceNormal;
	vector<uvec3> faceTexCoord;
	ifstream modelFile(path);
	int faceCount = 0;
	while(!modelFile.eof()) {
		string line;
		getline(modelFile, line);
		stringstream ssline(line);
		string token;
		ssline>>token;
		if(token.compare("v") == 0) {
			float x, y, z;
			ssline>>x>>y>>z;
			vertexPosition.push_back(vec3(x, y, z));
		} else if(token.compare("vn") == 0) {
			float x, y, z;
			ssline>>x>>y>>z;
			vertexNormal.push_back(vec3(x, y, z));
		} else if(token.compare("vt") == 0) {
			float x, y;
			ssline>>x>>y;
			vertexTexCoord.push_back(vec2(x, y));
		} else if(token.compare("f") == 0) {
			char c;
			uvec3 vp, vn, vt;
			for(int i = 0; i < 3; i++) {
				ssline>>vp[i];
				ssline.get();
				ssline>>vt[i];
				ssline.get();
				ssline>>vn[i];
			}
			facePosition.push_back(vp);
			faceTexCoord.push_back(vt);
			faceNormal.push_back(vn);
			faceCount++;
		}
	}

	for(int i = 0; i < faceCount; i++) {
		for(int j = 0; j < 3; j++) {
			furvertexdata_t fur;
			fur.position = vertexPosition[facePosition[i][j] - 1];
			fur.normal = vertexNormal[faceNormal[i][j] - 1];
			fur.texcoord = vertexTexCoord[faceTexCoord[i][j] - 1];
			fur.layer = 0;
			vertices.push_back(fur);
		}
	}
}

void createFurTexture(GLuint *tex, int w, int h, float density, int layer) {
	vector<ubvec4> texArray;
	for(int i = 0; i < w * h; i++) {
		texArray.push_back(ubvec4(0, 0, 0, 0));
	}

	int numStrands = (int)(density * w * h);
	int strandsPerLayer = numStrands / layer;

	for(int i = 0; i < numStrands; i++) {
		int x = rand() % h;
		int y = rand() % w;

		float maxLayer = pow((float)(i / strandsPerLayer) / (float)layer, 0.9f);
		texArray[x * w + y] = ubvec4((unsigned char)(maxLayer * 255), 0, 0, 255);
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texArray.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void init(void) {
	setupProgram();

	vector<furvertexdata_t> verticesQuad;
	verticesQuad.push_back({{ 20.0, -20.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, 0.0});
	verticesQuad.push_back({{ 30.0,  20.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}, 0.0});
	verticesQuad.push_back({{-30.0,  20.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}, 0.0});
	verticesQuad.push_back({{-30.0,  20.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}, 0.0});
	verticesQuad.push_back({{-20.0, -20.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}, 0.0});
	verticesQuad.push_back({{ 20.0, -20.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}, 0.0});

	vector<furvertexdata_t> furverticesQuad;
	for(int i = 0; i < LAYER2; i++) {
		float layer = (float)i / (float)(LAYER2 - 1);
		float layerHairLength = 2.0f * layer;
		for(furvertexdata_t f : verticesQuad) {
			f.position = f.position + f.normal * layerHairLength;
			f.layer = layer;
			furverticesQuad.push_back(f);
		}
	}

	GLuint vbo;
	glGenVertexArrays(1, &vaoFloor);
	glBindVertexArray(vaoFloor);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(furvertexdata_t) * furverticesQuad.size(), furverticesQuad.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, texcoord));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, layer));
	glEnableVertexAttribArray(3);
	countFloor = furverticesQuad.size();

	vector<furvertexdata_t> vertices;
	initModelVerticesForFur("../models/teddy/teddybodyc.obj", vertices);

	vector<furvertexdata_t> furvertices;
	for(int i = 0; i < LAYER1; i++) {
		float layer = (float)i / (float)(LAYER1 - 1);
		float layerHairLength = 0.1f * layer;
		for(furvertexdata_t f : vertices) {
			f.position = f.position + f.normal * layerHairLength;
			f.layer = layer;
			furvertices.push_back(f);
		}
	}

	GLuint vbo2;
	glGenVertexArrays(1, &vaoTeddyBody);
	glBindVertexArray(vaoTeddyBody);
	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(furvertexdata_t) * furvertices.size(), furvertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, texcoord));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(furvertexdata_t), (void*)offsetof(furvertexdata_t, layer));
	glEnableVertexAttribArray(3);
	countTeddyBody = furvertices.size();

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenRenderbuffers(1, &rboScene);
	glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 1024, 1024);

	glGenTextures(1, &texSceneColor);
	glBindTexture(GL_TEXTURE_2D, texSceneColor);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1024, 1024);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texSceneColor, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	cout<<createModel(&teddyeyes, "../models/teddy/teddyeyes.obj", 0, 0, 1, 2);

	createFurTexture(&texFur, 512, 512, 0.7f, LAYER1);
	createFurTexture(&texGrass, 512, 512, 0.3f, LAYER2);
	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, ubvec4(190, 100, 30, 255));
	cout<<createTexture2D(&texColorG, "../textures/grass.png");
	cout<<createTexture2D(&texChocolateDay, "../textures/teddyday.jpg");

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float fade = 1.0f;
	static float mix = 0.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	
	glClearBufferfv(GL_COLOR, 0, vec4(0.4f, 0.7f, 1.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, 1024, 1024);

	vec3 gravity;
	vec3 force;
	vec3 disp;
	glUseProgram(modelProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, 0.7f, 0.08f) * scale(0.7f));
	glUniform3fv(3, 1, vec3(0.0f, 10.0f, 10.0f));
	glUniform3fv(4, 1, glcamera.position);
	drawModel(&teddyeyes, -1);

	glUseProgram(furProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, -4.0f, 0.0f) * rotate(-90.0f, 1.0f, 0.0f, 0.0f) * scale(0.5f, 0.5f, 0.5f));
	gravity = vec3(0.0f, -0.8f, 0.0f);
	force = vec3(sin(getTime()) * 0.5, 0.0f, 0.0f);
	disp = gravity + force;
	glUniform3fv(5, 1, disp);
	glUniform3fv(3, 1, vec3(0.0f, 10.0f, 3.0f));
	glUniform3fv(4, 1, glcamera.position);
	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texGrass);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texColorG);
	glBindVertexArray(vaoFloor);
	glDrawArrays(GL_TRIANGLES, 0, countFloor);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, 0.7f, 0.0f) * scale(0.7f));
	gravity = vec3(0.0f, -0.008f, 0.0f);
	force = vec3(sin(getTime()) * 0.05, 0.0f, 0.0f);
	disp = gravity + force;
	glUniform3fv(5, 1, disp);
	glUniform3fv(3, 1, vec3(0.0f, 10.0f, 3.0f));
	glUniform3fv(4, 1, glcamera.position);
	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFur);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glBindVertexArray(vaoTeddyBody);
	glDrawArrays(GL_TRIANGLES, 0, countTeddyBody);
	glBindVertexArray(0);
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.3f, 0.3f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, winSize.w, winSize.h);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(fsquadProgram.programObject);
	glUniform1f(0, fade);
	if(currentScene == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texSceneColor);
	} else if(currentScene == 1) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texChocolateDay);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	if(startFade) {
		fade += (float)gltime.delta * 0.2f * signFade;
		if(fade > 1.0f || fade < 0.0f) {
			fade = (fade > 0.5) ? 1.0f : 0.0f;
			startFade = false;
		}
	}

	if(startMix) {
		glcamera.position += glcamera.front * gltime.delta * 0.3f;
		glcamera.front[1] += gltime.delta * 0.01f;
	}
}

void uninit(void) {

}

void keyboard(unsigned int key, int state) {
	float speed = 7.5f * gltime.delta;
	switch(key) {
	case 'W': case 'w':
		glcamera.position += speed * glcamera.front;
		break;
	case 'S': case 's':
		glcamera.position -= speed * glcamera.front;
		break;
	case 'D': case 'd':
		glcamera.position += speed * normalize(cross(glcamera.front, glcamera.up));
		break;
	case 'A': case 'a':
		glcamera.position -= speed * normalize(cross(glcamera.front, glcamera.up));
		break;
	case 'F': case 'f':
		toggleFullscreen();
		break;
	case 'C': case 'c':
		currentScene += 1;
		break;
	case 'Z': case 'z':
		startMix = !startMix;
		break;
	case ' ':
		startFade = true;
		signFade *= -1;
		break;
	case DL_ESCAPE:
		closeOpenGLWindow();
		break;
	}
}

void mouse(int x, int y) {
	static bool isFirst = true;
	static float lastX;
	static float lastY;

	if(isFirst) {
		lastX = x;
		lastY = y;
		isFirst = false;
	}

	float xoffset = x - lastX;
	float yoffset = lastY - y; 
	lastX = x;
	lastY = y;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	glcamera.yaw   += xoffset;
	glcamera.pitch += yoffset;

	if(glcamera.pitch > 89.0f) {
		glcamera.pitch = 89.0f;
	} else if(glcamera.pitch < -89.0f) {
		glcamera.pitch = -89.0f;
	}
	vec3 direction = vec3(cos(radians(glcamera.yaw)) * cos(radians(glcamera.pitch)), sin(radians(glcamera.pitch)), sin(radians(glcamera.yaw)) * cos(radians(glcamera.pitch)));
	glcamera.front = normalize(direction);
}

int main(void) {
	setKeyboardFunc(keyboard);
	setMouseFunc(mouse);
	createOpenGLWindow();
	init();
	toggleFullscreen();
	while(!isOpenGLWindowClosed()) {
		processEvents();
		render();
		swapBuffers();
	}
	uninit();
	destroyOpenGLWindow();
}
