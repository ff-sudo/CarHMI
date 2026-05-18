#include <rhi/texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdio>

namespace CarHMI::RHI {

Texture2D::~Texture2D() {
    if (m_id) glDeleteTextures(1, &m_id);
}

bool Texture2D::LoadFromFile(const std::string& path) {
    stbi_set_flip_vertically_on_load(0);
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
    if (!data) {
        printf("Texture: cannot load %s\n", path.c_str());
        return false;
    }

    bool ok = LoadFromMemory(data, m_width, m_height, channels);
    stbi_image_free(data);
    return ok;
}

bool Texture2D::LoadFromMemory(const unsigned char* data, int width, int height, int channels) {
    m_width = width;
    m_height = height;

    GLenum internalFormat = GL_RGBA8;
    GLenum dataFormat = GL_RGBA;
    if (channels == 3) { internalFormat = GL_RGB8; dataFormat = GL_RGB; }
    else if (channels == 1) { internalFormat = GL_R8; dataFormat = GL_RED; }

    if (m_id) glDeleteTextures(1, &m_id);
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
    return true;
}

void Texture2D::CreateWhitePixel() {
    unsigned char white[] = { 255, 255, 255, 255 };
    LoadFromMemory(white, 1, 1, 4);
}

void Texture2D::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

} // namespace CarHMI::RHI
