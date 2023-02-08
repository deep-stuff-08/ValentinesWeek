#include<iostream>
#include<string>
#include<vector>

#include<GL/glew.h>

#include"../include/glwindowing.h"
#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/glmodelloader.h"

using namespace std;
using namespace vmath;

#define len(x) sizeof(x) / sizeof(x[0])

#define SHADOW_WIDTH_HEIGHT 4096

glprogram_dl simpleDepthProgram;
glprogram_dl shadowMapProgram;
glprogram_dl fsquadProgram;
glmodel_dl boy;
glmodel_dl girl;

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
	vec3(0.0f, 1.0f, 5.0f),
	vec3(cos(radians(-90.0f)) * cos(radians(-30.0f)), sin(radians(-30.0f)), sin(radians(-90.0f)) * cos(radians(-30.0f))),
	vec3(0.0f, 1.0f, 0.0f),
	-90.0f,
	-30.0f
};

GLuint vaoPlane;
GLuint vboPlane;
GLuint fboDepth;
GLuint texDepth;
GLuint texFloor;
GLuint fboScene;
GLuint rboScene;
GLuint texScene;
GLuint texProposeDay;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

void createProgram() {
	glshader_dl vertexShader, fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/simpleDepth.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/simpleDepth.frag");

	cout<<glprogramCreate(&simpleDepthProgram, "Simple Depth", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/shadowMap.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/shadowMap.frag");

	cout<<glprogramCreate(&shadowMapProgram, "Shadow Map", { vertexShader, fragmentShader });

	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/fsquad.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/fsquad.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&fsquadProgram, "FSQuad", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);
}

void init(void) {
	createProgram();

	float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

	glGenVertexArrays(1, &vaoPlane);
	glBindVertexArray(vaoPlane);
	glGenBuffers(1, &vboPlane);
	glBindBuffer(GL_ARRAY_BUFFER, vboPlane);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	cout<<createTexture2D(&texFloor, "../textures/floor.jpg");
	cout<<createTexture2D(&texProposeDay, "../textures/proposeday.jpg");

	glGenTextures(1, &texDepth);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT);
	
	glGenFramebuffers(1, &fboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, fboDepth);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenTextures(1, &texScene);
	glBindTexture(GL_TEXTURE_2D, texScene);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT);

	glGenRenderbuffers(1, &rboScene);
	glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texScene, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	cout<<createModel(&boy, "../models/proposeboy/boy.obj", 0, 0, 1, 2);
	cout<<createModel(&girl, "../models/proposegirl/girl.obj", 0, 0, 1, 2);

	glEnable(GL_DEPTH_TEST);
	glPolygonOffset(4.0f, 4.0f);

	currentScene = 0;
}

void renderScene() {
	glUniformMatrix4fv(2, 1, GL_FALSE, mat4::identity());
	glBindVertexArray(vaoPlane);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.45f, -0.5f, 0.0f) * rotate(270.0f, 0.0f, 1.0f, 0.0f));
	drawModel(&boy, -1, 1);
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(-0.45f, -0.5f, 0.0f) * rotate(90.0f, 0.0f, 1.0f, 0.0f));
	drawModel(&girl, -1, 1);
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float fade = 1.0f;
	static float theta = M_PI;
	vec3 lightPos = vec3(2.0f * sin(theta), 2.0f * cos(theta), -3.0f);

	mat4 scaleBiasMatrix = mat4(
		vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f)
	);

	mat4 lightSpaceMatrix = ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f) * lookat(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

	glUseProgram(simpleDepthProgram.programObject);
	glBindFramebuffer(GL_FRAMEBUFFER, fboDepth);
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT);
	glUseProgram(simpleDepthProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, lightSpaceMatrix);
	glEnable(GL_POLYGON_OFFSET_FILL);
	renderScene();

	glDisable(GL_POLYGON_OFFSET_FILL);
	glUseProgram(shadowMapProgram.programObject);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, 4096, 4096);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	glUniformMatrix4fv(3, 1, GL_FALSE, scaleBiasMatrix * lightSpaceMatrix);
	glUniform3fv(4, 1, lightPos);
	glUniform3fv(5, 1, glcamera.position);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFloor);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texDepth);
	renderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, winSize.w, winSize.h);
	glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
	glDisable(GL_DEPTH_TEST);
	glUseProgram(fsquadProgram.programObject);
	if(currentScene == 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texScene);
	} else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texProposeDay);
	}
	glUniform1f(0, fade);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);

	theta += gltime.delta * 0.1f;

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
	// setMouseFunc(mouse);
	createOpenGLWindow();
	toggleFullscreen();
	processEvents();

	init();
	while(!isOpenGLWindowClosed()) {
		processEvents();
		render();
		swapBuffers();
	}
	uninit();
	destroyOpenGLWindow();
}
