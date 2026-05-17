#include "renderer3d.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

namespace CarHMI {

bool Renderer3D::Init() {
    if (!m_shader.LoadFromFile("resources/shaders/phong.vert", "resources/shaders/phong.frag")) {
        spdlog::error("Renderer3D: failed to load shaders");
        return false;
    }
    spdlog::info("Renderer3D initialized");
    return true;
}

void Renderer3D::Shutdown() {}

void Renderer3D::Begin(Camera& camera, float aspect) {
    glEnable(GL_DEPTH_TEST);

    m_shader.Bind();

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 proj = camera.GetProjectionMatrix(aspect);
    m_viewPos = camera.GetPosition();

    m_shader.SetMat4("u_View", view);
    m_shader.SetMat4("u_Projection", proj);

    GLuint pid = m_shader.GetID();
    glUniform3fv(glGetUniformLocation(pid, "u_LightPos"), 1, glm::value_ptr(m_lightPos));
    glUniform3fv(glGetUniformLocation(pid, "u_LightColor"), 1, glm::value_ptr(m_lightColor));
    glUniform3fv(glGetUniformLocation(pid, "u_ViewPos"), 1, glm::value_ptr(m_viewPos));
}

void Renderer3D::DrawMesh(Mesh& mesh, const glm::mat4& model) {
    m_shader.SetMat4("u_Model", model);

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    GLuint pid = m_shader.GetID();
    glUniformMatrix3fv(glGetUniformLocation(pid, "u_NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

    mesh.Draw(pid);
}

void Renderer3D::End() {
    m_shader.Unbind();
    glDisable(GL_DEPTH_TEST);
}

} // namespace CarHMI
