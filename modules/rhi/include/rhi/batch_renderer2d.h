#pragma once

#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>

namespace CarHMI::RHI {

class BatchRenderer2D {
public:
    static constexpr int MaxQuads = 10000;
    static constexpr int MaxVertices = MaxQuads * 4;
    static constexpr int MaxIndices = MaxQuads * 6;
    static constexpr int MaxTextureSlots = 16;

    struct Stats {
        int drawCalls = 0;
        int quadCount = 0;
        int textureSlotUsed = 0;
    };

    BatchRenderer2D();
    ~BatchRenderer2D();

    bool Init();
    void Shutdown();

    void Begin(const glm::mat4& projection);
    void End();

    void DrawQuad(glm::vec2 pos, glm::vec2 size, const glm::vec4& color);
    void DrawQuad(glm::vec2 pos, glm::vec2 size, Texture2D& texture, const glm::vec4& tint = glm::vec4(1.0f));
    void DrawRotatedQuad(glm::vec2 pos, glm::vec2 size, float angleDeg, const glm::vec4& color);
    void DrawSubTexturedQuad(glm::vec2 pos, glm::vec2 size, Texture2D& texture,
                             glm::vec2 uvMin, glm::vec2 uvMax, const glm::vec4& tint = glm::vec4(1.0f));

    void DrawCircle(glm::vec2 center, float radius, const glm::vec4& color, int segments = 32);
    void DrawRing(glm::vec2 center, float innerRadius, float outerRadius,
                  const glm::vec4& color, int segments = 32);
    void DrawArc(glm::vec2 center, float innerRadius, float outerRadius,
                 float startAngle, float endAngle, const glm::vec4& color, int segments = 32);
    void DrawLine(glm::vec2 from, glm::vec2 to, float thickness, const glm::vec4& color);

    void DrawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, const glm::vec4& color);

    void DrawRoundedRect(glm::vec2 pos, glm::vec2 size, float radius, const glm::vec4& color, int cornerSegments = 8);

    // Scissor clipping stack
    void SetViewportSize(int width, int height);
    void PushScissor(glm::vec2 pos, glm::vec2 size);
    void PopScissor();

    const Stats& GetStats() const { return m_stats; }

private:
    struct Vertex {
        glm::vec2 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        float texIndex;
    };

    void Flush();
    float FindOrAddTexture(GLuint textureID);

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    Shader m_shader;
    Texture2D m_whiteTexture;

    Vertex* m_vertexBuffer = nullptr;
    Vertex* m_vertexPtr = nullptr;
    uint32_t m_indexCount = 0;

    std::array<GLuint, MaxTextureSlots> m_textureSlots{};
    int m_textureSlotIndex = 1;

    Stats m_stats;

    // Scissor stack
    std::vector<glm::vec4> m_scissorStack; // each entry: (x, y, w, h) in screen coords
    int m_viewportWidth = 1280;
    int m_viewportHeight = 720;
    void ApplyScissor();
};

} // namespace CarHMI::RHI
