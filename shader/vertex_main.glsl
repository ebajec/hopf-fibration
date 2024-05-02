#version 330 core

//uniform float near;
//uniform float far;
//uniform mat4 view;
//uniform mat4 proj;

layout (std140) uniform Camera {
    mat4 view;
    mat4 proj;
	float near;
	float far;
};

layout (location = 0) in vec4 v_pos;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec4 v_normal;

out vec4 frag_color;
out vec3 frag_pos;
out vec3 frag_normal;

vec3 hsvtorgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	
	vec4 v_pos_new = vec4(v_pos.xyz,1);
	//Make it so z is up
	v_pos_new = v_pos_new.xzyw;
	//vec4 normal_new = geom_model*vec4(normal,1);
	
	//lighting is based on geometry transformations
	frag_color = vec4(hsvtorgb(vec3(v_pos.w,1,1)),1);
	frag_pos = vec3(v_pos_new);
	frag_normal = vec3(v_normal);
	
	//now we do camera transformations
	v_pos_new = proj*view * v_pos_new;

	float w = v_pos_new.z/near;
	float depth = 2*(v_pos_new.z - near)/(far - near) - 1;

    gl_Position = vec4((v_pos_new).xy,depth,w);
}