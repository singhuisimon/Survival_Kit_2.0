



#include "FontCompiler.h"

#include <map>
#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>

namespace AssetCompiler {

	//static member initialization
	FT_Library FontCompiler::ftLibrary;
	bool FontCompiler::ftInitialized = false;

	bool FontCompiler::compile(const fs::path& ttfPath) {
		// Initialize FreeType if needed
		if (!ftInitialized) {
			if (FT_Init_FreeType(&ftLibrary)) {
				return false;
			}

			int major, minor, patch;
			FT_Library_Version(ftLibrary, &major, &minor, &patch);

			ftInitialized = true;
		}

		// Verify file exists
		if (!fs::exists(ttfPath)) {
			return false;
		}

		// 1. Load TTF file into memory
		std::ifstream file(ttfPath, std::ios::binary);
		if (!file) {
			return false;
		}

		std::vector<uint8_t> ttfData((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
		file.close();


		// 2. Load font face
		FT_Face face;
		if (FT_New_Memory_Face(ftLibrary, ttfData.data(),
			static_cast<FT_Long>(ttfData.size()), 0, &face)) {
			return false;
		}

		// 3. Get ASCII character set
		std::vector<uint32_t> characters = getCharacterSet();

		// 4. Build atlas
		AtlasData atlas = buildAtlas(ttfData, characters);

		if (atlas.pixels.empty()) {
			FT_Done_Face(face);
			return false;
		}

		// 5. Extract font metrics
		FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

		FontMetrics metrics;
		metrics.ascent = face->size->metrics.ascender / 64.0f;
		metrics.descent = face->size->metrics.descender / 64.0f;
		metrics.lineHeight = face->size->metrics.height / 64.0f;
		metrics.baseSize = static_cast<float>(FONT_SIZE);

		// 6. Serialize to binary
		std::vector<uint8_t> compiledData = serializeFontData(metrics, atlas);

		// 7. Write output file
		fs::path outputPath = ttfPath;
		outputPath += ".font";  // e.g., arial.ttf ? arial.ttf.font

		if (!writeFontFile(outputPath, compiledData)) {
			FT_Done_Face(face);
			return false;
		}

		// Cleanup
		FT_Done_Face(face);

		return true;
	}

	std::vector<uint32_t> FontCompiler::getCharacterSet() {
		std::vector<uint32_t> chars;

		//printable ASCII: space(32) to tilde(126)
		for (uint32_t c = 32; c <= 126; c++) {
			chars.push_back(c);
		}

		return chars;
	}

	FontCompiler::GlyphRasterData FontCompiler::rasterizeGlyph(FT_Face face, uint32_t charCode) {
		GlyphRasterData result; 

		//set the size 
		FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

		//load glyph 
		if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
			return result; 
		}

		//copy bitmap 
		FT_Bitmap& bitmap = face->glyph->bitmap;
		result.width = bitmap.width; 
		result.height = bitmap.rows;
		result.bearingX = face->glyph->bitmap_left;
		result.bearingY = face->glyph->bitmap_top;
		result.advance = face->glyph->advance.x / 64.0f;

		//copy pixel data
		result.data.resize(result.width * result.height);
		std::copy(bitmap.buffer,
			bitmap.buffer + result.width * result.height,
			result.data.begin()); 

		return result; 
	}

	FontCompiler::AtlasData FontCompiler::buildAtlas(
		const std::vector<uint8_t>& ttfData,
		const std::vector<uint32_t>& characters
	) {
		//load font face
		FT_Face face; 
		if (FT_New_Memory_Face(ftLibrary, ttfData.data(), ttfData.size(), 0, &face)) {
			return{};
		}

		//rasterize all glyphs 
		std::vector<std::pair<uint32_t, GlyphRasterData>> glyphs; 

		for (uint32_t charCode : characters) {
			GlyphRasterData rasterData = rasterizeGlyph(face, charCode);

			if (!rasterData.data.empty()) {
				//convert grayscale bitmap to SDF 
				if (rasterData.width > 0 && rasterData.height > 0) {
					std::vector<uint8_t> sdfData = generateSDF(
						rasterData.data.data(),
						rasterData.width,
						rasterData.height
					);
					rasterData.data = sdfData; //replace with SDF
				}
				glyphs.push_back({ charCode, rasterData });
			}
		}

		//pack into atlas 
		AtlasData atlas = packGlyphsIntoAtlas(glyphs);

		FT_Done_Face(face); 

		return atlas;
	}

	std::vector<uint8_t> FontCompiler::generateSDF(const uint8_t* bitmap, int width, int height) {
		std::vector<uint8_t> sdf(width * height); 

		// Simple brute-force SDF generation
			// For each pixel, find distance to nearest edge

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int idx = y * width + x;
				bool inside = bitmap[idx] > 127;  // Threshold

				float minDist = static_cast<float>(SDF_SPREAD);

				// Search in a square region
				for (int dy = -SDF_SPREAD; dy <= SDF_SPREAD; ++dy) {
					for (int dx = -SDF_SPREAD; dx <= SDF_SPREAD; ++dx) {
						int nx = x + dx;
						int ny = y + dy;

						if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
							int nidx = ny * width + nx;
							bool neighborInside = bitmap[nidx] > 127;

							// Edge detection: inside/outside transition
							if (inside != neighborInside) {
								float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
								minDist = std::min(minDist, dist);
							}
						}
					}
				}

				// Normalize distance to 0-1 range
				float normalizedDist = minDist / SDF_SPREAD;
				normalizedDist = std::clamp(normalizedDist, 0.0f, 1.0f);

				// Map to 0-255: inside = 128-255, outside = 0-127
				float sdfValue = inside ? (0.5f + normalizedDist * 0.5f)
					: (0.5f - normalizedDist * 0.5f);

				sdf[idx] = static_cast<uint8_t>(sdfValue * 255.0f);
			}
		}

		return sdf;
	}

	FontCompiler::AtlasData FontCompiler::packGlyphsIntoAtlas(
		const std::vector<std::pair<uint32_t, GlyphRasterData>>& glyphs
	) {

		AtlasData atlas; 
		atlas.width = ATLAS_WIDTH; 
		atlas.height = ATLAS_HEIGHT;
		atlas.pixels.resize(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0); //RGBA

		//simple row packing algirthm
		int currentX = PADDING;
		int currentY = PADDING; 
		int rowHeight = 0; 

		for (const auto& [charCode, glyph] : glyphs) {
			// Check if we need to move to next row
			if (currentX + glyph.width + PADDING > ATLAS_WIDTH) {
				currentX = PADDING;
				currentY += rowHeight + PADDING;
				rowHeight = 0;
			}

			// Check if we've run out of vertical space
			if (currentY + glyph.height + PADDING > ATLAS_HEIGHT) {
				break;
			}

			// Copy glyph into atlas
			for (int y = 0; y < glyph.height; ++y) {
				for (int x = 0; x < glyph.width; ++x) {
					int srcIdx = y * glyph.width + x;
					int dstIdx = ((currentY + y) * ATLAS_WIDTH + (currentX + x)) * 4;

					uint8_t value = glyph.data[srcIdx];
					atlas.pixels[dstIdx + 0] = 255;      // R
					atlas.pixels[dstIdx + 1] = 255;      // G
					atlas.pixels[dstIdx + 2] = 255;      // B
					atlas.pixels[dstIdx + 3] = value;    // A (SDF value)
				}
			}

			// Store glyph metrics
			GlyphMetrics metrics;
			metrics.charCode = charCode;
			metrics.uv_x = static_cast<float>(currentX) / ATLAS_WIDTH;
			metrics.uv_y = static_cast<float>(currentY) / ATLAS_HEIGHT;
			metrics.uv_width = static_cast<float>(glyph.width) / ATLAS_WIDTH;
			metrics.uv_height = static_cast<float>(glyph.height) / ATLAS_HEIGHT;
			metrics.p_width = static_cast<float>(glyph.width);
			metrics.p_height = static_cast<float>(glyph.height);
			metrics.bearing_x = static_cast<float>(glyph.bearingX);
			metrics.bearing_y = static_cast<float>(glyph.bearingY);
			metrics.advance = glyph.advance;

			atlas.glyphs.push_back(metrics);

			// Update position
			currentX += glyph.width + PADDING;
			rowHeight = std::max(rowHeight, glyph.height);

		}
		return atlas;
	}

	std::vector<uint8_t> FontCompiler::serializeFontData(const FontMetrics& fontMetrics,
		const AtlasData& atlas) {
		std::vector<uint8_t> data;

		// Helper to write binary data
		auto WriteBytes = [&](const void* src, size_t size) {
			const uint8_t* bytes = static_cast<const uint8_t*>(src);
			data.insert(data.end(), bytes, bytes + size);
			};

		// 1. Write font metrics
		WriteBytes(&fontMetrics.lineHeight, sizeof(float));
		WriteBytes(&fontMetrics.ascent, sizeof(float));
		WriteBytes(&fontMetrics.descent, sizeof(float));
		WriteBytes(&fontMetrics.baseSize, sizeof(float));

		// 2. Write atlas dimensions
		WriteBytes(&atlas.width, sizeof(int));
		WriteBytes(&atlas.height, sizeof(int));

		// 3. Write atlas pixel data size and data
		uint32_t pixelDataSize = static_cast<uint32_t>(atlas.pixels.size());
		WriteBytes(&pixelDataSize, sizeof(uint32_t));
		WriteBytes(atlas.pixels.data(), pixelDataSize);

		// 4. Write glyph count
		uint32_t glyphCount = static_cast<uint32_t>(atlas.glyphs.size());
		WriteBytes(&glyphCount, sizeof(uint32_t));

		// 5. Write all glyph metrics
		for (const auto& glyph : atlas.glyphs) {
			WriteBytes(&glyph, sizeof(GlyphMetrics));
		}

		return data;
	}

	bool FontCompiler:: writeFontFile(const fs::path& outputPath,
		const std::vector<uint8_t>& data) {
		std::ofstream file(outputPath, std::ios::binary);
		if (!file) {
			return false;
		}

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();

		return true;
	}

}//end of namespace AssetCompiler