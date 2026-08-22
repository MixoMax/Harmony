//
// Created by tim on 22.08.26.
//

#include "AudioVisualizer.h"

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
        newVertices.emplace_back(static_cast<float>(i) / data->size() * 1 - .5);
        newVertices.emplace_back(static_cast<float>((*data)[i]) / 65536.0f);
    }
    vertexCount = data->size() * 2;

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), newVertices.data(), GL_STREAM_DRAW);


    glUniform1f(shader.getUniform("uWidth"), 1.f / static_cast<float>(data->size()));
    mesh.draw();
}
