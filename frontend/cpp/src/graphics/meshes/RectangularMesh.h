//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_RECANGULARMESH_H
#define HARMONY_RECANGULARMESH_H
#include "mesh.h"


class RectangularMesh : public Mesh {
    static RectangularMesh *instance;

public:
    RectangularMesh();

    static RectangularMesh *getInstance();

    static void initializeMesh();

    static void deleteMesh();
};


#endif //HARMONY_RECANGULARMESH_H
