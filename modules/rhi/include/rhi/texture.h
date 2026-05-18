#pragma once

#include <glad/gl.h>
#include <string>

namespace CarHMI::RHI {

class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();

    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const unsigned char* data, int width, int height, int channels);
    void CreateWhitePixel();

    void Bind(unsigned int slot = 0) const;

    GLuint GetID() const { return m_id; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    GLuint m_id = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace CarHMI::RHI
