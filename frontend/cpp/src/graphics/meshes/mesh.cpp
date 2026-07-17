#include "mesh.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>


Mesh::Mesh(const void *vertices, int size, std::function<void()> initFunction, std::function<void()> drawFunction)
{
	setDrawFunction(drawFunction);
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	initFunction();

	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Mesh::setDrawFunction(std::function<void()> drawFunction) {
	this->draw = [this, drawFunction]()
	{
		if(vertexArray == 0 || vertexBuffer == 0) {
			std::cout << "Error: Attempted to draw uninitialized Mesh." << std::endl;
			return;
		}
		glBindVertexArray(vertexArray);
		drawFunction();
		glBindVertexArray(0);
	};
}

Mesh::~Mesh() {
    deleteMesh();
}
