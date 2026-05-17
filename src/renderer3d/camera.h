#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CarHMI {

class Camera {
public:
    Camera(float distance = 50.0f, float yaw = -90.0f, float pitch = 20.0f)
        : m_distance(distance), m_yaw(yaw), m_pitch(pitch) {
        UpdateVectors();
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(m_position, m_target, m_up);
    }

    glm::mat4 GetProjectionMatrix(float aspect) const {
        return glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
    }

    void Orbit(float deltaYaw, float deltaPitch) {
        m_yaw += deltaYaw;
        m_pitch += deltaPitch;
        m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
        UpdateVectors();
    }

    void Zoom(float delta) {
        m_distance -= delta * m_distance * 0.1f;
        m_distance = glm::clamp(m_distance, 2.0f, 500.0f);
        UpdateVectors();
    }

    void Pan(float deltaX, float deltaY) {
        glm::vec3 right = glm::normalize(glm::cross(m_target - m_position, m_up));
        glm::vec3 up = glm::normalize(glm::cross(right, m_target - m_position));
        float scale = m_distance * 0.002f;
        m_target += right * (-deltaX * scale) + up * (deltaY * scale);
        UpdateVectors();
    }

    void SetTarget(const glm::vec3& target) { m_target = target; UpdateVectors(); }
    void SetDistance(float d) { m_distance = d; UpdateVectors(); }

    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetTarget() const { return m_target; }
    float GetDistance() const { return m_distance; }
    float GetYaw() const { return m_yaw; }
    float GetPitch() const { return m_pitch; }

private:
    void UpdateVectors() {
        float yawRad = glm::radians(m_yaw);
        float pitchRad = glm::radians(m_pitch);

        glm::vec3 offset;
        offset.x = cosf(pitchRad) * cosf(yawRad);
        offset.y = sinf(pitchRad);
        offset.z = cosf(pitchRad) * sinf(yawRad);

        m_position = m_target + offset * m_distance;
    }

    glm::vec3 m_position = {0, 0, 0};
    glm::vec3 m_target = {0, 5, 0};
    glm::vec3 m_up = {0, 1, 0};

    float m_distance;
    float m_yaw;
    float m_pitch;
    float m_fov = 45.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
};

} // namespace CarHMI
