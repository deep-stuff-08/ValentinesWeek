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
glprogram_dl mapProgram;
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
	GLfloat yaw;
	GLfloat pitch;
} glcamera = {
	vec3(0.0f, 0.5f, 0.7f),
	vec3(cos(radians(-90.0f)) * cos(radians(-45.0f)), sin(radians(-45.0f)), sin(radians(-90.0f)) * cos(radians(-45.0f))),
	vec3(0.0f, 1.0f, 0.0f),
	-90.0f,
	-30.0f
};

GLuint fboScene;
GLuint texSceneColor;
GLuint texChoDiffuse;
GLuint texChoNormal;
GLuint texChoBump;
GLuint texChocolateDay;
GLuint rboScene;
GLuint vaoChocolate;

GLuint currentScene = 0;
int signFade = 1;
bool startFade = false;

bool startMix = false;

void setupProgram() {
	glshader_dl vertexShader, fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/draw.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/draw.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&modelProgram, "Draw", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/skybox.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/skybox.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&skyBoxProgram, "Skybox", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/fsquad.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/fsquad.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&fsquadProgram, "FSQuad", { vertexShader, fragmentShader });

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "shaders/map.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "shaders/map.frag", DL_SHADER_CORE, 460);

	cout<<glprogramCreate(&mapProgram, "Map", { vertexShader, fragmentShader });

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texSceneColor, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboScene);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	cout<<createModel(&model, "../models/table/table.obj", aiProcess_Triangulate | aiProcess_CalcTangentSpace, 0, 1, 2);

	cout<<createTexture2D(&texChoDiffuse, "../textures/chocolate/diffuse.jpg");
	glBindTexture(GL_TEXTURE_2D, texChoDiffuse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cout<<createTexture2D(&texChoNormal, "../textures/chocolate/normal.jpg");
	glBindTexture(GL_TEXTURE_2D, texChoNormal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cout<<createTexture2D(&texChoBump, "../textures/chocolate/bump.jpg");
	glBindTexture(GL_TEXTURE_2D, texChoBump);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	cout<<createTexture2D(&texChocolateDay, "../textures/chocolateday.jpg");

	vec3 pos1(-1.0f, 0.0f,  1.0f);
	vec3 pos2(-1.0f, 0.0f, -1.0f);
	vec3 pos3( 1.0f, 0.0f, -1.0f);
	vec3 pos4( 1.0f, 0.0f,  1.0f);
	vec2 uv1(0.0f, 1.0f);
	vec2 uv2(0.0f, 0.0f);
	vec2 uv3(1.0f, 0.0f);
	vec2 uv4(1.0f, 1.0f);
	vec3 nm(0.0f, 1.0f, 0.0f);
	vec3 tangent1, bitangent1;
	vec3 tangent2, bitangent2;
	vec3 edge1 = pos2 - pos1;
	vec3 edge2 = pos3 - pos1;
	vec2 deltaUV1 = uv2 - uv1;
	vec2 deltaUV2 = uv3 - uv1;

	float f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

	tangent1[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
	tangent1[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
	tangent1[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);
	tangent1 = normalize(tangent1);

	bitangent1[0] = f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]);
	bitangent1[1] = f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]);
	bitangent1[2] = f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2]);
	bitangent1 = normalize(bitangent1);

	edge1 = pos3 - pos1;
	edge2 = pos4 - pos1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1]);

	tangent2[0] = f * (deltaUV2[1] * edge1[0] - deltaUV1[1] * edge2[0]);
	tangent2[1] = f * (deltaUV2[1] * edge1[1] - deltaUV1[1] * edge2[1]);
	tangent2[2] = f * (deltaUV2[1] * edge1[2] - deltaUV1[1] * edge2[2]);
	tangent2 = normalize(tangent2);

	bitangent2[0] = f * (-deltaUV2[0] * edge1[0] + deltaUV1[0] * edge2[0]);
	bitangent2[1] = f * (-deltaUV2[0] * edge1[1] + deltaUV1[0] * edge2[1]);
	bitangent2[2] = f * (-deltaUV2[0] * edge1[2] + deltaUV1[0] * edge2[2]);
	bitangent2 = normalize(bitangent2);

	float quadVertices[] = {
		pos1[0], pos1[1], pos1[2], nm[0], nm[1], nm[2], uv1[0], uv1[1], tangent1[0], tangent1[1], tangent1[2], bitangent1[0], bitangent1[1], bitangent1[2],
		pos2[0], pos2[1], pos2[2], nm[0], nm[1], nm[2], uv2[0], uv2[1], tangent1[0], tangent1[1], tangent1[2], bitangent1[0], bitangent1[1], bitangent1[2],
		pos3[0], pos3[1], pos3[2], nm[0], nm[1], nm[2], uv3[0], uv3[1], tangent1[0], tangent1[1], tangent1[2], bitangent1[0], bitangent1[1], bitangent1[2],

		pos1[0], pos1[1], pos1[2], nm[0], nm[1], nm[2], uv1[0], uv1[1], tangent2[0], tangent2[1], tangent2[2], bitangent2[0], bitangent2[1], bitangent2[2],
		pos3[0], pos3[1], pos3[2], nm[0], nm[1], nm[2], uv3[0], uv3[1], tangent2[0], tangent2[1], tangent2[2], bitangent2[0], bitangent2[1], bitangent2[2],
		pos4[0], pos4[1], pos4[2], nm[0], nm[1], nm[2], uv4[0], uv4[1], tangent2[0], tangent2[1], tangent2[2], bitangent2[0], bitangent2[1], bitangent2[2]
	};

	GLuint vbo;
	glGenVertexArrays(1, &vaoChocolate);
	glBindVertexArray(vaoChocolate);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 8));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 14, (void*)(sizeof(float) * 11));
	

	glEnable(GL_DEPTH_TEST);
}

void render(void) {
	float currentTime = getTime();
	gltime.delta = currentTime - gltime.last;
	gltime.last = currentTime;

	static float fade = 1.0f;
	static float mix = 0.0f;

	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	
	glClearBufferfv(GL_COLOR, 0, vec4(0.2f, 0.2f, 0.2f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	glViewport(0, 0, 1024, 1024);

	glUseProgram(modelProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, -0.89f, 0.0f));
	glUniform3fv(3, 1, vec3(0.0f, -10.0f, -10.0f));
	glUniform3fv(4, 1, glcamera.position);
	drawModel(&model, -1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texChoDiffuse);
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, 0.01f, 0.3f) * rotate(90.0f, vec3(1.0f, 0.0f, 0.0f)) * scale(0.1f, 1.0f, 0.01f));
	glBindVertexArray(vaoChocolate);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.1f, 0.01f, 0.0f) * rotate(90.0f, vec3(0.0f, 0.0f, 1.0f)) * scale(0.01f, 1.0f, 0.3f));
	glBindVertexArray(vaoChocolate);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUniformMatrix4fv(2, 1, GL_FALSE, translate(-0.1f, 0.01f, 0.0f) * rotate(90.0f, vec3(0.0f, 0.0f, 1.0f)) * scale(0.01f, 1.0f, 0.3f));
	glBindVertexArray(vaoChocolate);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUseProgram(mapProgram.programObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, perspective(45.0f, winSize.w / winSize.h, 0.1f, 100.0f));
	glUniformMatrix4fv(1, 1, GL_FALSE, lookat(glcamera.position, glcamera.front + glcamera.position, glcamera.up));
	glUniformMatrix4fv(2, 1, GL_FALSE, translate(0.0f, 0.02f, 0.0f) * scale(0.1f, 1.0f, 0.3f));
	glUniform3fv(3, 1, vec3(1.0f, 10.0f, 1.0f));
	glUniform3fv(4, 1, glcamera.position);
	glUniform1f(6, mix);
	glUniform1f(7, mix - 1.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texChoDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texChoNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texChoBump);
	glBindVertexArray(vaoChocolate);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

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
		mix += (float)gltime.delta * 0.05f;
		if(mix > 2.0f) {
			startMix = false;
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
	case 'Z': case 'z':
		startMix = true;
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
