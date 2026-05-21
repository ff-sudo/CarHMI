#pragma once

#include <gui/widget.h>
#include <gui/layout/box_layout.h>
#include <gui/style/theme_manager.h>
#include <gui/i18n/i18n.h>
#include <core/scene/scene_manager.h>
#include <core/animation/animation_manager.h>
#include <imgui.h>
#include <cstdio>

namespace CarHMI::GUI::Debug {

// --- Render Stats ---
inline void ShowRenderStats(const RHI::BatchRenderer2D::Stats& stats) {
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Draw Calls: %d", stats.drawCalls);
    ImGui::Text("Quads: %d", stats.quadCount);
}

// --- Theme Selector ---
inline void ShowThemeSelector() {
    ImGui::Text("Theme: %s", ThemeManager::Get().GetActiveThemeName().c_str());
    if (ImGui::Button("Dark")) ThemeManager::Get().SetActiveTheme("dark");
    ImGui::SameLine();
    if (ImGui::Button("Light")) ThemeManager::Get().SetActiveTheme("light");
}

// --- Language Selector ---
inline void ShowLanguageSelector() {
    ImGui::Text("Language: %s (%s)", I18n::Get().GetLanguageCode().c_str(),
                I18n::Get().IsRTL() ? "RTL" : "LTR");
    if (ImGui::Button("EN")) I18n::Get().SetLanguage("en");
    ImGui::SameLine();
    if (ImGui::Button("ZH")) I18n::Get().SetLanguage("zh");
    ImGui::SameLine();
    if (ImGui::Button("AR")) I18n::Get().SetLanguage("ar");
}

// --- Scene Info ---
inline void ShowSceneInfo(Core::SceneManager& sm) {
    ImGui::Text("Scene Stack: %d", sm.StackSize());
    ImGui::Text("Animations: %d", Core::AnimationManager::Get().ActiveCount());
    if (sm.IsTransitioning())
        ImGui::Text("Transition offset: %.1f", sm.GetTransitionOffset());
}

// --- Insets Editor (4 sliders for top/right/bottom/left) ---
inline bool EditInsets(const char* label, Insets& insets, float minVal = 0.0f, float maxVal = 100.0f) {
    bool changed = false;
    ImGui::PushID(label);
    ImGui::Text("%s", label);
    changed |= ImGui::SliderFloat("Top", &insets.top, minVal, maxVal);
    changed |= ImGui::SliderFloat("Right", &insets.right, minVal, maxVal);
    changed |= ImGui::SliderFloat("Bottom", &insets.bottom, minVal, maxVal);
    changed |= ImGui::SliderFloat("Left", &insets.left, minVal, maxVal);
    ImGui::PopID();
    return changed;
}

// --- Widget Editor (margin + sizePolicy + size) ---
inline bool EditWidget(const char* label, Widget* w) {
    bool changed = false;
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        // Size
        auto sz = w->GetSize();
        if (ImGui::DragFloat2("Size", &sz.x, 1.0f, 0.0f, 2000.0f)) {
            w->SetSize(sz);
            changed = true;
        }

        // Margin
        auto margin = w->GetMargin();
        if (EditInsets("Margin", margin, 0.0f, 50.0f)) {
            w->SetMargin(margin);
            changed = true;
        }

        // SizePolicy
        auto sp = w->GetSizePolicy();
        const char* modes[] = {"Fixed", "Percent", "Fill"};

        int wMode = (int)sp.widthMode;
        if (ImGui::Combo("Width Mode", &wMode, modes, 3)) {
            sp.widthMode = (SizeMode)wMode;
            changed = true;
        }
        if (sp.widthMode == SizeMode::Percent) {
            if (ImGui::SliderFloat("Width %", &sp.widthValue, 0.0f, 100.0f))
                changed = true;
        }

        int hMode = (int)sp.heightMode;
        if (ImGui::Combo("Height Mode", &hMode, modes, 3)) {
            sp.heightMode = (SizeMode)hMode;
            changed = true;
        }
        if (sp.heightMode == SizeMode::Percent) {
            if (ImGui::SliderFloat("Height %", &sp.heightValue, 0.0f, 100.0f))
                changed = true;
        }

        if (changed) w->SetSizePolicy(sp);

        ImGui::TreePop();
    }
    ImGui::PopID();
    return changed;
}

// --- BoxLayout Editor (padding + spacing + direction) ---
inline bool EditBoxLayout(const char* label, BoxLayout* layout) {
    bool changed = false;
    ImGui::PushID(label);
    if (ImGui::TreeNode(label)) {
        // Direction
        int dir = (int)layout->GetDirection();
        if (ImGui::Combo("Direction", &dir, "Vertical\0Horizontal\0")) {
            layout->SetDirection((BoxDirection)dir);
            changed = true;
        }

        // Spacing
        float spacing = layout->GetSpacing();
        if (ImGui::SliderFloat("Spacing", &spacing, 0.0f, 50.0f)) {
            layout->SetSpacing(spacing);
            changed = true;
        }

        // Padding
        auto padding = layout->GetPadding();
        if (EditInsets("Padding", padding, 0.0f, 80.0f)) {
            layout->SetPadding(padding);
            changed = true;
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
    return changed;
}

// --- Widget Tree Inspector (recursive) ---
inline void ShowWidgetTree(Widget* w, int depth = 0) {
    if (!w) return;
    char buf[128];
    auto sz = w->GetSize();
    auto pos = w->GetPos();
    snprintf(buf, sizeof(buf), "[%d] pos(%.0f,%.0f) size(%.0f,%.0f)",
             w->GetID(), pos.x, pos.y, sz.x, sz.y);

    if (w->GetChildren().empty()) {
        ImGui::BulletText("%s", buf);
    } else {
        if (ImGui::TreeNode((void*)(intptr_t)w->GetID(), "%s", buf)) {
            for (auto* child : w->GetChildren())
                ShowWidgetTree(child, depth + 1);
            ImGui::TreePop();
        }
    }
}

} // namespace CarHMI::GUI::Debug
