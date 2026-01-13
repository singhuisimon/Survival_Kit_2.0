

#pragma once 

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>

#include "../freetype/include/freetype/freetype.h"



namespace AssetCompiler {


	struct GlyphMetrics {
		uint32_t charCode; //ASCII code value

		float uv_x, uv_y;
		float uv_width, uv_height;

		//glyph dimensions in pixels 
		float p_width, p_height;

		//offset from baseline
		float bearing_x, bearing_y;

		//to advance to the next glyph 
		float advance;
	};

	struct FontMetrics {
		float lineHeight; //distance between baselines
		float ascent;	//distance from baseline to top
		float descent;  //distance from baseline to bottom (negative)
		float baseSize; //size font was compiled at
	};

	class FontCompiler {
	public:


	private:

		static constexpr int ATLAS_WIDTH = 1024;
		static constexpr int ATLAS_HEIGHT = 1024;
		static constexpr int FONT_SIZE = 48;
		static constexpr int PADDING = 4;
		static constexpr int SDF_SPREAD = 8;


		/*
		* @brief tempo structure holding atlas generation results
		*/
		struct AtlasData {
			std::vector<uint8_t> pixels; //RGB data
			int width, height;
			std::vector<GlyphMetrics> glyphs;

		};

		/*
		* @brief tempo storage for a rasterized glyph
		*/
		struct GlyphRasterData {
			std::vector<uint8_t>data;

			int width, height;
			int bearingX, bearingY;
			float advance;
		};


		/*
		* @brief Get list of characters to compile based on character set name
		* @return Vector of printable ASCII character codes to compile
		*/
		//generate character set 
		std::vector<uint32_t> getCharacterSet();

		/*
		* @brief Rasterize a single glyph using FreeType
		*
		* @param face FreeType font face
		* @param charCode character to rasterize
		*
		* @return Rasterized glyph bitmap
		*/
		GlyphRasterData rasterizeGlyph(FT_Face face, uint32_t charCode);

		/*
		* @brief build texture atlas with all glyphs
		*
		* @param ttfData raw ttf file data
		* @param characters list of characters to compile
		*
		* @return Atlas data with packed glyphs
		*/
		AtlasData buildAtlas(const std::vector<uint8_t>& ttfData,
			const std::vector<uint32_t>& characters);

		/*
		* @brief generate signed distance field from bitmap
		*
		* @param bitmap Input bitmap (0 or 225)
		* @param width bitmap width
		* @param height bitmap height
		*
		* @return SDF bitmap
		*/

		std::vector<uint8_t> generateSDF(const uint8_t* bitmap,
			int width, int height);

		/*
		* @brief pack glyphs into atlas using simple row-packing algorithm
		*
		* @param glyphs list of glyph data to pack
		*
		* @return Packed atlas with glyph positions
		*/
		AtlasData packGlyphsIntoAtlas(
			const std::vector<std::pair<uint32_t, GlyphRasterData>>& glyphs);

		/*
		* @brief serialize font data to binary format
		*
		* @param fontMetrics font metrics
		* @param atlas Atlas data
		*
		* @return Binary data ready to write to .font file
		*/

		//serialization
		std::vector<uint8_t> serializeFontData(const FontMetrics& fontMetrics, const AtlasData& atlas);

		static FT_Library ftLibrary;
		static bool ftInitialized;
	};

} //namespace AssetCompiler