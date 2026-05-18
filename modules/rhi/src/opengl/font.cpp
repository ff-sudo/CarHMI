#include <rhi/font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <fstream>
#include <cstdio>
#include <cstring>
#include <spdlog/spdlog.h>

namespace CarHMI::RHI {

Font::~Font() = default;

bool Font::LoadFromFile(const std::string& path, float pixelSize) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        spdlog::error("Font: cannot open {}", path);
        return false;
    }
    m_fontData.assign((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());

    int fontOffset = stbtt_GetFontOffsetForIndex(m_fontData.data(), 0);
    if (fontOffset < 0) fontOffset = 0;

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, m_fontData.data(), fontOffset)) {
        spdlog::error("Font: stbtt_InitFont failed for {}", path);
        return false;
    }

    m_pixelSize = pixelSize;
    m_scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelSize);
    m_fontOffset = fontOffset;

    int descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &m_ascent, &descent, &lineGap);
    m_lineHeight = (m_ascent - descent + lineGap) * m_scale;

    m_glyphCache.clear();
    m_atlasBitmap.assign(AtlasSize * AtlasSize, 0);
    m_packX = 1;
    m_packY = 1;
    m_packRowHeight = 0;
    m_atlasDirty = false;

    for (int c = 32; c < 127; c++)
        GetOrLoadGlyph(c);
    RebuildAtlas();

    m_fontLoaded = true;
    spdlog::info("Font loaded: {} (offset={}, scale={:.4f})", path, fontOffset, m_scale);
    return true;
}

Font::GlyphInfo& Font::GetOrLoadGlyph(uint32_t codepoint) {
    auto it = m_glyphCache.find(codepoint);
    if (it != m_glyphCache.end())
        return it->second;

    GlyphInfo glyph{};
    BakeGlyph(codepoint, glyph);
    auto [inserted, _] = m_glyphCache.emplace(codepoint, glyph);
    return inserted->second;
}

bool Font::BakeGlyph(uint32_t codepoint, GlyphInfo& glyph) {
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, m_fontData.data(), m_fontOffset)) {
        glyph.valid = false;
        return false;
    }

    int glyphIndex = stbtt_FindGlyphIndex(&fontInfo, codepoint);
    if (glyphIndex == 0 && codepoint != 0) {
        glyph.valid = false;
        glyph.advance = m_pixelSize * 0.5f;
        spdlog::debug("Font: no glyph for U+{:04X}", codepoint);
        return false;
    }

    int x0, y0, x1, y1;
    stbtt_GetGlyphBitmapBox(&fontInfo, glyphIndex, m_scale, m_scale, &x0, &y0, &x1, &y1);

    int gw = x1 - x0;
    int gh = y1 - y0;

    if (gw <= 0 || gh <= 0) {
        int advW, lsb;
        stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advW, &lsb);
        glyph.valid = true;
        glyph.width = 0;
        glyph.height = 0;
        glyph.advance = advW * m_scale;
        return true;
    }

    if (m_packX + gw + 1 > AtlasSize) {
        m_packX = 1;
        m_packY += m_packRowHeight + 1;
        m_packRowHeight = 0;
    }

    if (m_packY + gh + 1 > AtlasSize) {
        spdlog::warn("Font: atlas full at U+{:04X}", codepoint);
        glyph.valid = false;
        return false;
    }

    stbtt_MakeGlyphBitmap(&fontInfo, &m_atlasBitmap[m_packY * AtlasSize + m_packX],
                           gw, gh, AtlasSize, m_scale, m_scale, glyphIndex);

    glyph.u0 = (float)m_packX / AtlasSize;
    glyph.v0 = (float)m_packY / AtlasSize;
    glyph.u1 = (float)(m_packX + gw) / AtlasSize;
    glyph.v1 = (float)(m_packY + gh) / AtlasSize;
    glyph.xoff = (float)x0;
    glyph.yoff = (float)y0;
    glyph.width = (float)gw;
    glyph.height = (float)gh;

    int advW, lsb;
    stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advW, &lsb);
    glyph.advance = advW * m_scale;
    glyph.valid = true;

    m_packX += gw + 1;
    if (gh > m_packRowHeight) m_packRowHeight = gh;
    m_atlasDirty = true;

    return true;
}

void Font::RebuildAtlas() {
    if (!m_atlasDirty) return;

    std::vector<unsigned char> rgba(AtlasSize * AtlasSize * 4);
    for (int i = 0; i < AtlasSize * AtlasSize; i++) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = m_atlasBitmap[i];
    }

    m_atlas.LoadFromMemory(rgba.data(), AtlasSize, AtlasSize, 4);
    m_atlasDirty = false;
}

uint32_t Font::DecodeUTF8(const char*& ptr, const char* end) {
    if (ptr >= end) return 0;

    unsigned char c = (unsigned char)*ptr;

    if (c < 0x80) {
        ptr++;
        return c;
    }

    uint32_t cp = 0;
    int bytes = 0;

    if ((c & 0xE0) == 0xC0)      { cp = c & 0x1F; bytes = 1; }
    else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; bytes = 2; }
    else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; bytes = 3; }
    else { ptr++; return 0xFFFD; }

    ptr++;
    for (int i = 0; i < bytes && ptr < end; i++) {
        unsigned char next = (unsigned char)*ptr;
        if ((next & 0xC0) != 0x80) return 0xFFFD;
        cp = (cp << 6) | (next & 0x3F);
        ptr++;
    }

    return cp;
}

void Font::DrawText(BatchRenderer2D& renderer, const std::string& text,
                    float x, float y, const glm::vec4& color) {
    if (!m_fontLoaded) return;

    {
        const char* ptr = text.c_str();
        const char* end = ptr + text.size();
        while (ptr < end) {
            uint32_t cp = DecodeUTF8(ptr, end);
            if (cp == 0) break;
            if (cp != '\n') GetOrLoadGlyph(cp);
        }
        if (m_atlasDirty) RebuildAtlas();
    }

    float cursorX = x;
    float baseline = y + m_ascent * m_scale;

    const char* ptr = text.c_str();
    const char* end = ptr + text.size();

    while (ptr < end) {
        uint32_t cp = DecodeUTF8(ptr, end);
        if (cp == 0) break;

        if (cp == '\n') {
            cursorX = x;
            baseline += m_lineHeight;
            continue;
        }

        auto& g = GetOrLoadGlyph(cp);
        if (!g.valid) {
            cursorX += g.advance;
            continue;
        }

        if (g.width > 0 && g.height > 0) {
            float qx = cursorX + g.xoff;
            float qy = baseline + g.yoff;

            renderer.DrawSubTexturedQuad(
                {qx, qy}, {g.width, g.height},
                m_atlas, {g.u0, g.v0}, {g.u1, g.v1}, color);
        }

        cursorX += g.advance;
    }
}

float Font::MeasureWidth(const std::string& text) {
    if (!m_fontLoaded) return 0;

    float width = 0;
    const char* ptr = text.c_str();
    const char* end = ptr + text.size();

    while (ptr < end) {
        uint32_t cp = DecodeUTF8(ptr, end);
        if (cp == 0 || cp == '\n') break;
        auto& g = GetOrLoadGlyph(cp);
        width += g.advance;
    }

    if (m_atlasDirty) RebuildAtlas();
    return width;
}

} // namespace CarHMI::RHI
