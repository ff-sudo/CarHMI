#pragma once

#include "batch_renderer2d.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace CarHMI {

class Font {
public:
    Font() = default;
    ~Font();

    bool LoadFromFile(const std::string& path, float pixelSize);

    void DrawText(BatchRenderer2D& renderer, const std::string& text,
                  float x, float y, const glm::vec4& color = glm::vec4(1.0f));

    float MeasureWidth(const std::string& text);
    float GetLineHeight() const { return m_lineHeight; }

    Texture2D& GetAtlasTexture() { return m_atlas; }

private:
    struct GlyphInfo {
        float u0, v0, u1, v1;
        float xoff, yoff;
        float width, height;
        float advance;
        bool valid = false;
    };

    GlyphInfo& GetOrLoadGlyph(uint32_t codepoint);
    bool BakeGlyph(uint32_t codepoint, GlyphInfo& glyph);
    void RebuildAtlas();

    static uint32_t DecodeUTF8(const char*& ptr, const char* end);

    std::vector<unsigned char> m_fontData;
    float m_pixelSize = 0;
    float m_lineHeight = 0;
    float m_scale = 0;
    int m_ascent = 0;
    int m_fontOffset = 0;

    // Glyph cache
    std::unordered_map<uint32_t, GlyphInfo> m_glyphCache;

    // Atlas packing state
    static constexpr int AtlasSize = 1024;
    std::vector<unsigned char> m_atlasBitmap;
    int m_packX = 0;
    int m_packY = 0;
    int m_packRowHeight = 0;
    bool m_atlasDirty = false;

    Texture2D m_atlas;
    bool m_fontLoaded = false;
};

} // namespace CarHMI
