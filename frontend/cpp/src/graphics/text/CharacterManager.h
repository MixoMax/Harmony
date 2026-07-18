//
// Created by tim on 18.07.26.
//

#ifndef HARMONY_CHARACTERMANAGER_H
#define HARMONY_CHARACTERMANAGER_H
#include <map>

#include "../../../BasicCppMathLibrary/vectors/ivec2.h"
#include "../../../BasicCppMathLibrary/vectors/vec3.h"
#include "../../../BasicCppMathLibrary/vectors/vec4.h"
#include "../../supporters/Alignment.h"
#include "../meshes/RectangularMesh.h"
#include "../shaders/shader.h"

struct Character {
    GLuint textureID;
    ivec2 size;
    ivec2 bearing; // Offset from baseline to left/top of glyph
    unsigned int advance; //Offset to advance to next glyph

    Character() : textureID(0), size(ivec2(0, 0)), bearing(ivec2(0, 0)), advance(0) {
    }

    Character(const unsigned int textureId, const ivec2 &size, const ivec2 &bearing, const unsigned int advance)
        : textureID(textureId),
          size(size),
          bearing(bearing),
          advance(advance) {
    }
};

class CharacterManager {
    static Shader *shader;
    static GLuint vao, vbo;

public:
    static std::map<char, Character> characters;

    static void initializeCharacterManager();

    static void deleteCharacterManager();

    static vec2 getSizeOfText(const std::string &text, float scale);

    static void drawText(const std::string &text, float x, float y, float scale, float rotation, const vec4 &color,
                         Alignment alignment);
};


#endif //HARMONY_CHARACTERMANAGER_H
