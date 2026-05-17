#include "batch_renderer2d.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <cstdio>

namespace CarHMI {

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

    // index buffer: 0,1,2, 2,3,0 pattern
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

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    // texcoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    // texindex
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texIndex));

    glBindVertexArray(0);

    // set texture sampler uniforms
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
    if (m_indexCount >= MaxIndices)
        Flush();

    float texIdx = 0.0f; // white texture

    m_vertexPtr->position = pos;
    m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0.0f, 0.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y};
    m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {1.0f, 0.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y};
    m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {1.0f, 1.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x, pos.y + size.y};
    m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0.0f, 1.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawQuad(glm::vec2 pos, glm::vec2 size, Texture2D& texture, const glm::vec4& tint) {
    if (m_indexCount >= MaxIndices)
        Flush();

    float texIdx = FindOrAddTexture(texture.GetID());

    m_vertexPtr->position = pos;
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {0.0f, 0.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {1.0f, 0.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {1.0f, 1.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x, pos.y + size.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {0.0f, 1.0f};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

void BatchRenderer2D::DrawRotatedQuad(glm::vec2 pos, glm::vec2 size, float angleDeg, const glm::vec4& color) {
    if (m_indexCount >= MaxIndices)
        Flush();

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
        pos,
        {pos.x + size.x, pos.y},
        {pos.x + size.x, pos.y + size.y},
        {pos.x, pos.y + size.y}
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
    if (m_indexCount >= MaxIndices)
        Flush();

    float texIdx = FindOrAddTexture(texture.GetID());

    m_vertexPtr->position = pos;
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMin.x, uvMin.y};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMax.x, uvMin.y};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x + size.x, pos.y + size.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMax.x, uvMax.y};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

    m_vertexPtr->position = {pos.x, pos.y + size.y};
    m_vertexPtr->color = tint;
    m_vertexPtr->texCoord = {uvMin.x, uvMax.y};
    m_vertexPtr->texIndex = texIdx;
    m_vertexPtr++;

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

        // degenerate quad: two triangles sharing center
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
    m_vertexPtr->position = to + offset;   m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;
    m_vertexPtr->position = to - offset;   m_vertexPtr->color = color;
    m_vertexPtr->texCoord = {0, 0}; m_vertexPtr->texIndex = texIdx; m_vertexPtr++;

    m_indexCount += 6;
    m_stats.quadCount++;
}

} // namespace CarHMI
