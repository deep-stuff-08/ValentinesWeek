#version 460 core

out vec2 tex;

void main(void) {
	vec2 vPos[] = vec2[4](
		vec2(1.0, 1.0),
		vec2(-1.0, 1.0),
		vec2(1.0, -1.0),
		vec2(-1.0, -1.0)
	);
	gl_Position = vec4(vPos[gl_VertexID], 0.0, 1.0);
	tex = clamp(vPos[gl_VertexID], 0.0, 1.0);
}