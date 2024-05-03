#version 330 core

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


void main() {
	
	vec4 v_pos_new = vec4(v_pos.xyz,1);
	//Make it so z is up
	v_pos_new = v_pos_new.xzyw;
	//vec4 normal_new = geom_model*vec4(normal,1);
	
	//lighting is based on geometry transformations
	frag_color = v_color;
	frag_pos = vec3(v_pos_new);
	frag_normal = vec3(v_normal);
	
	//now we do camera transformations
	v_pos_new = proj*view * v_pos_new;

	float w = v_pos_new.z/near;
	float depth = 2*(v_pos_new.z - near)/(far - near) - 1;

    gl_Position = vec4((v_pos_new).xy,depth,w);
}