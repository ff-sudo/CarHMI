#include <gtest/gtest.h>
#include <gui/i18n/bidi.h>

using namespace CarHMI::GUI;

// --- UTF-8 <-> UTF-32 ---

TEST(BiDiTest, UTF8ToUTF32_Ascii) {
    auto result = BiDi::UTF8ToUTF32("Hello");
    ASSERT_EQ(result.size(), 5u);
    EXPECT_EQ(result[0], (uint32_t)'H');
    EXPECT_EQ(result[4], (uint32_t)'o');
}

TEST(BiDiTest, UTF8ToUTF32_Chinese) {
    auto result = BiDi::UTF8ToUTF32("中文");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], 0x4E2Du);  // 中
    EXPECT_EQ(result[1], 0x6587u);  // 文
}

TEST(BiDiTest, UTF8ToUTF32_Arabic) {
    auto result = BiDi::UTF8ToUTF32("مرحبا");
    // مرحبا = 5 Arabic characters
    ASSERT_EQ(result.size(), 5u);
}

TEST(BiDiTest, UTF32ToUTF8RoundTrip) {
    std::string original = "Hello 世界 مرحبا";
    auto utf32 = BiDi::UTF8ToUTF32(original);
    auto utf8 = BiDi::UTF32ToUTF8(utf32);
    EXPECT_EQ(utf8, original);
}

TEST(BiDiTest, UTF32ToUTF8_Ascii) {
    std::u32string input = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // Hello
    auto result = BiDi::UTF32ToUTF8(input);
    EXPECT_EQ(result, "Hello");
}

TEST(BiDiTest, UTF8ToUTF32_Empty) {
    auto result = BiDi::UTF8ToUTF32("");
    EXPECT_EQ(result.size(), 0u);
}

TEST(BiDiTest, UTF32ToUTF8_Empty) {
    auto result = BiDi::UTF32ToUTF8({});
    EXPECT_EQ(result, "");
}

// --- ReorderLine ---

TEST(BiDiTest, ReorderLine_EmptyInput) {
    auto result = BiDi::ReorderLine({}, true);
    EXPECT_EQ(result.size(), 0u);
}

TEST(BiDiTest, ReorderLine_AsciiLTR) {
    auto input = BiDi::UTF8ToUTF32("Hello World");
    auto result = BiDi::ReorderLine(input, false);
    // LTR text with LTR base should remain in order
    auto utf8 = BiDi::UTF32ToUTF8(result);
    EXPECT_EQ(utf8, "Hello World");
}

// --- ProcessText ---

TEST(BiDiTest, ProcessText_Empty) {
    auto result = BiDi::ProcessText("", false);
    EXPECT_EQ(result, "");
}

TEST(BiDiTest, ProcessText_SimpleEnglish) {
    auto result = BiDi::ProcessText("Hello World", false);
    EXPECT_EQ(result, "Hello World");
}

TEST(BiDiTest, ProcessText_ChineseLTR) {
    auto result = BiDi::ProcessText("你好世界", false);
    // Chinese text with LTR base should maintain order
    EXPECT_EQ(result, "你好世界");
}
