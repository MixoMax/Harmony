//
// Created by tim on 22.08.26.
//

#ifndef HARMONY_AUDIOVISUALIZER_H
#define HARMONY_AUDIOVISUALIZER_H
#include <atomic>
#include <memory>
#include <vector>

#include "../../../BasicCppLibrary/visuals/mesh.h"
#include "../../BasicCppLibrary/visuals/geometryShader.h"


class AudioVisualizer {
    GeometryShader shader{"audioVisualizerShader"};
    float vertices[8]{
        -.5, .5,
        -.3, .3,
        -.1, .1,
        .1, .4
    };

    GLsizei vertexCount = 4;

    Mesh mesh{
        vertices, 8 * sizeof(float), [] {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0, 2, GL_FLOAT, GL_FALSE,
                2 * sizeof(float), nullptr);
        },
        [this] {
            glDrawArrays(GL_POINTS, 0, vertexCount);
        }
    };

public:
    static std::atomic<std::shared_ptr<std::vector<int16_t> > > audioData;

    AudioVisualizer();


    void draw();
};


#endif //HARMONY_AUDIOVISUALIZER_H
