//
// Created by tim on 22.08.26.
//

#include "AudioVisualizer.h"

#include <iostream>

#include "../../graphics/meshes/RectangularMesh.h"


std::atomic<std::shared_ptr<std::vector<int16_t> > > AudioVisualizer::audioData = std::make_shared<std::vector<
    int16_t> >();

AudioVisualizer::AudioVisualizer() {
    shader.dumpCompilationData();
}

void AudioVisualizer::draw() {
    shader.useShader();

    const auto data = audioData.load();
    if (data == nullptr) {
        return;
    }

    std::vector<float> newVertices{};
    for (int i = 0; i < data->size(); i++) {
        float influence = std::min(std::clamp((i) / 300.f, 0.f, 1.f),
                                   std::max(0.f, (data->size() - i) / 300.f));

        newVertices.emplace_back(static_cast<float>(i) / data->size() * 1 - .5);
        newVertices.emplace_back(static_cast<float>((*data)[i]) * influence / 23768.0f);
    }
    vertexCount = newVertices.size();

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), newVertices.data(), GL_STREAM_DRAW);


    glUniform1f(shader.getUniform("uWidth"), 1.f / static_cast<float>(data->size()));
    mesh.draw();
}
