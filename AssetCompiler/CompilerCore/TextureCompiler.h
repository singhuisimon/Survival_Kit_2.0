/*
* @file TextureCompiler.h
* @brief Texture resource compiler for AssetCompiler
* @details Compiles PNG/JPG/TGA textures to optimized binary format for runtime loading
* @author
* @date
*/

#pragma once

#include <string>
#include <vector>

namespace AssetCompiler {

	/**
	 * @brief Texture Settings for compilation (read from Descriptor.txt)
	 */
	struct TextureSettingsCompiler {
		// Output format
		std::string outputFormat = "RGBA8";

		// Mipmap generation
		bool generateMipmaps = true;

		// Color space
		bool srgb = true;  // Use sRGB color space (true for most diffuse textures)

		// Compression
		std::string compression = "NONE";  // NONE, DXT1, DXT5, etc.
		float quality = 1.0f;  // Compression quality 0.0-1.0

		// Channels to include
		int forceChannels = 0;  // 0=auto, 3=RGB, 4=RGBA

		// Processing flags
		bool flipVertical = false;  // Flip texture vertically (OpenGL convention)
		bool premultiplyAlpha = false;  // Premultiply alpha for blending
	};

	/**
	 * @brief Binary texture file header (follows CompiledResourceHeader)
	 */
	struct CompiledTextureHeader {
		char magic[4] = { 'T', 'E', 'X', '\0' };  // Magic number "TEX"
		uint32_t version = 1;                      // Format version

		uint32_t width = 0;                        // Texture width in pixels
		uint32_t height = 0;                       // Texture height in pixels
		uint32_t channels = 4;                     // Number of channels (3=RGB, 4=RGBA)
		uint32_t mipLevels = 1;                    // Number of mipmap levels

		uint32_t format = 0;                       // Internal format (0=uncompressed)
		uint32_t dataFormat = 0;                   // Data format (GL_RGBA, etc.)
		uint32_t dataType = 0;                     // Data type (GL_UNSIGNED_BYTE, etc.)

		uint32_t srgb = 0;                         // 1 if sRGB, 0 if linear
		uint32_t compressed = 0;                   // 1 if compressed, 0 if not

		uint32_t reserved[5] = { 0 };              // For future use
	};

	class TextureCompiler {
	public:
		TextureCompiler() = default;
		~TextureCompiler() = default;

		/**
		* @brief Compile a texture from descriptor
		* @param descriptorPath path to Descriptor.txt folder
		* @param outputPath path to write compiled GUID.tex file
		* @param verbose Enable verbose logging
		* @return true if compilation succeeded
		*/
		bool compile(const std::string& descriptorPath,
			const std::string& outputPath,
			bool verbose = false);

	private:
		// === Loading ===
		bool loadImage(const std::string& path, int& width, int& height,
			int& channels, std::vector<unsigned char>& data, int forceChannels = 0);

		// === Processing ===
		void flipVertical(std::vector<unsigned char>& data, int width, int height, int channels);
		void premultiplyAlpha(std::vector<unsigned char>& data, int width, int height, int channels);

		// === Mipmap Generation ===
		std::vector<std::vector<unsigned char>> generateMipmaps(
			const std::vector<unsigned char>& data,
			int width, int height, int channels, int levels);

		int calculateMipLevels(int width, int height) const;

		// === Serialization ===
		bool writeBinaryTexture(const std::string& outputPath,
			const CompiledTextureHeader& header,
			const std::vector<std::vector<unsigned char>>& mipData);

		// === Helpers ===
		bool parseSettings(const std::string& descriptorPath,
			std::string& sourcePath,
			TextureSettingsCompiler& settings);

		std::string fixPathSeparators(const std::string& path);

		uint32_t calculateCRC32(const unsigned char* data, size_t length);

		bool verbose_ = false;

		void log(const char* format, ...);
	};

} // end of namespace AssetCompiler