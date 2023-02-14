#include"../include/glwindowing.h"
#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/glmodelloader.h"
#include"../include/vmath.h"
#include"../include/shapes.h"
#include<iostream>
#include<assimp/postprocess.h>

using namespace std;
using namespace vmath;

typedef struct particle_t {
	vec3 position;
	vec3 velocity;
	float gravity;
	float lifeLenght;
	float rotate;
	float scale;
	float elapsedTime;
} particle_t;

glmodel_dl model;
glprogram_dl modelProgram;
glprogram_dl fsquadProgram;
glprogram_dl skyBoxProgram;
glprogram_dl progRender;
shapes_dl sphereMap;

struct GLTimer {
	double delta;
	double last;	
} gltime = {0.0f, 0.0f};

struct GLCamera {
	vec3 position;
	vec3 front;
	vec3 up;
} glcamera = {
	vec3(0.0f, 0.0f, 1.0f),
	vec3(0.0f, 0.0f, -1.0f),
	vec3(0.0f, 1.0f, 0.0f)
};

vector<particle_t> particleList;

GLuint vaoParticle;
GLuint fboScene;
GLuint fboPing;
GLuint fboPong;
GLuint texSceneColor;
GLuint texSceneBright;
GLuint texPing;
GLuint texPong;
GLuint texForest;
GLuint texKissDay;
GLuint texImage;
GLuint rboScene;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

bool updateParticle(particle_t &p, float deltaTime) {
	p.velocity += 10.0f * vec3(0.0f, -p.gravity, 0.0f) * deltaTime;
	p.position += p.velocity * deltaTime;
	p.elapsedTime += deltaTime;
	return p.elapsedTime < p.lifeLenght;
}

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

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/skybox.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/skybox.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&skyBoxProgram, "Skybox", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/render.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/render.frag");

	cout<<glprogramCreate(&progRender, "Render", {vertexShader, fragmentShader});

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);
}

void init(void) {
	setupProgram();

	glGenRenderbuffers(1, &rboScene);
	glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 1024, 1024);

	glGenTextures(1, &texSceneColor);
	glBindTexture(GL_TEXTURE_2D, texSceneColor);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1024, 1024);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texSceneColor, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	GLenum db[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(2, db);

		vec2 quadVertices[] = {
		vec2(1.0f, 1.0f),
		vec2(-1.0f, 1.0f),
		vec2(1.0f, -1.0f),
		vec2(-1.0f, -1.0f)
	};

	GLuint vbo;
	glGenVertexArrays(1, &vaoParticle);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vaoParticle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	cout<<createModel(&model, "../models/wand/wand.obj", 0, 0, 1, 2);

	cout<<createTexture2D(&texForest, "../textures/grass.jpg");
	glBindTexture(GL_TEXTURE_2D, texForest);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	cout<<createTexture2D(&texKissDay, "../textures/kissday.jpg");

	cout<<createTexture2D(&texImage, "../textures/kiss.png");

	initUnitSphere(&sphereMap, 15, 30, 0, 1, 2);

	glEnable(GL_DEPTH_TEST);
}

void generateParticle(float pps, vec3 position, float scale, float life, float gravity, float speed, float delta) {
	float particlesToCreate = pps * delta;
	int count = (int)particlesToCreate;
	float partialParticle = particlesToCreate - count;
	if ((float)rand() / RAND_MAX < partialParticle) {
		count++;
	}
	for (int i = 0; i < count; i++) {
		float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
		float z = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		float rootOneMinusZSquared = (float)sqrt(1 - z * z);
		float x = rootOneMinusZSquared * cos(theta);
		float y = rootOneMinusZSquared * sin(theta);
		vec3 velocity = normalize(vec3(x, y, z)) * (speed + ((float)rand() / RAND_MAX * 2.0f - 1.0f));
		float scaler = scale + ((float)rand() / RAND_MAX * 0.2f);
		float rotater = ((float)rand() / RAND_MAX * 90.0f - 45.0f);
		float lifeLength = life + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 2.0f;
		particleList.push_back({position, velocity, gravity, lifeLength, rotater, scaler, 0});
	}
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float transY = 4.3f;
	static float pps = 0.0f;
	static float fade = 1.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, 1024, 1024);

	glUseProgram(skyBoxProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(vec3(0.0f, 0.0f, 0.0f), glcamera.front, glcamera.up));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, transY, 0.0f) * rotate((float)getTime() * 10.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(10.0f));	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texForest);
	renderShape(&sphereMap);

	glUseProgram(modelProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, transY, -5.0f) * rotate(90.0f, 0.0f, 0.0f, 1.0f) * scale(0.5f));	
	drawModel(&model, 4);
	
	if(transY < 0.3f) {
		generateParticle(pps, vec3(0.0f, 2.4f + transY, -5.0f), 0.03f, 3.0f, 0.06f, 3.0f, gltime.delta);
		pps += 0.07f;
		pps = std::min(pps, 50.0f);
	}

	glUseProgram(progRender.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	vector<particle_t>::iterator it = particleList.begin();
	while(it != particleList.end()) {
		glUniformMatrix4fv(2, 1, GL_FALSE, translate(it->position) * rotate(it->rotate, vec3(0.0f, 0.0f, 1.0f)) * scale(it->scale));
		bool isAlive = updateParticle(*it, gltime.delta);
		if(!isAlive) {
			it = particleList.erase(it);
			continue;
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texImage);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(vaoParticle);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		it++;
	}

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
		glBindTexture(GL_TEXTURE_2D, texKissDay);
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	transY -= (float)gltime.delta * 0.4f;
	transY = std::max(transY, -0.7f);

	if(startFade) {
		fade += (float)gltime.delta * 0.2f * signFade;
		if(fade > 1.0f || fade < 0.0f) {
			fade = (fade > 0.5) ? 1.0f : 0.0f;
			startFade = false;
		}
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

	static float pitch;
	static float yaw = -90.0f;

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

	yaw   += xoffset;
	pitch += yoffset;

	if(pitch > 89.0f) {
		pitch = 89.0f;
	} else if(pitch < -89.0f) {
		pitch = -89.0f;
	}
	vec3 direction = vec3(cos(radians(yaw)) * cos(radians(pitch)), sin(radians(pitch)), sin(radians(yaw)) * cos(radians(pitch)));
	glcamera.front = normalize(direction);
}

int main(void) {
	setKeyboardFunc(keyboard);
	// setMouseFunc(mouse);
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
