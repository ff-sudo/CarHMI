#include "canvas_demo_scene.h"
#include "application.h"
#include <gui/canvas.h>
#include <gui/debug/debug_panel.h>
#include <core/scene/scene_manager.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace CarHMI::Gallery {

using namespace CarHMI::GUI;

void CanvasDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    auto& renderer = app.GetRenderer();

    renderer.SetViewportSize(winSize.x, winSize.y);
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    renderer.Begin(proj);

    glm::vec4 fill = {m_fillColor[0], m_fillColor[1], m_fillColor[2], m_fillColor[3]};
    glm::vec4 stroke = {m_strokeColor[0], m_strokeColor[1], m_strokeColor[2], m_strokeColor[3]};

    Canvas cv(renderer, app.GetFont(), {30, 30});

    cv.SetFillColor(fill);
    cv.SetStrokeColor(stroke);
    cv.SetStrokeWidth(m_strokeWidth);

    // --- Row 1: Rectangles ---
    cv.Save();
    cv.FillRect(0, 0, 100, 60);
    cv.StrokeRect(120, 0, 100, 60);
    cv.FillRoundRect(240, 0, 120, 60, m_roundRadius);
    cv.Restore();

    // --- Row 2: Circles & Arcs ---
    cv.Save();
    cv.Translate(0, 90);
    cv.FillCircle(40, 40, 35, m_segments);
    cv.StrokeCircle(140, 40, 35, m_segments);
    cv.FillArc(260, 40, 35, m_arcStart, m_arcEnd, m_segments);
    cv.StrokeArc(370, 40, 35, m_arcStart, m_arcEnd, m_segments);
    cv.Restore();

    // --- Row 3: Lines & Polyline ---
    cv.Save();
    cv.Translate(0, 180);
    cv.DrawLine(0, 30, 100, 0);
    cv.DrawLine(0, 0, 100, 30);

    std::vector<glm::vec2> poly = {{130, 0}, {180, 10}, {200, 40}, {160, 60}, {120, 40}};
    cv.DrawPolyline(poly);
    cv.Restore();

    // --- Row 4: Path Fill & Stroke ---
    cv.Save();
    cv.Translate(0, 270);

    // Filled triangle via path
    cv.BeginPath();
    cv.MoveTo(0, 60);
    cv.LineTo(50, 0);
    cv.LineTo(100, 60);
    cv.ClosePath();
    cv.FillPath();

    // Filled pentagon
    cv.BeginPath();
    for (int i = 0; i < 5; i++) {
        float angle = -90.0f + 72.0f * i;
        float rad = angle * 3.14159f / 180.0f;
        float px = 180 + 35 * cosf(rad);
        float py = 30 + 35 * sinf(rad);
        if (i == 0) cv.MoveTo(px, py);
        else cv.LineTo(px, py);
    }
    cv.ClosePath();
    cv.FillPath();

    // Stroked star
    cv.BeginPath();
    for (int i = 0; i < 10; i++) {
        float angle = -90.0f + 36.0f * i;
        float rad = angle * 3.14159f / 180.0f;
        float r = (i % 2 == 0) ? 35.0f : 15.0f;
        float px = 300 + r * cosf(rad);
        float py = 30 + r * sinf(rad);
        if (i == 0) cv.MoveTo(px, py);
        else cv.LineTo(px, py);
    }
    cv.ClosePath();
    cv.StrokePath();

    cv.Restore();

    // --- Row 5: Text + ClipRect ---
    cv.Save();
    cv.Translate(0, 360);
    cv.DrawText("Canvas API v1", 0, 0);

    // Clip demo: draw a circle but clip to a rect
    cv.ClipRect(150, 0, 80, 40);
    cv.FillCircle(190, 20, 30, m_segments);
    cv.ResetClip();

    cv.Restore();

    // --- Row 6: Save/Restore + Translate demo ---
    cv.Save();
    cv.Translate(0, 420);
    for (int i = 0; i < 5; i++) {
        cv.Save();
        cv.Translate(i * 60.0f, 0);
        float alpha = 1.0f - i * 0.15f;
        cv.SetFillColor({fill.r, fill.g, fill.b, alpha});
        cv.FillRoundRect(0, 0, 50, 40, 8);
        cv.Restore();
    }
    cv.Restore();

    renderer.End();
}

void CanvasDemoScene::OnImGui() {
    ImGui::Begin("Canvas Demo");

    auto& stats = Application::Get().GetRenderer().GetStats();
    Debug::ShowRenderStats(stats);
    ImGui::Separator();

    if (ImGui::Button("< Back")) {
        Application::Get().GetSceneManager().Pop();
    }
    ImGui::Separator();

    ImGui::ColorEdit4("Fill Color", m_fillColor);
    ImGui::ColorEdit4("Stroke Color", m_strokeColor);
    ImGui::SliderFloat("Stroke Width", &m_strokeWidth, 0.5f, 10.0f);
    ImGui::SliderFloat("Round Radius", &m_roundRadius, 0.0f, 30.0f);
    ImGui::SliderFloat("Arc Start", &m_arcStart, 0.0f, 360.0f);
    ImGui::SliderFloat("Arc End", &m_arcEnd, 0.0f, 360.0f);
    ImGui::SliderInt("Segments", &m_segments, 4, 64);

    ImGui::End();
}

} // namespace CarHMI::Gallery
