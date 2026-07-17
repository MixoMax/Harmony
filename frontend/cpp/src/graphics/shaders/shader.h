#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader
{
private:
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;
	const char *name;

public:
	struct CompilationData
	{
		bool vertexCompiled;
		bool fragmentCompiled;
		bool programLinked;

		bool success() const
		{
			return vertexCompiled && fragmentCompiled && programLinked;
		}
	};

	Shader(const char *path);

	~Shader();

	void useShader() const;
	GLint getUniform(const char *name) const;

	CompilationData getCompilationData() const;

	void dumpCompilationData() const;
	void dumpCompilationData(const char *name) const;

	void stopProgrammIfNotCompiledProperly(bool &running) const;
	void stopProgrammIfNotCompiledProperly(const char *name, bool &running) const;
};
