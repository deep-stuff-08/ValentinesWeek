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

glmodel_dl model;
glprogram_dl modelProgram;
glprogram_dl fsquadProgram;
glprogram_dl blurProgram;
glprogram_dl skyBoxProgram;
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

GLuint fboScene;
GLuint fboPing;
GLuint fboPong;
GLuint texSceneColor;
GLuint texSceneBright;
GLuint texPing;
GLuint texPong;
GLuint texForest;
GLuint texRoseDay;
GLuint rboScene;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

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

	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/blur.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&blurProgram, "Blur", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/skybox.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/skybox.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&skyBoxProgram, "Skybox", { vertexShader, fragmentShader });

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

	glGenTextures(1, &texSceneBright);
	glBindTexture(GL_TEXTURE_2D, texSceneBright);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, 1024, 1024);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texSceneColor, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texSceneBright, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	GLenum db[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, db);

	glGenTextures(1, &texPing);
	glBindTexture(GL_TEXTURE_2D, texPing);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glGenFramebuffers(1, &fboPing);
	glBindFramebuffer(GL_FRAMEBUFFER, fboPing);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texPing, 0);

	glGenTextures(1, &texPong);
	glBindTexture(GL_TEXTURE_2D, texPong);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glGenFramebuffers(1, &fboPong);
	glBindFramebuffer(GL_FRAMEBUFFER, fboPong);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texPong, 0);

	cout<<createModel(&model, "../models/rose/rose.obj", aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace, 0, 1, 2);

	cout<<createTexture2D(&texForest, "../textures/hall.png");
	glBindTexture(GL_TEXTURE_2D, texForest);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	cout<<createTexture2D(&texRoseDay, "../textures/roseday.jpg");

	initUnitSphere(&sphereMap, 15, 30, 0, 1, 2);

	glEnable(GL_DEPTH_TEST);
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float transY = 0.8f;
	static float transZ = -0.3f;
	static float fade = 1.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_COLOR, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
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
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, transY, transZ) * rotate((float)getTime() * 10.0f, vec3(0.0f, 1.0f, 0.0f)));	
	drawModel(&model, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, fboPing);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texSceneBright);
	glUseProgram(blurProgram.programObject);
	glUniform1i(0, GL_TRUE);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, fboPong);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texPing);
	glUseProgram(blurProgram.programObject);
	glUniform1i(0, GL_FALSE);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fboPing);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texPong);
	glUseProgram(blurProgram.programObject);
	glUniform1i(0, GL_TRUE);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, fboPong);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texPing);
	glUseProgram(blurProgram.programObject);
	glUniform1i(0, GL_FALSE);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
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
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texPong);
	} else if(currentScene == 1) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texRoseDay);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);	
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	transY -= (float)gltime.delta * 0.05f;
	transY = std::max(transY, -0.7f);

	transZ -= (float)gltime.delta * 0.03f;
	transZ = std::max(transZ, -0.6f);

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
