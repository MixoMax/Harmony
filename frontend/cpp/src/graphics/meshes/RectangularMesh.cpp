//
// Created by tim on 17.07.26.
//

#include "RectangularMesh.h"

RectangularMesh *RectangularMesh::instance = nullptr;

inline float vertices[8]{
    -1.f, 1.f,
    1.f, 1.f,
    -1.f, -1.f,
    1.f, -1.f,
};

RectangularMesh::RectangularMesh() : Mesh(vertices, 8 * sizeof(float),
                                          [] {
                                              glEnableVertexAttribArray(0);
                                              glVertexAttribPointer(
                                                  0, 2, GL_FLOAT, GL_FALSE,
                                                  2 * sizeof(float), nullptr);
                                          }, [] {
                                              glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                                          }) {
}

RectangularMesh *RectangularMesh::getInstance() {
    return instance;
}

void RectangularMesh::initializeMesh() {
    instance = new RectangularMesh();
}

void RectangularMesh::deleteMesh() {
    delete instance;
}
