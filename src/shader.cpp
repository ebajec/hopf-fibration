#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#define MAX_LINE_LENGTH 100

inline string read_text_file(const char* src){
	ifstream file(src);

	if (!file.is_open()) {
		fprintf(stderr, "ERROR: could not open file:\n",src,"\n");
		return "";
	}

	string line;
	string text;

	while (std::getline(file,line)) {
		text.append(line).append("\n");
	}

	return text;
}

/*
	Creates shader with source from path and specified type.  Automatically links
	program, so if multipe shaders are needed use addShader() and link().
*/
ShaderProgram::ShaderProgram(
const char* path, GLuint type) {
	_init();
	addShader(path,type);
	glLinkProgram(program);
}

void ShaderProgram::addShader(const char *path, GLuint type)
{
	if (!initialized) {
		_init();
	}
	string source = read_text_file(path);

	if (source == "") {
		fprintf(stderr, "ERROR: could not open shader files\n");
		glfwTerminate();
	}
	
	//compile shaders
	_compileShader(type, source.c_str());
}

void ShaderProgram::link() {
	glLinkProgram(program);
	GLint success = 0;
	glGetShaderiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE) {
		GLint length = 0;
		glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> error_message(length);
		glGetShaderInfoLog(program, length, NULL, &error_message[0]);

		printf(&error_message[0]);
	}
}

void ShaderProgram::setUniform(const char* name, int value)
{
	glUniform1i(this->getUniform(name), value);
}

void ShaderProgram::setUniform(const char* name, unsigned int value) {
	glUniform1ui(this->getUniform(name), value);
}

void ShaderProgram::setUniform(const char* name, float value)
{
	glUniform1f(this->getUniform(name), value);
}

void ShaderProgram::setUniform(const char* name, vec3 value)
{
	glUniform3fv(this->getUniform(name), 1, value.data());
}

void ShaderProgram::setUniform(const char* name, mat3 value, GLboolean transpose)
{
	glUniformMatrix3fv(this->getUniform(name), 1, transpose, value.data());
}

void ShaderProgram::setUniform(const char* name, mat4 value, GLboolean transpose)
{
	glUniformMatrix4fv(this->getUniform(name), 1, transpose, value.data());
}

void ShaderProgram::_compileShader(GLenum type, const char* source)
{
	GLuint s = glCreateShader(type);
	glShaderSource(s, 1, &source, NULL);
	glCompileShader(s);

	GLint success = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		GLint length = 0;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> error_message(length);
		glGetShaderInfoLog(s, length, NULL, &error_message[0]);

		printf(&error_message[0]);

		glDeleteShader(s);
	}
	else {
		glAttachShader(program, s);
	}


}

