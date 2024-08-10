#version 430 core

layout (std140,binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
	vec4 cam_pos;
	vec4 cam_dir;
	float near;
	float far;
};

layout (std430, binding = 0) buffer Positions
{
    vec4[] positions;
};

uniform mat4 model;
uniform float scale;

layout (location = 0) in vec4 v_pos;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec4 v_normal;

out vec4 fcolor;
out vec3 fpos;
out vec3 fnormal;

vec3 hsvtorgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	// Apply geometry transformation
    vec4 offset = positions[gl_InstanceID];

	vec4 v_pos_new = model*vec4(scale*v_pos.xyz,1) + offset;
	vec4 v_normal_new = model*vec4(v_normal.xyz,0);

	fcolor = v_color;
	fpos = vec3(v_pos_new);
	fnormal = vec3(v_normal_new);

	// Make it so z is up
	v_pos_new = v_pos_new.xzyw;
	
	v_pos_new = proj*view * v_pos_new;

	float w = v_pos_new.z/near;
	float depth = 2*(v_pos_new.z - near)/(far - near) - 1;

    gl_Position = vec4((v_pos_new).xy,depth,w);
}