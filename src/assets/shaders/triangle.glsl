#shader vert

const vec2 pos[3] = {
	vec2(-0.4, +0.6),
	vec2(+0.4, +0.6),
	vec2(0, -0.6)
};

const vec3 col[3] = {
	vec3(0.3255, 0.3765, 0.5333),
	vec3(0.302, 0.4588, 0.9686),
	vec3(0.7059, 0.7529, 0.9059)
};

layout(location = 0) out vec3 out_color;

void main() {
	gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
	out_color = col[gl_VertexIndex];
}

#shader frag

layout(location = 0) in vec3 in_color;
layout(location = 0) out vec4 out_color;

void main() {
	out_color = vec4(in_color, 1.0);
}
