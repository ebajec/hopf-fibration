#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include "defines.h"
#include "shader.h"
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#define PI 3.141592654f

struct Camera {
public:
	Camera() { this->_near_dist = 0; this->_far_dist = 0; }

	Camera(
		vec3 normal,
		vec3 pos,
		int w_screen = 1920,
		int h_screen = 1080,
		GLfloat focus = PI / 3,
		GLfloat far = 500
	);
	void init();
	void updateUniformData();
	bool bindToShader(Shader shader,const char* name, int binding);
	void rotate(float pitch, float yaw);
	void setScreenRatio(int w, int h); 	// Adjusts projection matrix to account for screen ratio.
	void translate(vec3 delta);			// Moves camera in direction delta. 
	void reset();
	vec3 pos() {return  (_pos - basis[2]*_near_dist);}
	vec3 coord(int i){return mat3(_model_pitch * _model_yaw)* basis[i];}


	GLuint ubo;
	GLfloat _near_dist;
	GLfloat _far_dist;
	vec3 _pos;
	vec3 basis[3];
	mat3 coord_trans; 	// converts std R^3 basis to camera coordinates
	mat4 _world;
	mat4 _model_pitch; 	//vertical rotations
	mat4 _model_yaw;	//horizontal rotations
	mat4 _proj;

	void _updateTransformations() {
		_world = mat4(mat3::id() | -1 * (_pos - basis[2] * _near_dist));
	}
};



#endif
