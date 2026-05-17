#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <cstdio>

namespace CarHMI {

Shader::~Shader() {
    if (m_id) glDeleteProgram(m_id);
}

bool Shader::LoadFromFile(const std::string& vertPath, const std::string& fragPath) {
    auto readFile = [](const std::string& path) -> std::string {
        std::ifstream f(path);
        if (!f.is_open()) {
            printf("Shader: cannot open %s\n", path.c_str());
            return "";
        }
        std::stringstream ss;
        ss << f.rdbuf();
        return ss.str();
    };

    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    return Compile(vertSrc.c_str(), fragSrc.c_str());
}

bool Shader::LoadFromSource(const char* vertSrc, const char* fragSrc) {
    return Compile(vertSrc, fragSrc);
}

void Shader::Bind() const { glUseProgram(m_id); }
void Shader::Unbind() const { glUseProgram(0); }

void Shader::SetInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void Shader::SetIntArray(const char* name, const int* values, int count) const {
    glUniform1iv(glGetUniformLocation(m_id, name), count, values);
}

void Shader::SetFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name), value);
}

void Shader::SetVec2(const char* name, const glm::vec2& v) const {
    glUniform2f(glGetUniformLocation(m_id, name), v.x, v.y);
}

void Shader::SetVec4(const char* name, const glm::vec4& v) const {
    glUniform4f(glGetUniformLocation(m_id, name), v.x, v.y, v.z, v.w);
}

void Shader::SetMat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(m));
}

bool Shader::Compile(const char* vertSrc, const char* fragSrc) {
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);
    if (!CheckCompileErrors(vert, "VERTEX")) { glDeleteShader(vert); return false; }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);
    if (!CheckCompileErrors(frag, "FRAGMENT")) { glDeleteShader(vert); glDeleteShader(frag); return false; }

    if (m_id) glDeleteProgram(m_id);
    m_id = glCreateProgram();
    glAttachShader(m_id, vert);
    glAttachShader(m_id, frag);
    glLinkProgram(m_id);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return CheckCompileErrors(m_id, "PROGRAM");
}

bool Shader::CheckCompileErrors(GLuint shader, const char* type) {
    int success;
    char log[1024];

    if (std::string(type) != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            printf("Shader %s compile error:\n%s\n", type, log);
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, sizeof(log), nullptr, log);
            printf("Shader PROGRAM link error:\n%s\n", log);
            return false;
        }
    }
    return true;
}

} // namespace CarHMI
