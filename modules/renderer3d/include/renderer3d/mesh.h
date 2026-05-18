#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace CarHMI::Render3D {

struct Vertex3D {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Material3D {
    std::string name;
    glm::vec3 ambient  = {0.2f, 0.2f, 0.2f};
    glm::vec3 diffuse  = {0.8f, 0.8f, 0.8f};
    glm::vec3 specular = {0.5f, 0.5f, 0.5f};
    float shininess = 32.0f;
    GLuint diffuseTexture = 0;
    bool hasDiffuseTexture = false;
};

struct SubMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    int indexCount = 0;
    int materialIndex = -1;
};

class Mesh {
public:
    ~Mesh();

    bool LoadOBJ(const std::string& path);
    void Draw(GLuint shaderProgram) const;

    const std::vector<Material3D>& GetMaterials() const { return m_materials; }
    glm::vec3 GetCenter() const { return m_center; }
    float GetBoundingRadius() const { return m_boundingRadius; }

private:
    void CalculateBounds(const std::vector<Vertex3D>& vertices);

    std::vector<SubMesh> m_subMeshes;
    std::vector<Material3D> m_materials;

    glm::vec3 m_center = {0, 0, 0};
    float m_boundingRadius = 1.0f;
};

} // namespace CarHMI::Render3D
