//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_MESHINITIALIZER_H
#define HARMONY_MESHINITIALIZER_H
#include "RectangularMesh.h"

inline void initializeMeshes() {
    RectangularMesh::initializeMesh();
}

inline void deleteMeshes() {
    RectangularMesh::deleteMesh();
}

#endif //HARMONY_MESHINITIALIZER_H
