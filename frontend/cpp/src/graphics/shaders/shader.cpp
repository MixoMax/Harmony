#include "shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <cmath>

#include "../../supporters/fileWizard.h"

Shader::Shader(const char *path) {
	name = path;
	const char *vertexShaderText = FileWizard::readFile(("res/shaders/" + std::string(path) + ".vert").c_str());
	const char *fragmentShaderText = FileWizard::readFile(("res/shaders/" + std::string(path) + ".frag").c_str());

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
	glCompileShader(fragmentShader);
	delete[] vertexShaderText;
	delete[] fragmentShaderText;

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glValidateProgram(program);
}

Shader::~Shader() {
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(program);
}

void Shader::useShader() const {
	glUseProgram(program);
}

GLint Shader::getUniform(const char *name) const {
	return glGetUniformLocation(program, name);
}

Shader::CompilationData Shader::getCompilationData() const {
	CompilationData data;

	int status;

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	data.vertexCompiled = status == GL_TRUE;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	data.fragmentCompiled = status == GL_TRUE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	data.programLinked = status == GL_TRUE;

	return data;
}

void Shader::dumpCompilationData() const {
	dumpCompilationData(name);
}

void Shader::dumpCompilationData(const char *name) const {
	Shader::CompilationData compilationData = getCompilationData();
	if (!compilationData.vertexCompiled) {
		std::cerr << "ERROR: " << name << " Vertex Shader Compilation Failed!" << std::endl;
	}
	if (!compilationData.fragmentCompiled) {
		std::cerr << "ERROR: " << name << " Fragment Shader Compilation Failed!" << std::endl;
	}
	if (!compilationData.programLinked) {
		std::cerr << "ERROR: " << name << " Shader Program Linking Failed!" << std::endl;
	}
}

void Shader::stopProgrammIfNotCompiledProperly(bool &running) const {
	stopProgrammIfNotCompiledProperly(name, running);
}

void Shader::stopProgrammIfNotCompiledProperly(const char *name, bool &running) const {
	Shader::CompilationData compilationData = getCompilationData();
	if (compilationData.success()) {
		return;
	}
	if (!compilationData.vertexCompiled) {
		std::cerr << "ERROR: " << name << " Vertex Shader Compilation Failed! Stopping program." << std::endl;
		running = false;
	}
	if (!compilationData.fragmentCompiled) {
		std::cerr << "ERROR: " << name << " Fragment Shader Compilation Failed! Stopping program." << std::endl;
		running = false;
	}
	if (!compilationData.programLinked) {
		std::cerr << "ERROR: " << name << " Shader Program Linking Failed! Stopping program." << std::endl;
		running = false;
	}


	std::cout << "Shader Compilation Log for " << name << ":" << std::endl;
	int shaderInfoLength = 1024;
	if (!compilationData.vertexCompiled) {
		char info[shaderInfoLength];
		glGetShaderiv(vertexShader, std::min(shaderInfoLength - 1, GL_INFO_LOG_LENGTH), &shaderInfoLength);
		glGetShaderInfoLog(vertexShader, shaderInfoLength, nullptr, info);
		std::cout << "Compilation (vertexShader) Info:" << info << std::endl;
		std::cout << "---------------------------------" << std::endl;
	}
	if (!compilationData.fragmentCompiled) {
		char info[shaderInfoLength];
		glGetShaderiv(fragmentShader, std::min(shaderInfoLength - 1, GL_INFO_LOG_LENGTH), &shaderInfoLength);
		glGetShaderInfoLog(fragmentShader, shaderInfoLength, nullptr, info);
		std::cout << "Compilation (fragmentShader) Info:" << info << std::endl;
		std::cout << "---------------------------------" << std::endl;
	}
	std::cout << "End of Shader Compilation Log for " << name << "." << std::endl;
}
