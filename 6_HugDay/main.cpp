#include"../include/glwindowing.h"
#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/glmodelloader.h"
#include"../include/vmath.h"
#include"../include/shapes.h"
#include<iostream>
#include<assimp/postprocess.h>

#define FBO_SIZE 1024

using namespace std;
using namespace vmath;

struct GLTimer {
	double delta;
	double last;	
} gltime = {0.0f, 0.0f};

struct GLCamera {
	vec3 position;
	vec3 front;
	vec3 up;
} glcamera = {
	vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, -1.0f),
	vec3(0.0f, 1.0f, 0.0f)
};

glprogram_dl programColor;
glprogram_dl programGodrays;
glprogram_dl programTexture;
shapes_dl sun;

GLuint texHug;
GLuint fboScene;
GLuint texScene;
GLuint rboScene;
GLuint texHugDay;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

void setupProgram() {
	glshader_dl vertexShader, fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/color.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/color.frag");

	cout<<glprogramCreate(&programColor, "Color", {vertexShader, fragmentShader});

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/godrays.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/godrays.frag");

	cout<<glprogramCreate(&programGodrays, "Godrays", {vertexShader, fragmentShader});

	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/texrender.frag");

	cout<<glprogramCreate(&programTexture, "Texture", {vertexShader, fragmentShader});

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);
}

void init(void) {
	setupProgram();

	cout<<createTexture2D(&texHug, "../textures/hug.png");
	cout<<createTexture2D(&texHugDay, "../textures/hugday.jpg");

	initUnitSphere(&sun, 25, 25, 0);

	glGenTextures(1, &texScene);
	glBindTexture(GL_TEXTURE_2D, texScene);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, FBO_SIZE, FBO_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &rboScene);
	glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, FBO_SIZE, FBO_SIZE);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texScene, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float transy = -1.5f;

	static float fade = 1.0f;
	
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);

	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));

	glViewport(0, 0, FBO_SIZE, FBO_SIZE);

	glUseProgram(programColor.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, ortho(-1.0f * winSize.w / winSize.h, 1.0f * winSize.w / winSize.h, -1.0f, 1.0f, -1.0f, 1.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, mat4::identity());
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, transy, 0.0f) * scale(0.4f));
	glUniform4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	renderShape(&sun);
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(programTexture.programObject);
	glUniform1i(0, GL_TRUE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texHug);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearBufferfv(GL_COLOR, 0, vec4(0.2f, 0.2f, 0.2f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));

	glViewport(0, 0, winSize.w, winSize.h);

	glUseProgram(programColor.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, 0.0f, 0.0f) * scale(0.4f));
	glUniform4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	renderShape(&sun);
	glEnable(GL_DEPTH_TEST);
	
	
	if(currentScene == 0) {
		glUseProgram(programGodrays.programObject);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texScene);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texHug);
		glUniform1f(0, 1.1f);
		glUniform1f(1, 0.06f);
		glUniform1f(2, 0.98f);
		glUniform1f(3, 0.5f);
		glUniform1i(4, 100);
		glUniform2fv(5, 1, vec2(0.5f, transy * 0.5f + 0.5f));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	} else if(currentScene == 1) {
		glUseProgram(programTexture.programObject);
		glUniform1i(0, GL_FALSE);
		glUniform1f(1, fade);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texHugDay);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	
	transy += gltime.delta * 0.1f;

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
