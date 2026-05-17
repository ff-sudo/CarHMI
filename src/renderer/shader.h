#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace CarHMI {

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool LoadFromFile(const std::string& vertPath, const std::string& fragPath);
    bool LoadFromSource(const char* vertSrc, const char* fragSrc);

    void Bind() const;
    void Unbind() const;

    void SetInt(const char* name, int value) const;
    void SetIntArray(const char* name, const int* values, int count) const;
    void SetFloat(const char* name, float value) const;
    void SetVec2(const char* name, const glm::vec2& v) const;
    void SetVec4(const char* name, const glm::vec4& v) const;
    void SetMat4(const char* name, const glm::mat4& m) const;

    GLuint GetID() const { return m_id; }

private:
    GLuint m_id = 0;

    bool Compile(const char* vertSrc, const char* fragSrc);
    bool CheckCompileErrors(GLuint shader, const char* type);
};

} // namespace CarHMI
