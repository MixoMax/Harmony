//
// Created by tim on 18.07.26.
//

#include "CharacterManager.h"

#include <ranges>
#include <freetype/freetype.h>

#include "../GraphicsManager.h"
#include "../../settings.h"
#include "glad/glad.h"

std::map<char, Character> CharacterManager::characters{};
Shader *CharacterManager::shader = nullptr;
GLuint CharacterManager::vao = 0;
GLuint CharacterManager::vbo = 0;

void CharacterManager::initializeCharacterManager() {
    shader = new Shader{"text/glyphShader"};

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }
    FT_Set_Pixel_Sizes(face, 0, fontQualityInPixel);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            static_cast<int>(face->glyph->bitmap.width),
            static_cast<int>(face->glyph->bitmap.rows),
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        Character character{
            textureID,
            ivec2{static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows)},
            ivec2{face->glyph->bitmap_left, face->glyph->bitmap_top},
            static_cast<unsigned>(face->glyph->advance.x)
        };

        characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void CharacterManager::deleteCharacterManager() {
    for (auto &character: characters | std::views::values) {
        glDeleteTextures(1, &character.textureID);
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    delete shader;
}

vec2 CharacterManager::getSizeOfText(const std::string &text, const float scale) {
    vec2 size{};

    for (char c: text) {
        const Character &character = characters[c];

        size.x += static_cast<float>(character.advance >> 6) * scale;

        const float h = static_cast<float>(character.size.y) * scale;
        size.y = std::max(size.y, h);
    }

    return size;
}


void CharacterManager::drawText(const std::string &text,
                                float x, float y,
                                const float scale,
                                const float rotation,
                                const vec4 &color,
                                const Alignment alignment) {
    shader->useShader();
    const GraphicsManager &graphicsManager = GraphicsManager::getInstance();
    glUniform2f(shader->getUniform("uScreenSize"),
                static_cast<float>(graphicsManager.getScreenWidth()),
                static_cast<float>(graphicsManager.getScreenHeight()));
    glUniform1f(shader->getUniform("uRotation"), rotation);
    glUniform4f(shader->getUniform("uColor"), color.x, color.y, color.z, color.w);
    glUniform1f(shader->getUniform("uTime"), glfwGetTime());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    const vec2 offset = getSizeOfText(text, scale);
    x += -offset.x / 2.f * (1.f + alignment.x);
    y += -offset.y / 2.f * (1.f + alignment.y);


    for (char c: text) {
        const Character &character = characters[c];
        const float xPos = x + static_cast<float>(character.bearing.x) * scale;
        const float yPos = y - static_cast<float>(character.size.y - character.bearing.y) * scale;

        const float w = static_cast<float>(character.size.x) * scale;
        const float h = static_cast<float>(character.size.y) * scale;

        const float vertices[4][4] = {
            {xPos, yPos + h, 0.0f, 0.0f},
            {xPos, yPos, 0.0f, 1.0f},
            {xPos + w, yPos + h, 1.0f, 0.0f},
            {xPos + w, yPos, 1.0f, 1.0f},
        };

        glBindTexture(GL_TEXTURE_2D, character.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        x += static_cast<float>(character.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
