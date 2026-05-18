#include <gui/i18n/bidi.h>

extern "C" {
#include <SheenBidi/SBAlgorithm.h>
#include <SheenBidi/SBParagraph.h>
#include <SheenBidi/SBLine.h>
#include <SheenBidi/SBRun.h>
#include <SheenBidi/SBCodepointSequence.h>
#include <SheenBidi/SBBase.h>
}

#include <algorithm>

namespace CarHMI::GUI {

std::u32string BiDi::UTF8ToUTF32(const std::string& utf8) {
    std::u32string result;
    const char* ptr = utf8.c_str();
    const char* end = ptr + utf8.size();

    while (ptr < end) {
        unsigned char c = (unsigned char)*ptr;
        uint32_t cp = 0;
        int bytes = 0;

        if (c < 0x80)           { cp = c; bytes = 0; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; bytes = 1; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; bytes = 2; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; bytes = 3; }
        else { ptr++; result.push_back(0xFFFD); continue; }

        ptr++;
        for (int i = 0; i < bytes && ptr < end; i++) {
            cp = (cp << 6) | ((unsigned char)*ptr & 0x3F);
            ptr++;
        }
        result.push_back(cp);
    }
    return result;
}

std::string BiDi::UTF32ToUTF8(const std::u32string& utf32) {
    std::string result;
    for (uint32_t cp : utf32) {
        if (cp < 0x80) {
            result += (char)cp;
        } else if (cp < 0x800) {
            result += (char)(0xC0 | (cp >> 6));
            result += (char)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            result += (char)(0xE0 | (cp >> 12));
            result += (char)(0x80 | ((cp >> 6) & 0x3F));
            result += (char)(0x80 | (cp & 0x3F));
        } else {
            result += (char)(0xF0 | (cp >> 18));
            result += (char)(0x80 | ((cp >> 12) & 0x3F));
            result += (char)(0x80 | ((cp >> 6) & 0x3F));
            result += (char)(0x80 | (cp & 0x3F));
        }
    }
    return result;
}

std::u32string BiDi::ReorderLine(const std::u32string& input, bool baseRTL) {
    if (input.empty()) return input;

    SBCodepointSequence seq;
    seq.stringEncoding = SBStringEncodingUTF32;
    seq.stringBuffer = (void*)input.data();
    seq.stringLength = input.size();

    SBAlgorithmRef algorithm = SBAlgorithmCreate(&seq);
    if (!algorithm) return input;

    SBLevel baseLevel = baseRTL ? 1 : 0;
    SBParagraphRef paragraph = SBAlgorithmCreateParagraph(
        algorithm, 0, (SBUInteger)input.size(), baseLevel);

    if (!paragraph) {
        SBAlgorithmRelease(algorithm);
        return input;
    }

    SBLineRef line = SBParagraphCreateLine(paragraph, 0, (SBUInteger)input.size());
    if (!line) {
        SBParagraphRelease(paragraph);
        SBAlgorithmRelease(algorithm);
        return input;
    }

    SBUInteger runCount = SBLineGetRunCount(line);
    const SBRun* runs = SBLineGetRunsPtr(line);

    std::u32string result;
    result.reserve(input.size());

    for (SBUInteger i = 0; i < runCount; i++) {
        const SBRun& run = runs[i];
        std::u32string segment(input.begin() + run.offset,
                               input.begin() + run.offset + run.length);

        if (run.level & 1)
            std::reverse(segment.begin(), segment.end());

        result += segment;
    }

    SBLineRelease(line);
    SBParagraphRelease(paragraph);
    SBAlgorithmRelease(algorithm);

    return result;
}

std::string BiDi::ProcessText(const std::string& text, bool rtl) {
    auto utf32 = UTF8ToUTF32(text);
    auto reordered = ReorderLine(utf32, rtl);
    return UTF32ToUTF8(reordered);
}

} // namespace CarHMI::GUI
