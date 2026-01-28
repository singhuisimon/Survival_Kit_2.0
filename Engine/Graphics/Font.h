
#pragma once 
#ifndef __FONT_H
#define __FONT_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <unordered_map>
#include <cstdint>

namespace Engine {


    /**
     * @brief Glyph metrics for a single character
     * @details Contains UV coordinates, size, bearing, and advance information
     *          needed to render a character from the font atlas
     */
    struct Glyph {
        glm::vec2 uvMin;        //Bottom-left UV coordinate in atlas
        glm::vec2 uvMax;        // Top-right UV coordinate in atlas
        glm::vec2 size;         // Pixel size of the glyph
        glm::vec2 bearing;      // Offset from baseline to left/top of glyph
        float advance;          // Horizontal advance to next glyph

        Glyph()
            : uvMin(0.0f), uvMax(0.0f), size(0.0f),
            bearing(0.0f), advance(0.0f) {
        }
    };

    /**
     * @brief Font data container
     * @details Stores atlas texture and glyph metadata for text rendering.
     *          Similar to Material class - holds all data needed for rendering
     *          text with this font. The Renderer uses this data to render text.
     */
    class Font {
    public:
        /**
         * @brief Default constructor creates empty font
         */
        Font()
            : atlasTexture(0)
            , lineHeight(0.0f)
            , ascent(0.0f)
            , descent(0.0f)
            , baseSize(48.0f)
            , atlasWidth(0)
            , atlasHeight(0)
        {
        }

        /**
         * @brief Destructor cleans up OpenGL texture
         */
        ~Font() {
            if (atlasTexture != 0) {
                glDeleteTextures(1, &atlasTexture);
                atlasTexture = 0;
            }
        }

        // Delete copy to prevent double-free of OpenGL texture
        Font(const Font&) = delete;
        Font& operator=(const Font&) = delete;

        // Allow move
        Font(Font&& other) noexcept
            : atlasTexture(other.atlasTexture)
            , glyphs(std::move(other.glyphs))
            , lineHeight(other.lineHeight)
            , ascent(other.ascent)
            , descent(other.descent)
            , baseSize(other.baseSize)
            , atlasWidth(other.atlasWidth)
            , atlasHeight(other.atlasHeight)
        {
            other.atlasTexture = 0;
        }

        Font& operator=(Font&& other) noexcept {
            if (this != &other) {
                if (atlasTexture != 0) {
                    glDeleteTextures(1, &atlasTexture);
                }

                atlasTexture = other.atlasTexture;
                glyphs = std::move(other.glyphs);
                lineHeight = other.lineHeight;
                ascent = other.ascent;
                descent = other.descent;
                baseSize = other.baseSize;
                atlasWidth = other.atlasWidth;
                atlasHeight = other.atlasHeight;

                other.atlasTexture = 0;
            }
            return *this;
        }

        // Getters
        GLuint getAtlasTexture() const { return atlasTexture; }
        float getLineHeight() const { return lineHeight; }
        float getAscent() const { return ascent; }
        float getDescent() const { return descent; }
        float getBaseSize() const { return baseSize; }
        int getAtlasWidth() const { return atlasWidth; }
        int getAtlasHeight() const { return atlasHeight; }

        /**
         * @brief Get glyph data for a character
         * @param charCode Character code (e.g., 'A' = 65)
         * @return Pointer to glyph data, or nullptr if not found
         */
        const Glyph* getGlyph(uint32_t charCode) const {
            auto it = glyphs.find(charCode);
            return (it != glyphs.end()) ? &it->second : nullptr;
        }

        /**
         * @brief Check if font has a specific character
         */
        bool hasGlyph(uint32_t charCode) const {
            return glyphs.find(charCode) != glyphs.end();
        }

    private:
        // Allow Renderer to set data when loading font
        friend class Renderer;

        GLuint atlasTexture;                         ///< OpenGL texture handle for atlas
        std::unordered_map<uint32_t, Glyph> glyphs;  ///< Character to glyph map

        float lineHeight;   ///< Distance between baselines
        float ascent;       ///< Distance from baseline to top
        float descent;      ///< Distance from baseline to bottom
        float baseSize;     ///< Base font size used for atlas generation

        int atlasWidth;     ///< Atlas texture width
        int atlasHeight;    ///< Atlas texture height
    };



}//end of namespace engine



#endif // !__FONT_H
