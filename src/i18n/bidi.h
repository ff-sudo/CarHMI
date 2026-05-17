#pragma once

#include <string>
#include <vector>

namespace CarHMI {

class BiDi {
public:
    static std::u32string ReorderLine(const std::u32string& input, bool baseRTL);
    static std::u32string UTF8ToUTF32(const std::string& utf8);
    static std::string UTF32ToUTF8(const std::u32string& utf32);

    static std::string ProcessText(const std::string& text, bool rtl);
};

} // namespace CarHMI
