#pragma once

namespace CarHMI::GUI {

enum class SizeMode { Fixed, Percent, Fill };

struct SizePolicy {
    SizeMode widthMode = SizeMode::Fixed;
    SizeMode heightMode = SizeMode::Fixed;
    float widthValue = 0;   // Fixed: ignored (uses m_size), Percent: 0-100
    float heightValue = 0;
};

} // namespace CarHMI::GUI
