#include "dialog_demo_scene.h"
#include "application.h"
#include <core/scene/scene_manager.h>
#include <gui/focus_manager.h>
#include <gui/widgets/dialog.h>
#include <gui/widgets/button.h>
#include <gui/widgets/label.h>
#include <gui/layout/box_layout.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <memory>

namespace CarHMI::Gallery {

using namespace CarHMI::Core;
using namespace CarHMI::GUI;
using namespace CarHMI::RHI;

struct DialogDemoScene::Impl {
    BoxLayout* root = nullptr;
    Dialog* activeDialog = nullptr;
    Toast* activeToast = nullptr;
    Label* statusLabel = nullptr;
    BoxLayout* overlayPanel = nullptr;

    // === Safe deferred operations (all run at start of next OnUpdate) ===

    void RequestShowDialog(const std::string& title, const std::string& msg) {
        // Defer so AddChild / Recalculate don't run during Widget::Update iteration
        pendingAction = Action::ShowDialog;
        pendingTitle = title;
        pendingMsg = msg;
    }

    void RequestHideDialog() {
        if (overlayPanel) {
            overlayPanel->SetVisible(false);
            pendingAction = Action::HideDialog;
        }
    }

    void RequestShowToast(const std::string& msg, ToastType type) {
        pendingAction = Action::ShowToast;
        pendingToastMsg = msg;
        pendingToastType = type;
    }

    void Cleanup() {
        if (overlayPanel) { root->RemoveChild(overlayPanel); delete overlayPanel; overlayPanel = nullptr; }
        if (activeToast)  { root->RemoveChild(activeToast);  delete activeToast;  activeToast = nullptr; }
        activeDialog = nullptr;
    }

    void FlushPending() {
        switch (pendingAction) {
        case Action::ShowDialog:
            DoShowDialog(pendingTitle, pendingMsg);
            break;
        case Action::HideDialog:
            DoHideDialog();
            break;
        case Action::ShowToast:
            DoShowToast(pendingToastMsg, pendingToastType);
            break;
        case Action::None:
            break;
        }
        pendingAction = Action::None;
    }

private:
    void DoShowDialog(const std::string& title, const std::string& msg) {
        DoHideDialog(); // clean up any existing dialog

        overlayPanel = new BoxLayout(200, {0, 0}, {800, 700}, BoxDirection::Vertical, 0, 0);
        overlayPanel->SetDrawBackground(true, {0.0f, 0.0f, 0.0f, 0.4f});

        auto* dialog = new Dialog(201, {150, 200}, {500, 300}, title, msg);
        dialog->SetCornerRadius(10.0f);

        dialog->AddButton("Cancel", [this]() {
            RequestHideDialog();
            if (statusLabel) statusLabel->SetText("Dialog: cancelled");
        });
        dialog->AddButton("OK", [this]() {
            RequestHideDialog();
            if (statusLabel) statusLabel->SetText("Dialog: confirmed");
        });

        overlayPanel->AddChild(dialog);
        root->AddChild(overlayPanel);
        activeDialog = dialog;
    }

    void DoHideDialog() {
        if (!overlayPanel) return;
        root->RemoveChild(overlayPanel);
        delete overlayPanel;
        overlayPanel = nullptr;
        activeDialog = nullptr;
    }

    void DoShowToast(const std::string& msg, ToastType type) {
        if (activeToast) {
            root->RemoveChild(activeToast);
            delete activeToast;
        }
        activeToast = new Toast(300, {200, 620}, {400, 40}, msg);
        activeToast->SetType(type);
        activeToast->SetDuration(2.5f);
        root->AddChild(activeToast);
        activeToast->Show();
        if (statusLabel) statusLabel->SetText("Toast: " + msg);
    }

    enum class Action { None, ShowDialog, HideDialog, ShowToast };
    Action pendingAction = Action::None;
    std::string pendingTitle, pendingMsg, pendingToastMsg;
    ToastType pendingToastType = ToastType::Info;
};

void DialogDemoScene::OnEnter() {
    m = new Impl();

    m->root = new BoxLayout(1, {30, 30}, {740, 660}, BoxDirection::Vertical, 15, 10);
    m->root->SetDrawBackgroundFromTheme(true);

    auto* title = new Label(2, {0, 0}, "Dialog & Toast Demo", Label::Role::Title);
    title->SetSize({700, 30});

    auto* infoLabel = new Label(3, {0, 0}, "Click buttons to trigger dialogs and toast notifications");
    infoLabel->SetSize({700, 22});

    m->statusLabel = new Label(4, {0, 0}, "Status: idle");
    m->statusLabel->SetSize({700, 22});

    // Spacer
    auto* spacer = new Label(5, {0, 0}, "");
    spacer->SetSize({700, 30});

    // Dialog buttons
    auto* dlgLabel = new Label(10, {0, 0}, "--- 弹窗 Dialog ---", Label::Role::Subtitle);
    dlgLabel->SetSize({700, 22});

    auto* dlgBtnRow = new HBoxLayout(11, {0, 0}, {700, 42}, 0, 10);

    auto* alertBtn = new Button(12, {0, 0}, {160, 42}, "Alert");
    alertBtn->SetOnClick([this]() {
        m->RequestShowDialog("Alert", "This is an alert message.\nPlease acknowledge.");
    });

    auto* confirmBtn = new Button(13, {0, 0}, {160, 42}, "Confirm");
    confirmBtn->SetOnClick([this]() {
        m->RequestShowDialog("Confirm", "Are you sure you want to proceed?");
    });

    auto* multiBtn = new Button(14, {0, 0}, {160, 42}, "Multi-Button");
    multiBtn->SetOnClick([this]() {
        m->RequestShowDialog("Save Changes", "Do you want to save before closing?");
    });

    auto* closeBtn = new Button(15, {0, 0}, {160, 42}, "Close");
    closeBtn->SetOnClick([this]() {
        m->RequestHideDialog();
    });

    dlgBtnRow->AddChild(alertBtn);
    dlgBtnRow->AddChild(confirmBtn);
    dlgBtnRow->AddChild(multiBtn);
    dlgBtnRow->AddChild(closeBtn);

    // Toast buttons
    auto* toastLabel = new Label(20, {0, 0}, "--- 通知 Toast ---", Label::Role::Subtitle);
    toastLabel->SetSize({700, 22});

    auto* toastBtnRow = new HBoxLayout(21, {0, 0}, {700, 42}, 0, 10);

    auto* infoToast = new Button(22, {0, 0}, {110, 42}, "Info");
    infoToast->SetOnClick([this]() { m->RequestShowToast("Information message", ToastType::Info); });

    auto* successToast = new Button(23, {0, 0}, {110, 42}, "Success");
    successToast->SetOnClick([this]() { m->RequestShowToast("Operation completed!", ToastType::Success); });

    auto* warnToast = new Button(24, {0, 0}, {110, 42}, "Warning");
    warnToast->SetOnClick([this]() { m->RequestShowToast("Low battery warning", ToastType::Warning); });

    auto* errorToast = new Button(25, {0, 0}, {110, 42}, "Error");
    errorToast->SetOnClick([this]() { m->RequestShowToast("Connection failed!", ToastType::Error); });

    toastBtnRow->AddChild(infoToast);
    toastBtnRow->AddChild(successToast);
    toastBtnRow->AddChild(warnToast);
    toastBtnRow->AddChild(errorToast);

    // Back button
    auto* backBtn = new Button(99, {0, 0}, {150, 42}, "< Back to Dashboard");
    backBtn->SetOnClick([]() {
        Application::Get().GetSceneManager().Pop();
    });

    m->root->AddChild(title);
    m->root->AddChild(infoLabel);
    m->root->AddChild(m->statusLabel);
    m->root->AddChild(spacer);
    m->root->AddChild(dlgLabel);
    m->root->AddChild(dlgBtnRow);
    m->root->AddChild(toastLabel);
    m->root->AddChild(toastBtnRow);
    m->root->AddChild(backBtn);
    m->root->SubscribeThemeChange();
    FocusManager::Get().ClearAll();
    m->root->RegisterFocusRecursive();
}

void DialogDemoScene::OnExit() {
    m->Cleanup();
    delete m->root;
    delete m;
    m = nullptr;
}

void DialogDemoScene::OnUpdate(float dt) {
    if (m) m->FlushPending();  // safe: called before traversal, no iterator invalidation
    auto& ctx = Application::Get().GetUIContext();
    if (m && m->root) m->root->Update(ctx);
}

void DialogDemoScene::OnDraw() {
    auto& app = Application::Get();
    auto winSize = app.GetPlatform().GetWindowSize();
    glm::mat4 proj = glm::ortho(0.0f, (float)winSize.x, (float)winSize.y, 0.0f, -1.0f, 1.0f);
    app.GetRenderer().Begin(proj);
    auto& ctx = app.GetUIContext();
    if (m && m->root) m->root->Draw(ctx);
    app.GetRenderer().End();
}

void DialogDemoScene::OnImGui() {
    ImGui::Begin("Dialog Debug");
    ImGui::Text("Active dialog: %s", m->activeDialog ? "yes" : "no");
    ImGui::Text("Active toast: %s", m->activeToast ? "yes" : "no");
    ImGui::Separator();
    if (ImGui::Button("Show Info Toast (code)")) {
        ImGui::Text("Toast sent!");
        m->RequestShowToast("Hello from ImGui!", ToastType::Info);
    }
    ImGui::End();
}

} // namespace CarHMI::Gallery
