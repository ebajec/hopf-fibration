#version 430 core

layout (std140,binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
	mat4 pv;
	vec4 cam_pos;
	vec4 cam_dir;
	float near;
	float far;
};

uniform mat4 model;
uniform float scale;

layout (location = 0) in vec4 v_pos;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec4 v_normal;

out vec4 fcolor;
out vec3 fpos;
out vec3 fnormal;

void main() {
	// Apply geometry transformation
	vec4 position = model*vec4(scale*v_pos.xyz,1);
	vec4 normal = model*vec4(v_normal.xyz,0);

	fcolor =  v_color;
	fpos = vec3(position);
	fnormal = vec3(normal);

	position = proj*view*position;
    gl_Position = vec4(position);
}