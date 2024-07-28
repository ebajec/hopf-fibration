#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include "matrix.hpp"

#include "defines.h"

class Shader {
public:
	Shader(){}
	bool addShader(const char* path, GLuint type);
	void useProgram() const { glUseProgram(program); }
	bool linkProgram();

	void setUniform(const char* name, int value);
	void setUniform(const char* name, unsigned int value);
	void setUniform(const char* name, float value);
	void setUniform(const char* name, vec3 value);
	void setUniform(const char* name, mat3 value, GLboolean transpose);
	void setUniform(const char* name, mat4 value, GLboolean transpose);

	GLint getUniform(const char* name) const { return glGetUniformLocation(program, name); }

	bool isLinked() {return initialized;}
	GLuint program;
protected:
	bool initialized = false;
	void init();
	bool compileShader(GLenum type, const char* source);
};

#endif

