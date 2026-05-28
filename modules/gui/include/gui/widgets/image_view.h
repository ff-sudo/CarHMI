#pragma once

#include <gui/widget.h>
#include <gui/style/theme_manager.h>
#include <rhi/texture.h>
#include <string>
#include <functional>

namespace CarHMI::GUI {

class ImageView : public Widget {
public:
    enum class ScaleMode { Fit, Fill, Cover };

    ImageView(int id, glm::vec2 pos, glm::vec2 size)
        : Widget(id, pos, size) {
        m_bgColor = ThemeManager::Get().GetTheme().panel.backgroundColor;
    }

    void ApplyTheme() override {
        if (!m_hasCustomBg)
            m_bgColor = ThemeManager::Get().GetTheme().panel.backgroundColor;
    }

    // --- Texture（外部管理生命周期）---
    void SetTexture(RHI::Texture2D* texture) { m_texture = texture; }
    RHI::Texture2D* GetTexture() const { return m_texture; }

    // --- 缩放模式 ---
    void SetScaleMode(ScaleMode mode) { m_scaleMode = mode; }
    ScaleMode GetScaleMode() const { return m_scaleMode; }

    // --- 颜色调整 ---
    void SetTint(const glm::vec4& tint) { m_tint = tint; }
    void SetBgColor(const glm::vec4& color) { m_bgColor = color; m_hasCustomBg = true; }

    void Draw(UIContext& ctx) override {
        if (!m_visible) return;
        glm::vec2 abs = GetAbsolutePos();
        auto& renderer = ctx.GetRenderer();

        // Background (visible through transparent areas of texture)
        renderer.DrawQuad(abs, m_size, m_bgColor);

        if (!m_texture || m_texture->GetID() == 0) {
            // No texture loaded: draw placeholder
            glm::vec4 placeholderColor = {0.2f, 0.2f, 0.25f, 1.0f};
            renderer.DrawQuad({abs.x + 2, abs.y + 2}, {m_size.x - 4, m_size.y - 4}, placeholderColor);
            Widget::Draw(ctx);
            return;
        }

        float texW = (float)m_texture->GetWidth();
        float texH = (float)m_texture->GetHeight();
        if (texW <= 0 || texH <= 0) return;

        // Calculate draw region based on scale mode
        glm::vec2 drawPos = abs;
        glm::vec2 drawSize = m_size;

        switch (m_scaleMode) {
        case ScaleMode::Fit: {
            // Maintain aspect ratio, fit within bounds
            float scaleX = m_size.x / texW;
            float scaleY = m_size.y / texH;
            float scale = std::min(scaleX, scaleY);
            drawSize = {texW * scale, texH * scale};
            drawPos.x += (m_size.x - drawSize.x) * 0.5f;
            drawPos.y += (m_size.y - drawSize.y) * 0.5f;
            break;
        }
        case ScaleMode::Fill:
            // Stretch to fill (default)
            break;
        case ScaleMode::Cover: {
            // Maintain aspect ratio, cover entire bounds (crop)
            float scaleX = m_size.x / texW;
            float scaleY = m_size.y / texH;
            float scale = std::max(scaleX, scaleY);
            drawSize = {texW * scale, texH * scale};
            drawPos.x -= (drawSize.x - m_size.x) * 0.5f;
            drawPos.y -= (drawSize.y - m_size.y) * 0.5f;
            break;
        }
        }

        renderer.DrawQuad(drawPos, drawSize, *m_texture, m_tint);
        Widget::Draw(ctx);
    }

private:
    RHI::Texture2D* m_texture = nullptr;
    ScaleMode m_scaleMode = ScaleMode::Fill;
    glm::vec4 m_tint = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 m_bgColor;
    bool m_hasCustomBg = false;
};

} // namespace CarHMI::GUI
