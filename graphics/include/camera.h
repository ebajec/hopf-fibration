#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "defines.h"
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#define PI 3.141592654f

struct Camera {
public:
	Camera() {}
	Camera(
		vec3 normal,
		vec3 position,
		int w_screen = 1920,
		int h_screen = 1080,
		GLfloat fov = PI / 3,
		GLfloat near = 1,
		GLfloat far = 500
	);
	~Camera();

	void updateUbo();
	void bindUbo(GLuint binding);

	void rotate(float pitch, float yaw);
	void translate(vec3 delta);
	void resize(int width, int height);

	mat4 getViewMatrix();
	mat4 getProjMatrix();

	vec3 coord(int i){return vec3(coords[i]);}

	GLuint ubo;

	//new fields
	float fov, aspect, near, far;
	vec3 viewPos;

	vec3 position;
	mat4 coords;
	mat4 proj;
};



#endif
