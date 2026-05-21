#pragma once

#include <core/scene/scene.h>
#include <gui/widgets/text_input.h>

namespace CarHMI::Gallery {

class TextInputDemoScene : public Core::Scene {
public:
    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float dt) override;
    void OnDraw() override;
    void OnImGui() override;
    const char* GetName() const override { return "TextInputDemo"; }

private:
    GUI::TextInput* m_input1 = nullptr;
    GUI::TextInput* m_input2 = nullptr;
    GUI::TextInput* m_input3 = nullptr;
    std::string m_lastSubmit;
};

} // namespace CarHMI::Gallery
