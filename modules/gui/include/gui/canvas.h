#pragma once

#include <rhi/batch_renderer2d.h>
#include <rhi/font.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <algorithm>

namespace CarHMI::GUI {

class Canvas {
public:
    Canvas(RHI::BatchRenderer2D& renderer, RHI::Font& font, glm::vec2 origin = {0, 0})
        : m_renderer(renderer), m_font(font), m_origin(origin) {
        m_state.offset = {0, 0};
        m_state.fillColor = {1, 1, 1, 1};
        m_state.strokeColor = {1, 1, 1, 1};
        m_state.strokeWidth = 1.0f;
    }

    // --- State Stack ---

    void Save() { m_stateStack.push_back(m_state); }

    void Restore() {
        if (!m_stateStack.empty()) {
            m_state = m_stateStack.back();
            m_stateStack.pop_back();
        }
    }

    // --- Transform (translate only in v1) ---

    void Translate(float x, float y) { m_state.offset += glm::vec2(x, y); }

    // --- Style ---

    void SetFillColor(const glm::vec4& c) { m_state.fillColor = c; }
    void SetStrokeColor(const glm::vec4& c) { m_state.strokeColor = c; }
    void SetStrokeWidth(float w) { m_state.strokeWidth = w; }

    // --- Rectangles ---

    void FillRect(float x, float y, float w, float h) {
        m_renderer.DrawQuad(P(x, y), {w, h}, m_state.fillColor);
    }

    void StrokeRect(float x, float y, float w, float h) {
        float sw = m_state.strokeWidth;
        auto& c = m_state.strokeColor;
        auto p = P(x, y);
        // Top
        m_renderer.DrawQuad(p, {w, sw}, c);
        // Bottom
        m_renderer.DrawQuad({p.x, p.y + h - sw}, {w, sw}, c);
        // Left
        m_renderer.DrawQuad({p.x, p.y + sw}, {sw, h - 2 * sw}, c);
        // Right
        m_renderer.DrawQuad({p.x + w - sw, p.y + sw}, {sw, h - 2 * sw}, c);
    }

    void FillRoundRect(float x, float y, float w, float h, float radius) {
        m_renderer.DrawRoundedRect(P(x, y), {w, h}, radius, m_state.fillColor);
    }

    // --- Circles ---

    void FillCircle(float cx, float cy, float radius, int segments = 32) {
        m_renderer.DrawCircle(P(cx, cy), radius, m_state.fillColor, segments);
    }

    void StrokeCircle(float cx, float cy, float radius, int segments = 32) {
        float inner = radius - m_state.strokeWidth;
        if (inner < 0) inner = 0;
        m_renderer.DrawRing(P(cx, cy), inner, radius, m_state.strokeColor, segments);
    }

    // --- Arcs ---

    void FillArc(float cx, float cy, float radius, float startDeg, float endDeg, int segments = 32) {
        m_renderer.DrawArc(P(cx, cy), 0, radius, startDeg, endDeg, m_state.fillColor, segments);
    }

    void StrokeArc(float cx, float cy, float radius, float startDeg, float endDeg, int segments = 32) {
        float inner = radius - m_state.strokeWidth;
        if (inner < 0) inner = 0;
        m_renderer.DrawArc(P(cx, cy), inner, radius, startDeg, endDeg, m_state.strokeColor, segments);
    }

    // --- Lines ---

    void DrawLine(float x1, float y1, float x2, float y2) {
        m_renderer.DrawLine(P(x1, y1), P(x2, y2), m_state.strokeWidth, m_state.strokeColor);
    }

    void DrawPolyline(const std::vector<glm::vec2>& points) {
        for (size_t i = 0; i + 1 < points.size(); i++) {
            m_renderer.DrawLine(P(points[i].x, points[i].y),
                                P(points[i+1].x, points[i+1].y),
                                m_state.strokeWidth, m_state.strokeColor);
        }
    }

    // --- Path (polyline-based, no bezier in v1) ---

    void BeginPath() {
        m_pathPoints.clear();
        m_pathStarted = false;
    }

    void MoveTo(float x, float y) {
        if (!m_pathStarted) {
            m_pathPoints.clear();
            m_pathStarted = true;
        }
        m_pathPoints.push_back({x, y});
    }

    void LineTo(float x, float y) {
        m_pathPoints.push_back({x, y});
    }

    void ClosePath() {
        if (m_pathPoints.size() >= 2) {
            m_pathPoints.push_back(m_pathPoints[0]);
        }
    }

    void StrokePath() {
        for (size_t i = 0; i + 1 < m_pathPoints.size(); i++) {
            auto& a = m_pathPoints[i];
            auto& b = m_pathPoints[i + 1];
            m_renderer.DrawLine(P(a.x, a.y), P(b.x, b.y),
                                m_state.strokeWidth, m_state.strokeColor);
        }
    }

    void FillPath() {
        // Ear-clipping triangulation for arbitrary simple polygons
        if (m_pathPoints.size() < 3) return;

        // Build index list, remove closing duplicate if present
        std::vector<glm::vec2> poly(m_pathPoints);
        if (poly.size() >= 2 && glm::length(poly.front() - poly.back()) < 0.001f)
            poly.pop_back();
        if (poly.size() < 3) return;

        // Ensure CCW winding
        float signedArea = 0;
        for (size_t i = 0; i < poly.size(); i++) {
            auto& a = poly[i];
            auto& b = poly[(i + 1) % poly.size()];
            signedArea += (b.x - a.x) * (b.y + a.y);
        }
        if (signedArea > 0)
            std::reverse(poly.begin(), poly.end());

        // Build index list
        std::vector<int> indices(poly.size());
        for (size_t i = 0; i < poly.size(); i++) indices[i] = (int)i;

        while (indices.size() > 2) {
            bool earFound = false;
            size_t n = indices.size();
            for (size_t i = 0; i < n; i++) {
                size_t prev = (i + n - 1) % n;
                size_t next = (i + 1) % n;
                auto& A = poly[indices[prev]];
                auto& B = poly[indices[i]];
                auto& C = poly[indices[next]];

                // Check convex (CCW cross product > 0)
                float cross = (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
                if (cross <= 0) continue;

                // Check no other vertex inside triangle
                bool hasInside = false;
                for (size_t j = 0; j < n; j++) {
                    if (j == prev || j == i || j == next) continue;
                    if (PointInTriangle(poly[indices[j]], A, B, C)) {
                        hasInside = true;
                        break;
                    }
                }
                if (hasInside) continue;

                // Emit triangle
                m_renderer.DrawTriangle(P(A.x, A.y), P(B.x, B.y), P(C.x, C.y), m_state.fillColor);
                indices.erase(indices.begin() + (int)i);
                earFound = true;
                break;
            }
            if (!earFound) break; // degenerate polygon
        }
    }

    // --- Text ---

    void DrawText(const std::string& text, float x, float y) {
        auto p = P(x, y);
        m_font.DrawText(m_renderer, text, p.x, p.y, m_state.fillColor);
    }

    float MeasureText(const std::string& text) {
        return m_font.MeasureWidth(text);
    }

    // --- Clipping ---

    void ClipRect(float x, float y, float w, float h) {
        m_renderer.PushScissor(P(x, y), {w, h});
        m_clipDepth++;
    }

    void ResetClip() {
        while (m_clipDepth > 0) {
            m_renderer.PopScissor();
            m_clipDepth--;
        }
    }

private:
    // Convert local coords to absolute screen coords
    glm::vec2 P(float x, float y) const {
        return m_origin + m_state.offset + glm::vec2(x, y);
    }

    static bool PointInTriangle(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c) {
        auto cross = [](glm::vec2 o, glm::vec2 x, glm::vec2 y) {
            return (x.x - o.x) * (y.y - o.y) - (x.y - o.y) * (y.x - o.x);
        };
        float d1 = cross(p, a, b);
        float d2 = cross(p, b, c);
        float d3 = cross(p, c, a);
        bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
        return !(hasNeg && hasPos);
    }

    struct State {
        glm::vec2 offset = {0, 0};
        glm::vec4 fillColor = {1, 1, 1, 1};
        glm::vec4 strokeColor = {1, 1, 1, 1};
        float strokeWidth = 1.0f;
    };

    RHI::BatchRenderer2D& m_renderer;
    RHI::Font& m_font;
    glm::vec2 m_origin;
    State m_state;
    std::vector<State> m_stateStack;

    std::vector<glm::vec2> m_pathPoints;
    bool m_pathStarted = false;
    int m_clipDepth = 0;
};

} // namespace CarHMI::GUI
