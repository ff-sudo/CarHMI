#include "image_view_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/focus_manager.h>
#include <gui/widgets/image_view.h>
#include <gui/widgets/button.h>
#include <gui/widgets/label.h>
#include <gui/layout/box_layout.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <vector>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;

struct ImageViewDemoScene::Impl {
    BoxLayout* root = nullptr;
    ImageView* imageView = nullptr;
    Label* modeLabel = nullptr;
    RHI::Texture2D checkerTex;
    ImageView::ScaleMode currentMode = ImageView::ScaleMode::Fit;

    void GenerateCheckerTexture() {
        const int size = 128;
        const int cellCount = 8;
        const int cellPixels = size / cellCount;
        std::vector<unsigned char> pixels(size * size * 4);
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                int cellX = x / cellPixels;
                int cellY = y / cellPixels;
                bool white = (cellX + cellY) % 2 == 0;
                int idx = (y * size + x) * 4;
                pixels[idx + 0] = white ? 220 : 60;
                pixels[idx + 1] = white ? 220 : 60;
                pixels[idx + 2] = white ? 240 : 80;
                pixels[idx + 3] = 255;
            }
        }
        checkerTex.LoadFromMemory(pixels.data(), size, size, 4);
    }
};

void ImageViewDemoScene::OnEnter() {
    m = new Impl();
    m->GenerateCheckerTexture();

    m->root = new BoxLayout(1, {30, 30}, {700, 500}, BoxDirection::Vertical, 15, 10);
    m->root->SetDrawBackgroundFromTheme(true);

    auto* title = new Label(2, {0, 0}, "ImageView Demo", Label::Role::Title);
    title->SetSize({650, 30});

    // ImageView - center in available space
    m->imageView = new ImageView(3, {50, 0}, {600, 300});
    m->imageView->SetTexture(&m->checkerTex);
    m->imageView->SetScaleMode(m->currentMode);

    m->modeLabel = new Label(4, {0, 0}, "Mode: Fit");
    m->modeLabel->SetSize({650, 22});

    // Control buttons row
    auto* btnRow = new HBoxLayout(10, {0, 0}, {650, 40}, 0, 8);

    auto* fitBtn = new Button(11, {0, 0}, {120, 40}, "Fit");
    fitBtn->SetOnClick([this]() {
        m->currentMode = ImageView::ScaleMode::Fit;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Fit");
    });

    auto* fillBtn = new Button(12, {0, 0}, {120, 40}, "Fill");
    fillBtn->SetOnClick([this]() {
        m->currentMode = ImageView::ScaleMode::Fill;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Fill");
    });

    auto* coverBtn = new Button(13, {0, 0}, {120, 40}, "Cover");
    coverBtn->SetOnClick([this]() {
        m->currentMode = ImageView::ScaleMode::Cover;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Cover");
    });

    auto* backBtn = new Button(99, {0, 0}, {150, 40}, "< Back");
    backBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Pop();
    });

    btnRow->AddChild(fitBtn);
    btnRow->AddChild(fillBtn);
    btnRow->AddChild(coverBtn);
    btnRow->AddChild(backBtn);

    m->root->AddChild(title);
    m->root->AddChild(m->imageView);
    m->root->AddChild(m->modeLabel);
    m->root->AddChild(btnRow);
    m->root->SubscribeThemeChange();
    FocusManager::Get().ClearAll();
    m->root->RegisterFocusRecursive();
}

void ImageViewDemoScene::OnExit() {
    delete m->root;
    delete m;
    m = nullptr;
}

void ImageViewDemoScene::OnUpdate(float dt) {
    auto& ctx = Application::Get().GetUIContext();
    if (m && m->root) m->root->Update(ctx);
}

void ImageViewDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    app.GetRenderer().Begin(proj);
    auto& ctx = app.GetUIContext();
    if (m && m->root) m->root->Draw(ctx);
    app.GetRenderer().End();
}

void ImageViewDemoScene::OnImGui() {
    ImGui::Begin("ImageView Debug");
    ImGui::Text("Texture: %dx%d", m->checkerTex.GetWidth(), m->checkerTex.GetHeight());
    if (ImGui::Button("Fit")) {
        m->currentMode = ImageView::ScaleMode::Fit;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Fit");
    }
    ImGui::SameLine();
    if (ImGui::Button("Fill")) {
        m->currentMode = ImageView::ScaleMode::Fill;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Fill");
    }
    ImGui::SameLine();
    if (ImGui::Button("Cover")) {
        m->currentMode = ImageView::ScaleMode::Cover;
        m->imageView->SetScaleMode(m->currentMode);
        m->modeLabel->SetText("Mode: Cover");
    }
    ImGui::End();
}

} // namespace CarHMI::Gallery
