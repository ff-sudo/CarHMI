#pragma once

#include <gui/layout/insets.h>
#include <gui/layout/size_policy.h>
#include <glm/glm.hpp>

namespace CarHMI::GUI {

struct LayoutData {
    Insets margin;
    SizePolicy sizePolicy;
    glm::vec2 scrollOffset = {0, 0};
};

} // namespace CarHMI::GUI
