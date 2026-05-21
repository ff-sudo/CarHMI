#include <rhi/batch_renderer2d.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <cmath>

namespace CarHMI::RHI {

BatchRenderer2D::BatchRenderer2D() = default;

BatchRenderer2D::~BatchRenderer2D() {
    Shutdown();
}

bool BatchRenderer2D::Init() {
    if (!m_shader.LoadFromFile("resources/shaders/batch2d.vert", "resources/shaders/batch2d.frag")) {
        printf("BatchRenderer2D: failed to load shaders\n");
        return false;
    }

    m_whiteTexture.CreateWhitePixel();
    m_textureSlots[0] = m_whiteTexture.GetID();

    m_vertexBuffer = new Vertex[MaxVertices];

    auto* indices = new uint32_t[MaxIndices];
    uint32_t offset = 0;
    for (int i = 0; i < MaxIndices; i += 6) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    delete[] indices;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texIndex));

    glBindVertexArray(0);

    m_shader.Bind();
    int samplers[MaxTextureSlots];
    for (int i = 0; i < MaxTextureSlots; i++) samplers[i] = i;
    m_shader.SetIntArray("u_Textures", samplers, MaxTextureSlots);

    return true;
}

void BatchRenderer2D::Shutdown() {
    delete[] m_vertexBuffer;
    m_vertexBuffer = nullptr;
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
}

void BatchRenderer2D::Begin(const glm::mat4& projection) {
    m_shader.Bind();
    m_shader.SetMat4("u_Projection", projection);

    m_vertexPtr = m_vertexBuffer;
    m_indexCount = 0;
    m_textureSlotIndex = 1;
    m_stats = {};
}

void BatchRenderer2D::End() {
    Flush();
}

void BatchRenderer2D::Flush() {
    if (m_indexCount == 0) return;

    uint32_t dataSize = (uint32_t)((uint8_t*)m_vertexPtr - (uint8_t*)m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_vertexBuffer);

    for (int i = 0; i < m_textureSlotIndex; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureSlots[i]);
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    m_stats.drawCalls++;
    m_stats.textureSlotUsed = m_textureSlotIndex;

    m_vertexPtr = m_vertexBuffer;
    m_indexCount = 0;
    m_textureSlotIndex = 1;
}

float BatchRenderer2D::FindOrAddTexture(GLuint textureID) {
    for (int i = 0; i < m_textureSlotIndex; i++) {
        if (m_textureSlots[i] == textureID)
            return (float)i;
    }

    if (m_textureSlotIndex >= MaxTextureSlots)
        Flush();

    m_textureSlots[m_textureSlotIndex] = textureID;
    return (float)m_textureSlotIndex++;
}

void BatchRenderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, const glm::vec4& color) {
    if (m_indexCount >= MaxIndices) Flush();

    float texIdx = 0.0f;

    m_vertexPtr->position = pos; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0.0f, 0.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y}; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {1.0f, 0.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y}; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {1.0f, 1.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x, pos.y + size.y}; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0.0f, 1.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, Texture2D& texture, const glm::vec4& tint) {
    if (m_indexCount >= MaxIndices) Flush();

    float texIdx = FindOrAddTexture(texture.GetID());

    m_vertexPtr->position = pos; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {0.0f, 0.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {1.0f, 0.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {1.0f, 1.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x, pos.y + size.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {0.0f, 1.0f}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawRotatedQuad(glm::vec2 pos, glm::vec2 size, float angleDeg, const glm::vec4& color) {
    if (m_indexCount >= MaxIndices) Flush();

    float texIdx = 0.0f;
    glm::vec2 center = pos + size * 0.5f;

    float rad = glm::radians(angleDeg);
    float c = cosf(rad);
    float s = sinf(rad);

    auto rotate = [&](glm::vec2 p) -> glm::vec2 {
        p -= center;
        return glm::vec2(p.x * c - p.y * s, p.x * s + p.y * c) + center;
    };

    glm::vec2 corners[4] = {
        pos, {pos.x + size.x, pos.y},
        {pos.x + size.x, pos.y + size.y}, {pos.x, pos.y + size.y}
    };
    glm::vec2 uvs[4] = {{0,0},{1,0},{1,1},{0,1}};

    for (int i = 0; i < 4; i++) {
        m_vertexPtr->position = rotate(corners[i]);
        m_vertexPtr->color = color;
        m_vertexPtr->texCoord = uvs[i];
        m_vertexPtr->texIndex = texIdx;
        m_vertexPtr++;
    }

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawSubTexturedQuad(glm::vec2 pos, glm::vec2 size, Texture2D& texture,
                                           glm::vec2 uvMin, glm::vec2 uvMax, const glm::vec4& tint) {
    if (m_indexCount >= MaxIndices) Flush();

    float texIdx = FindOrAddTexture(texture.GetID());

    m_vertexPtr->position = pos; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMin.x, uvMin.y}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMax.x, uvMin.y}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMax.x, uvMax.y}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = {pos.x, pos.y + size.y}; m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMin.x, uvMax.y}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawCircle(glm::vec2 center, float radius, const glm::vec4& color, int segments) {
    float texIdx = 0.0f;
    float step = 2.0f * 3.14159265f / (float)segments;

    for (int i = 0; i < segments; i++) {
        if (m_indexCount + 6 > MaxIndices) Flush();

        float a0 = step * i;
        float a1 = step * (i + 1);

        glm::vec2 p0 = center;
        glm::vec2 p1 = center + glm::vec2(cosf(a0), sinf(a0)) * radius;
        glm::vec2 p2 = center + glm::vec2(cosf(a1), sinf(a1)) * radius;

        m_vertexPtr->position = p0; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = p1; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = p2; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = p0; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

        m_indexCount += 6;
        m_stats.quadCount++;
    }
}

void BatchRenderer2D::DrawRing(glm::vec2 center, float innerRadius, float outerRadius,
                                const glm::vec4& color, int segments) {
    DrawArc(center, innerRadius, outerRadius, 0.0f, 360.0f, color, segments);
}

void BatchRenderer2D::DrawArc(glm::vec2 center, float innerRadius, float outerRadius,
                               float startAngle, float endAngle, const glm::vec4& color, int segments) {
    float texIdx = 0.0f;
    float startRad = glm::radians(startAngle);
    float endRad = glm::radians(endAngle);
    float range = endRad - startRad;
    float step = range / (float)segments;

    for (int i = 0; i < segments; i++) {
        if (m_indexCount + 6 > MaxIndices) Flush();

        float a0 = startRad + step * i;
        float a1 = startRad + step * (i + 1);

        float c0 = cosf(a0), s0 = sinf(a0);
        float c1 = cosf(a1), s1 = sinf(a1);

        glm::vec2 outer0 = center + glm::vec2(c0, s0) * outerRadius;
        glm::vec2 outer1 = center + glm::vec2(c1, s1) * outerRadius;
        glm::vec2 inner0 = center + glm::vec2(c0, s0) * innerRadius;
        glm::vec2 inner1 = center + glm::vec2(c1, s1) * innerRadius;

        m_vertexPtr->position = inner0; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = outer0; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = outer1; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = inner1; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

        m_indexCount += 6;
        m_stats.quadCount++;
    }
}

void BatchRenderer2D::DrawLine(glm::vec2 from, glm::vec2 to, float thickness, const glm::vec4& color) {
    if (m_indexCount + 6 > MaxIndices) Flush();

    glm::vec2 dir = to - from;
    float len = glm::length(dir);
    if (len < 0.001f) return;
    dir /= len;

    glm::vec2 normal = {-dir.y, dir.x};
    glm::vec2 offset = normal * (thickness * 0.5f);

    float texIdx = 0.0f;

    m_vertexPtr->position = from - offset; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = from + offset; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = to + offset; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = to - offset; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawTriangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, const glm::vec4& color) {
    if (m_indexCount + 6 > MaxIndices) Flush();

    float texIdx = 0.0f;

    // Degenerate quad: v0=a, v1=b, v2=c, v3=a
    // Index pattern (0,1,2,2,3,0) → triangles (a,b,c) + (c,a,a) which is degenerate/invisible
    m_vertexPtr->position = a; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = b; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = c; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = a; m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawRoundedRect(glm::vec2 pos, glm::vec2 size, float radius, const glm::vec4& color, int cornerSegments) {
    // Clamp radius to half of the smallest dimension
    float maxRadius = std::min(size.x, size.y) * 0.5f;
    radius = std::min(radius, maxRadius);

    if (radius < 0.5f) {
        DrawQuad(pos, size, color);
        return;
    }

    float texIdx = 0.0f;
    float r = radius;

    // Inner rect corners (the rectangle excluding the rounded corners)
    float left = pos.x + r;
    float right = pos.x + size.x - r;
    float top = pos.y + r;
    float bottom = pos.y + size.y - r;

    // Center quad
    {
        if (m_indexCount + 6 > MaxIndices) Flush();
        m_vertexPtr->position = {left, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {left, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_indexCount += 6; m_stats.quadCount++;
    }

    // Top edge
    {
        if (m_indexCount + 6 > MaxIndices) Flush();
        m_vertexPtr->position = {left, pos.y}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, pos.y}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {left, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_indexCount += 6; m_stats.quadCount++;
    }

    // Bottom edge
    {
        if (m_indexCount + 6 > MaxIndices) Flush();
        m_vertexPtr->position = {left, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, pos.y + size.y}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {left, pos.y + size.y}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_indexCount += 6; m_stats.quadCount++;
    }

    // Left edge
    {
        if (m_indexCount + 6 > MaxIndices) Flush();
        m_vertexPtr->position = {pos.x, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {left, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {left, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {pos.x, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_indexCount += 6; m_stats.quadCount++;
    }

    // Right edge
    {
        if (m_indexCount + 6 > MaxIndices) Flush();
        m_vertexPtr->position = {right, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {pos.x + size.x, top}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {pos.x + size.x, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_vertexPtr->position = {right, bottom}; m_vertexPtr->color = color;
        m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
        m_indexCount += 6; m_stats.quadCount++;
    }

    // Four corner arcs (triangle fans from corner center)
    constexpr float PI = 3.14159265358979f;
    float step = (PI * 0.5f) / (float)cornerSegments;

    // Corner centers and start angles:
    // TL: center=(left,top),    startAngle=PI
    // TR: center=(right,top),   startAngle=PI*1.5
    // BR: center=(right,bottom),startAngle=0
    // BL: center=(left,bottom), startAngle=PI*0.5
    struct Corner { glm::vec2 center; float startAngle; };
    Corner corners[4] = {
        {{left, top}, PI},
        {{right, top}, PI * 1.5f},
        {{right, bottom}, 0.0f},
        {{left, bottom}, PI * 0.5f}
    };

    for (auto& corner : corners) {
        for (int i = 0; i < cornerSegments; i++) {
            if (m_indexCount + 6 > MaxIndices) Flush();

            float a0 = corner.startAngle + step * i;
            float a1 = corner.startAngle + step * (i + 1);

            glm::vec2 p0 = corner.center;
            glm::vec2 p1 = corner.center + glm::vec2(cosf(a0), sinf(a0)) * r;
            glm::vec2 p2 = corner.center + glm::vec2(cosf(a1), sinf(a1)) * r;

            // Triangle as degenerate quad (p0, p1, p2, p0)
            m_vertexPtr->position = p0; m_vertexPtr->color = color;
            m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
            m_vertexPtr->position = p1; m_vertexPtr->color = color;
            m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
            m_vertexPtr->position = p2; m_vertexPtr->color = color;
            m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
            m_vertexPtr->position = p0; m_vertexPtr->color = color;
            m_vertexPtr->texCoord = {0,0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

            m_indexCount += 6; m_stats.quadCount++;
        }
    }
}

// --- Scissor Clipping ---

void BatchRenderer2D::SetViewportSize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
}

void BatchRenderer2D::PushScissor(glm::vec2 pos, glm::vec2 size) {
    Flush();

    glm::vec4 rect(pos.x, pos.y, size.x, size.y);

    // Intersect with current top of stack
    if (!m_scissorStack.empty()) {
        auto& top = m_scissorStack.back();
        float x1 = std::max(rect.x, top.x);
        float y1 = std::max(rect.y, top.y);
        float x2 = std::min(rect.x + rect.z, top.x + top.z);
        float y2 = std::min(rect.y + rect.w, top.y + top.w);
        rect = glm::vec4(x1, y1, std::max(0.0f, x2 - x1), std::max(0.0f, y2 - y1));
    }

    m_scissorStack.push_back(rect);
    ApplyScissor();
}

void BatchRenderer2D::PopScissor() {
    Flush();

    if (!m_scissorStack.empty()) {
        m_scissorStack.pop_back();
    }

    if (m_scissorStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        ApplyScissor();
    }
}

void BatchRenderer2D::ApplyScissor() {
    if (m_scissorStack.empty()) {
        glDisable(GL_SCISSOR_TEST);
        return;
    }

    auto& rect = m_scissorStack.back();
    // Convert from top-left-origin to OpenGL bottom-left-origin
    int sx = (int)rect.x;
    int sy = m_viewportHeight - (int)(rect.y + rect.w);
    int sw = (int)rect.z;
    int sh = (int)rect.w;

    glEnable(GL_SCISSOR_TEST);
    glScissor(sx, sy, sw, sh);
}

} // namespace CarHMI::RHI
