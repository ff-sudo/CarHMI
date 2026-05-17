#pragma once

#include "camera.h"
#include "mesh.h"
#include "renderer/shader.h"
#include <glm/glm.hpp>

namespace CarHMI {

class Renderer3D {
public:
    bool Init();
    void Shutdown();

    void Begin(Camera& camera, float aspect);
    void DrawMesh(Mesh& mesh, const glm::mat4& model = glm::mat4(1.0f));
    void End();

    void SetLightPos(const glm::vec3& pos) { m_lightPos = pos; }
    void SetLightColor(const glm::vec3& color) { m_lightColor = color; }

private:
    Shader m_shader;

    glm::vec3 m_lightPos   = {50.0f, 80.0f, 50.0f};
    glm::vec3 m_lightColor = {1.0f, 1.0f, 1.0f};
    glm::vec3 m_viewPos    = {0, 0, 0};
};

} // namespace CarHMI
