#ifndef __SHAPES__
#define __SHAPES__

struct shapes_dl {
	unsigned vao;
	unsigned vbo;
	unsigned eabo;
	unsigned triangleCount;
};

void initUnitNPrisim(shapes_dl *shape, int stacks, int slices, int vPos = -1, int vNor = -1, int vTex = -1, int vTan = -1, int vBit = -1);
void initUnitNPyramid(shapes_dl *shape, int stacks, int slices, int vPos = -1, int vNor = -1, int vTex = -1, int vTan = -1, int vBit = -1);
void initUnitSphere(shapes_dl *shape, int stacks, int slices, int vPos = -1, int vNor = -1, int vTex = -1, int vTan = -1, int vBit = -1);
void initUnitTorus(shapes_dl *shape, int stacks, int slices, float innerRadius, int vPos = -1, int vNor = -1, int vTex = -1, int vTan = -1, int vBit = -1);
void initUnitSpring(shapes_dl *shape, int stacks, int slices, int twists, float innerRadius, int vPos = -1, int vNor = -1, int vTex = -1, int vTan = -1, int vBit = -1);

void renderShape(shapes_dl *shape, int instance = 1);

void uninitShape(shapes_dl *shape);
#endif