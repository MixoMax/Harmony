#pragma once
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Mesh {
protected:
	GLuint vertexArray;
	GLuint vertexBuffer;

public:
	std::function<void()> draw;

	Mesh(const void *vertices, int size, std::function<void()> initFunction, std::function<void()> drawFunction);

	Mesh() = delete;

	Mesh(Mesh &&other) = delete;

	Mesh &operator=(Mesh &&other) = delete;


	void setDrawFunction(std::function<void()> drawFunction);

	~Mesh();

	void deleteMesh() {
		// std::cout << "deleting Mesh" << std::endl;
		if (vertexBuffer != 0) {
			glDeleteBuffers(1, &vertexBuffer);
			vertexBuffer = 0;
		}
		if (vertexArray != 0) {
			glDeleteVertexArrays(1, &vertexArray);
			vertexArray = 0;
		}
	}
};

