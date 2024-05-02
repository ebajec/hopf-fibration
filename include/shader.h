#pragma once
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include "matrix.hpp"

#ifndef SHADER_H
#define SHADER_H

using vec3 = matrix<3, 1, GLfloat>;
using mat3 = matrix<3, 3, GLfloat>;
using mat4 = matrix<4, 4, GLfloat>;

class ShaderProgram {
public:
	ShaderProgram() {}
	ShaderProgram(const char* path, GLuint type);
	void addShader(const char* path, GLuint type);
	void use() const { glUseProgram(program); }
	void link();

	void setUniform(const char* name, int value);
	void setUniform(const char* name, unsigned int value);
	void setUniform(const char* name, float value);
	void setUniform(const char* name, vec3 value);
	void setUniform(const char* name, mat3 value, GLboolean transpose);
	void setUniform(const char* name, mat4 value, GLboolean transpose);

	GLint getUniform(const char* name) const { return glGetUniformLocation(program, name); }

	GLuint program;

	bool initialized = false;
protected:

	void _compileShader(GLenum type, const char* source);

	//create shader program
	void _init() { 
		initialized = true;
		program = glCreateProgram(); 
		}

};

#endif

