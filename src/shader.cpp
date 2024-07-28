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

void Shader::init()
{
	if (!initialized)
		program = glCreateProgram();
	initialized = true;
}

bool Shader::addShader(const char *path, GLuint type)
{
	init();
	string source = read_text_file(path);

	if (source == "") {
		fprintf(stderr, "ERROR: could not open shader files\n");
		glfwTerminate();
		return false;
	}
	
	//compile shaders
	if(!compileShader(type, source.c_str()))
		return false;
	return true;
}

bool Shader::linkProgram() {
	
	glLinkProgram(program);
	GLint success = 0;
	glGetShaderiv(program, GL_LINK_STATUS, &success);

	if (success == GL_FALSE) {
		GLint length = 0;
		glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> error_message(length);
		glGetShaderInfoLog(program, length, NULL, &error_message[0]);

		printf(&error_message[0]);
		return false;
	}

	initialized = true;
	return true;
}

void Shader::setUniform(const char* name, int value)
{
	glUniform1i(this->getUniform(name), value);
}

void Shader::setUniform(const char* name, unsigned int value) {
	glUniform1ui(this->getUniform(name), value);
}

void Shader::setUniform(const char* name, float value)
{
	glUniform1f(this->getUniform(name), value);
}

void Shader::setUniform(const char* name, vec3 value)
{
	glUniform3fv(this->getUniform(name), 1, value.data());
}

void Shader::setUniform(const char* name, mat3 value, GLboolean transpose)
{
	glUniformMatrix3fv(this->getUniform(name), 1, transpose, value.data());
}

void Shader::setUniform(const char* name, mat4 value, GLboolean transpose)
{
	glUniformMatrix4fv(this->getUniform(name), 1, transpose, value.data());
}

bool Shader::compileShader(GLenum type, const char* source)
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
		return false;
	}
	else {
		glAttachShader(program, s);
		return true;
	}


}

