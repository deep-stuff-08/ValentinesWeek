#include<shapes.h>
#include<iostream>
#include<math.h>
#include<GL/glew.h>
#include<vmath.h>

using namespace std;
using namespace vmath;

static GLint oldVao, oldVbo, oldEabo;

typedef struct vertex_t {
	vec3 position;
	vec3 normal;
	vec2 texcoord;
} vertex_t;

void initUnitSphere(shapes_dl *shape, int stacks, int slices, int vPos, int vNor, int vTex, int vTan, int vBit) {
	const float radius = 1.0f;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldVbo);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);

	glGenVertexArrays(1, &shape->vao);
	glBindVertexArray(shape->vao);

	glGenBuffers(1, &shape->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (stacks + 1) * (slices + 1), NULL, GL_STATIC_DRAW);
	vertex_t *array = (vertex_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * (stacks + 1) * (slices + 1), GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= stacks; i++) {
		float phi = M_PI * ((float)i / stacks);
		for(int j = 0; j <= slices; j++) {
			float theta = 2.0f * M_PI * ((float)j / slices);
			vec3 pos = vec3(sin(phi) * sin(theta) * radius, cos(phi) * radius, sin(phi) * cos(theta) * radius);
			array[index].position = pos;
			array[index].normal = normalize(pos);
			array[index].texcoord = vec2((float)j / slices, 1.0f - ((float)i / stacks));
			index++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(vPos > -1) {
		glEnableVertexAttribArray(vPos);
		glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	}
	if(vNor > -1) {
		glEnableVertexAttribArray(vNor);
		glVertexAttribPointer(vNor, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	}
	if(vTex > -1) {
		glEnableVertexAttribArray(vTex);
		glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));
	}

	glGenBuffers(1, &shape->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < stacks; i++) {
		for(int j = 0; j < slices; j++) {
			elementarray[index++] = uvec3(i * (slices + 1) + j, (i + 1) * (slices + 1) + j, (i + 1) * (slices + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (slices + 1) + j + 1, i * (slices + 1) + j + 1, i * (slices + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	shape->triangleCount = stacks * slices * 2 * 3;

	glBindVertexArray(oldVao);
	glBindBuffer(GL_ARRAY_BUFFER, oldVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void initUnitNPrisim(shapes_dl *shape, int stacks, int slices, int vPos, int vNor, int vTex, int vTan, int vBit) {
	const float radius = 1.0f;
	const float height = 2.0f;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldVbo);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);

	glGenVertexArrays(1, &shape->vao);
	glBindVertexArray(shape->vao);

	glGenBuffers(1, &shape->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (stacks + 1) * (slices + 1), NULL, GL_STATIC_DRAW);
	vertex_t *array = (vertex_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * (stacks + 1) * (slices + 1), GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= stacks; i++) {
		float deltaH = height * ((float)i / stacks) - 1.0f;
		for(int j = 0; j <= slices; j++) {
			float theta = 2.0f * M_PI * ((float)j / slices);
			vec3 pos = vec3(sin(theta) * radius, deltaH, cos(theta) * radius);
			array[index].position = pos;
			array[index].normal = normalize(pos);
			array[index].texcoord = vec2((float)j / slices, 1.0f - ((float)i / stacks));
			index++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(vPos > -1) {
		glEnableVertexAttribArray(vPos);
		glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	}
	if(vNor > -1) {
		glEnableVertexAttribArray(vNor);
		glVertexAttribPointer(vNor, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	}
	if(vTex > -1) {
		glEnableVertexAttribArray(vTex);
		glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));
	}

	glGenBuffers(1, &shape->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < stacks; i++) {
		for(int j = 0; j < slices; j++) {
			elementarray[index++] = uvec3(i * (slices + 1) + j, (i + 1) * (slices + 1) + j, (i + 1) * (slices + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (slices + 1) + j + 1, i * (slices + 1) + j + 1, i * (slices + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	shape->triangleCount = stacks * slices * 2 * 3;
	glBindVertexArray(oldVao);
	glBindBuffer(GL_ARRAY_BUFFER, oldVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void initUnitTorus(shapes_dl *shape, int stacks, int slices, float innerRadius, int vPos, int vNor, int vTex, int vTan, int vBit) {
	const float outerRadius = 1.0f;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldVbo);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);

	glGenVertexArrays(1, &shape->vao);
	glBindVertexArray(shape->vao);

	glGenBuffers(1, &shape->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (stacks + 1) * (slices + 1), NULL, GL_STATIC_DRAW);
	vertex_t *array = (vertex_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * (stacks + 1) * (slices + 1), GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= slices; i++) {
		float phi = 2.0f * M_PI * ((float)i / slices);
		for(int j = 0; j <= stacks; j++) {
			float theta = 2.0f * M_PI * ((float)j / stacks);
			array[index].position = vec3(cos(phi) * (outerRadius + cos(theta) * innerRadius), sin(theta) * innerRadius, sin(phi) * (outerRadius + cos(theta) * innerRadius));
			array[index].normal = normalize(vec3(cos(phi) * cos(theta), sin(theta), sin(phi) * cos(theta)));
			array[index].texcoord = vec2(1.0f - ((float)i / stacks), (float)j / slices);
			index++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(vPos > -1) {
		glEnableVertexAttribArray(vPos);
		glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	}
	if(vNor > -1) {
		glEnableVertexAttribArray(vNor);
		glVertexAttribPointer(vNor, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	}
	if(vTex > -1) {
		glEnableVertexAttribArray(vTex);
		glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));
	}

	glGenBuffers(1, &shape->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < slices; i++) {
		for(int j = 0; j < stacks; j++) {
			elementarray[index++] = uvec3(i * (stacks + 1) + j, (i + 1) * (stacks + 1) + j, (i + 1) * (stacks + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (stacks + 1) + j + 1, i * (stacks + 1) + j + 1, i * (stacks + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	shape->triangleCount = stacks * slices * 2 * 3;
	glBindVertexArray(oldVao);
	glBindBuffer(GL_ARRAY_BUFFER, oldVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void initUnitSpring(shapes_dl *shape, int stacks, int slices, int twists, float innerRadius, int vPos, int vNor, int vTex, int vTan, int vBit) {
	const float outerRadius = 1.0f;
	const float height = 2.0f;

	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldVbo);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);

	glGenVertexArrays(1, &shape->vao);
	glBindVertexArray(shape->vao);

	glGenBuffers(1, &shape->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * (stacks + 1) * (slices + 1) * 2, NULL, GL_STATIC_DRAW);
	vec3 *array = (vec3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vec3) * (stacks + 1) * (slices + 1) * 2, GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= slices; i++) {
		float phi = 2.0f * M_PI * ((float)i / slices) * twists;
		float deltaH = height * ((float)i / slices) - 1.0f;
		for(int j = 0; j <= stacks; j++) {
			float theta = 2.0f * M_PI * ((float)j / stacks);
			array[index++] = vec3(cos(phi) * (outerRadius + cos(theta) * innerRadius), sin(phi) * (outerRadius + cos(theta) * innerRadius), sin(theta) * innerRadius + deltaH);
			array[index++] = normalize(vec3(cos(phi) * cos(theta), sin(phi) * cos(theta), sin(theta)));
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	if(vPos > -1) {
		glEnableVertexAttribArray(vPos);
		glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)(0));
	}
	if(vNor > -1) {
		glEnableVertexAttribArray(vNor);
		glVertexAttribPointer(vNor, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)(sizeof(vec3)));
	}

	glGenBuffers(1, &shape->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < slices; i++) {
		for(int j = 0; j < stacks; j++) {
			elementarray[index++] = uvec3(i * (stacks + 1) + j, (i + 1) * (stacks + 1) + j, (i + 1) * (stacks + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (stacks + 1) + j + 1, i * (stacks + 1) + j + 1, i * (stacks + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	shape->triangleCount = stacks * slices * 2 * 3;
	glBindVertexArray(oldVao);
	glBindBuffer(GL_ARRAY_BUFFER, oldVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void initUnitNPyramid(shapes_dl *shape, int stacks, int slices, int vPos, int vNor, int vTex, int vTan, int vBit) {
	const float radius = 1.0f;
	const float height = 2.0f;
	const float lenght = sqrt(radius * radius + height * height);
	const float ratio = height / lenght;
	const float ynorm = radius / lenght;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldVbo);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);

	glGenVertexArrays(1, &shape->vao);
	glBindVertexArray(shape->vao);

	glGenBuffers(1, &shape->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, shape->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (stacks + 1) * (slices + 1), NULL, GL_STATIC_DRAW);
	vertex_t *array = (vertex_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * (stacks + 1) * (slices + 1), GL_MAP_WRITE_BIT);

	int index = 0;
	for(int i = 0; i <= stacks; i++) {
		float deltaH = height * ((float)i / stacks) - 1.0f;
		float delatR = 1.0f - radius * ((float)i / stacks);
		for(int j = 0; j <= slices; j++) {
			float theta = 2.0f * M_PI * ((float)j / slices);
			vec3 pos = vec3(sin(theta) * delatR, deltaH, cos(theta) * delatR);
			array[index].position = pos;
			array[index].normal = normalize(pos);
			array[index].texcoord = vec2((float)j / slices, 1.0f - ((float)i / stacks));
			index++;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	if(vPos > -1) {
		glEnableVertexAttribArray(vPos);
		glVertexAttribPointer(vPos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, position)));
	}
	if(vNor > -1) {
		glEnableVertexAttribArray(vNor);
		glVertexAttribPointer(vNor, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, normal)));
	}
	if(vTex > -1) {
		glEnableVertexAttribArray(vTex);
		glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)(offsetof(vertex_t, texcoord)));
	}

	glGenBuffers(1, &shape->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uvec3) * slices * stacks * 2, NULL, GL_STATIC_DRAW);
	uvec3 *elementarray = (uvec3*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uvec3) * stacks * slices * 2, GL_MAP_WRITE_BIT);
	
	index = 0;
	for(int i = 0; i < stacks; i++) {
		for(int j = 0; j < slices; j++) {
			elementarray[index++] = uvec3(i * (slices + 1) + j, (i + 1) * (slices + 1) + j, (i + 1) * (slices + 1) + j + 1);
			elementarray[index++] = uvec3((i + 1) * (slices + 1) + j + 1, i * (slices + 1) + j + 1, i * (slices + 1) + j);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	shape->triangleCount = stacks * slices * 2 * 3;
	glBindVertexArray(oldVao);
	glBindBuffer(GL_ARRAY_BUFFER, oldVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void renderShape(shapes_dl* shape, int instance) {
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldVao);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &oldEabo);
	glBindVertexArray(shape->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape->eabo);
	glDrawElementsInstanced(GL_TRIANGLES, shape->triangleCount, GL_UNSIGNED_INT, NULL, instance);
	glBindVertexArray(oldVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oldEabo);
}

void uninitShape(shapes_dl* shape) {
	glDeleteVertexArrays(1, &shape->vao);
	glDeleteBuffers(1, &shape->vbo);
	glDeleteBuffers(1, &shape->eabo);
}