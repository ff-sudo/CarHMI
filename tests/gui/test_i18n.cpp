#include <gtest/gtest.h>
#include <gui/i18n/i18n.h>

using namespace CarHMI::GUI;
using namespace CarHMI::Core;

class I18nTest : public ::testing::Test {
protected:
    void TearDown() override {
        // I18n singleton: flush LanguageChangedEvent that may have been posted
        EventBus::Get().FlushDeferred();
    }
};

TEST_F(I18nTest, LoadLanguageAndTranslate) {
    // I18n singleton, load using unique code to avoid cross-test interference
    const char* code = "test_LoadAndT";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/en.json");
    I18n::Get().SetLanguage(code);

    EXPECT_EQ(I18n::Get().T("app.title"), "CarHMI Dashboard");
    EXPECT_EQ(I18n::Get().T("btn.back"), "< Back");
}

TEST_F(I18nTest, MissingKeyReturnsKey) {
    const char* code = "test_MissingKey";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/en.json");
    I18n::Get().SetLanguage(code);

    EXPECT_EQ(I18n::Get().T("nonexistent.key.xyz"), "nonexistent.key.xyz");
}

TEST_F(I18nTest, LanguageDirection) {
    const char* en = "test_DirEN";
    const char* ar = "test_DirAR";

    I18n::Get().LoadLanguage(en, CARHMI_SOURCE_DIR "/resources/i18n/en.json");
    I18n::Get().LoadLanguage(ar, CARHMI_SOURCE_DIR "/resources/i18n/ar.json");

    I18n::Get().SetLanguage(en);
    EXPECT_EQ(I18n::Get().GetDirection(), LayoutDirection::LTR);
    EXPECT_FALSE(I18n::Get().IsRTL());

    I18n::Get().SetLanguage(ar);
    EXPECT_EQ(I18n::Get().GetDirection(), LayoutDirection::RTL);
    EXPECT_TRUE(I18n::Get().IsRTL());
}

TEST_F(I18nTest, FontPathAvailable) {
    const char* code = "test_FontPath";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/zh.json");
    I18n::Get().SetLanguage(code);

    EXPECT_NE(I18n::Get().GetFontPath(), "");
}

TEST_F(I18nTest, LanguageChangedEventPosted) {
    const char* code = "test_LangChanged";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/zh.json");

    bool received = false;
    auto conn = EventBus::Get().Subscribe<LanguageChangedEvent>(
        [&received, code](const LanguageChangedEvent& e) {
            if (std::string(e.languageCode) == code)
                received = true;
        });

    I18n::Get().SetLanguage(code);
    EXPECT_TRUE(received);
}

TEST_F(I18nTest, GetLanguageCodes) {
    const char* code = "test_GetCodes";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/en.json");
    I18n::Get().SetLanguage(code);

    auto codes = I18n::Get().GetLanguageCodes();
    bool found = false;
    for (auto& c : codes)
        if (c == code) { found = true; break; }
    EXPECT_TRUE(found);
}

TEST_F(I18nTest, GetLanguageCode) {
    const char* code = "test_GetLangCode";
    I18n::Get().LoadLanguage(code, CARHMI_SOURCE_DIR "/resources/i18n/en.json");
    I18n::Get().SetLanguage(code);

    EXPECT_EQ(I18n::Get().GetLanguageCode(), code);
}
