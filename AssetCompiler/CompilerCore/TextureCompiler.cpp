#include "TextureCompiler.h"
#include "../Utility/DescriptorParser.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <algorithm>
#include <cmath>
#include "../rapidjson/document.h"
#include "../rapidjson/istreamwrapper.h"

// STB Image - for loading images
#define STB_IMAGE_IMPLEMENTATION
#include "../Engine/Graphics/stb_image.h"

namespace fs = std::filesystem;

namespace AssetCompiler {

    // ============================================================================
    // PUBLIC API
    // ============================================================================

    bool TextureCompiler::compile(const std::string& descriptorPath,
        const std::string& outputPath,
        bool verbose) {
        verbose_ = verbose;

        log("=== Compiling Texture ===");
        log("Descriptor: %s", descriptorPath.c_str());

        // Step 1: Parse descriptor to get source path and settings
        std::string sourcePath;
        TextureSettingsCompiler settings;

        if (!parseSettings(descriptorPath, sourcePath, settings)) {
            log("ERROR: Failed to parse descriptor settings");
            return false;
        }

        // Fix path separators (handle \\ vs /)
        sourcePath = fixPathSeparators(sourcePath);

        log("Source: %s", sourcePath.c_str());
        log("Settings: mipmaps=%d, srgb=%d, channels=%d",
            settings.generateMipmaps, settings.srgb, settings.forceChannels);

        // Step 2: Check if source file exists
        if (!fs::exists(sourcePath)) {
            log("ERROR: Source file not found: %s", sourcePath.c_str());
            return false;
        }

        // Step 3: Load image data from source file
        int width, height, channels;
        std::vector<unsigned char> imageData;

        if (!loadImage(sourcePath, width, height, channels, imageData, settings.forceChannels)) {
            log("ERROR: Failed to load image: %s", sourcePath.c_str());
            return false;
        }

        // Update channels if we forced a channel count
        if (settings.forceChannels > 0) {
            channels = settings.forceChannels;
        }

        log("Loaded image: %dx%d, %d channels", width, height, channels);

        // Step 4: Apply processing based on settings
        if (settings.flipVertical) {
            flipVertical(imageData, width, height, channels);
            log("Flipped texture vertically");
        }

        if (settings.premultiplyAlpha && channels == 4) {
            premultiplyAlpha(imageData, width, height, channels);
            log("Premultiplied alpha");
        }

        // Step 5: Generate mipmaps if requested
        std::vector<std::vector<unsigned char>> mipChain;
        int mipLevels = 1;

        if (settings.generateMipmaps) {
            mipLevels = calculateMipLevels(width, height);
            mipChain = generateMipmaps(imageData, width, height, channels, mipLevels);
            log("Generated %d mip levels", mipLevels);
        }
        else {
            mipChain.push_back(imageData);
        }

        // Step 6: Prepare binary header
        CompiledTextureHeader header;
        header.width = static_cast<uint32_t>(width);
        header.height = static_cast<uint32_t>(height);
        header.channels = static_cast<uint32_t>(channels);
        header.mipLevels = static_cast<uint32_t>(mipLevels);
        header.format = 0;  // 0 = uncompressed for now
        header.srgb = settings.srgb ? 1 : 0;
        header.compressed = 0;  // No compression yet

        // Step 7: Create output directory if needed
        fs::path outPath(outputPath);
        if (!fs::exists(outPath.parent_path())) {
            fs::create_directories(outPath.parent_path());
        }

        // Step 8: Write binary file
        if (!writeBinaryTexture(outputPath, header, mipChain)) {
            log("ERROR: Failed to write binary texture");
            return false;
        }

        log("Success! Compiled texture: %s", outputPath.c_str());
        log("Output size: %.2f KB", fs::file_size(outputPath) / 1024.0f);

        return true;
    }

    // ============================================================================
    // LOADING
    // ============================================================================

    bool TextureCompiler::loadImage(const std::string& path, int& width, int& height,
        int& channels, std::vector<unsigned char>& data, int forceChannels) {
        log("Loading image: %s", path.c_str());

        // Use stb_image to load
        unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &channels, forceChannels);

        if (!imgData) {
            log("ERROR: stbi_load failed: %s", stbi_failure_reason());
            return false;
        }

        // Update channels if we forced it
        if (forceChannels > 0) {
            channels = forceChannels;
        }

        // Copy to vector
        size_t dataSize = static_cast<size_t>(width) * height * channels;
        data.assign(imgData, imgData + dataSize);

        // Free stb_image data
        stbi_image_free(imgData);

        return true;
    }

    // ============================================================================
    // PROCESSING
    // ============================================================================

    void TextureCompiler::flipVertical(std::vector<unsigned char>& data, int width, int height, int channels) {
        size_t rowSize = static_cast<size_t>(width) * channels;
        std::vector<unsigned char> temp(rowSize);

        for (int y = 0; y < height / 2; ++y) {
            size_t topRow = static_cast<size_t>(y) * rowSize;
            size_t bottomRow = static_cast<size_t>(height - 1 - y) * rowSize;

            // Swap rows
            std::copy(data.begin() + topRow, data.begin() + topRow + rowSize, temp.begin());
            std::copy(data.begin() + bottomRow, data.begin() + bottomRow + rowSize, data.begin() + topRow);
            std::copy(temp.begin(), temp.end(), data.begin() + bottomRow);
        }
    }

    void TextureCompiler::premultiplyAlpha(std::vector<unsigned char>& data, int width, int height, int channels) {
        if (channels != 4) return;  // Only works with RGBA

        for (size_t i = 0; i < data.size(); i += 4) {
            float alpha = data[i + 3] / 255.0f;
            data[i + 0] = static_cast<unsigned char>(data[i + 0] * alpha);
            data[i + 1] = static_cast<unsigned char>(data[i + 1] * alpha);
            data[i + 2] = static_cast<unsigned char>(data[i + 2] * alpha);
        }
    }

    // ============================================================================
    // MIPMAP GENERATION
    // ============================================================================

    std::vector<std::vector<unsigned char>> TextureCompiler::generateMipmaps(
        const std::vector<unsigned char>& data,
        int width, int height, int channels, int levels) {

        std::vector<std::vector<unsigned char>> mips;
        mips.reserve(levels);

        // Level 0 is the original
        mips.push_back(data);

        int currentWidth = width;
        int currentHeight = height;

        // Generate each subsequent level
        for (int level = 1; level < levels; ++level) {
            int newWidth = std::max(1, currentWidth / 2);
            int newHeight = std::max(1, currentHeight / 2);

            std::vector<unsigned char> mipData(static_cast<size_t>(newWidth) * newHeight * channels);

            // Simple box filter downsampling
            for (int y = 0; y < newHeight; ++y) {
                for (int x = 0; x < newWidth; ++x) {
                    // Sample 2x2 block from previous level
                    int srcX = x * 2;
                    int srcY = y * 2;

                    for (int c = 0; c < channels; ++c) {
                        int sum = 0;
                        int count = 0;

                        // Sample up to 2x2 pixels
                        for (int dy = 0; dy < 2 && (srcY + dy) < currentHeight; ++dy) {
                            for (int dx = 0; dx < 2 && (srcX + dx) < currentWidth; ++dx) {
                                size_t srcIdx = static_cast<size_t>((srcY + dy) * currentWidth + (srcX + dx)) * channels + c;
                                sum += mips[level - 1][srcIdx];
                                count++;
                            }
                        }

                        size_t dstIdx = static_cast<size_t>(y * newWidth + x) * channels + c;
                        mipData[dstIdx] = static_cast<unsigned char>(sum / count);
                    }
                }
            }

            mips.push_back(std::move(mipData));
            currentWidth = newWidth;
            currentHeight = newHeight;
        }

        return mips;
    }

    int TextureCompiler::calculateMipLevels(int width, int height) const {
        int maxDim = std::max(width, height);
        return static_cast<int>(std::floor(std::log2(maxDim))) + 1;
    }

    // ============================================================================
    // SERIALIZATION
    // ============================================================================

    bool TextureCompiler::writeBinaryTexture(const std::string& outputPath,
        const CompiledTextureHeader& header,
        const std::vector<std::vector<unsigned char>>& mipData) {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            log("ERROR: Failed to open output file: %s", outputPath.c_str());
            return false;
        }

        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(CompiledTextureHeader));

        // Write mip data
        for (const auto& mip : mipData) {
            file.write(reinterpret_cast<const char*>(mip.data()), mip.size());
        }

        file.close();
        return true;
    }

    // ============================================================================
    // HELPERS
    // ============================================================================

    bool TextureCompiler::parseSettings(const std::string& descriptorPath,
        std::string& sourcePath,
        TextureSettingsCompiler& settings) {
        rapidjson::Document doc;

        std::ifstream ifs(descriptorPath);
        if (!ifs.is_open()) {
            log("ERROR: Could not open descriptor: %s", descriptorPath.c_str());
            return false;
        }

        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);

        if (doc.HasParseError()) {
            log("ERROR: JSON parse error in descriptor");
            return false;
        }

        // Extract source path
        if (doc.HasMember("sourcePath") && doc["sourcePath"].IsString()) {
            sourcePath = doc["sourcePath"].GetString();
        }
        else {
            log("ERROR: No 'sourcePath' in descriptor");
            return false;
        }

        // Extract texture settings
        if (doc.HasMember("textureSettings") && doc["textureSettings"].IsObject()) {
            const auto& ts = doc["textureSettings"];

            if (ts.HasMember("outputFormat") && ts["outputFormat"].IsString()) {
                settings.outputFormat = ts["outputFormat"].GetString();
            }
            if (ts.HasMember("generateMipmaps") && ts["generateMipmaps"].IsBool()) {
                settings.generateMipmaps = ts["generateMipmaps"].GetBool();
            }
            if (ts.HasMember("srgb") && ts["srgb"].IsBool()) {
                settings.srgb = ts["srgb"].GetBool();
            }
            if (ts.HasMember("compression") && ts["compression"].IsString()) {
                settings.compression = ts["compression"].GetString();
            }
            if (ts.HasMember("quality") && ts["quality"].IsNumber()) {
                settings.quality = ts["quality"].GetFloat();
            }
            if (ts.HasMember("forceChannels") && ts["forceChannels"].IsInt()) {
                settings.forceChannels = ts["forceChannels"].GetInt();
            }
            if (ts.HasMember("flipVertical") && ts["flipVertical"].IsBool()) {
                settings.flipVertical = ts["flipVertical"].GetBool();
            }
            if (ts.HasMember("premultiplyAlpha") && ts["premultiplyAlpha"].IsBool()) {
                settings.premultiplyAlpha = ts["premultiplyAlpha"].GetBool();
            }
        }

        return true;
    }

    std::string TextureCompiler::fixPathSeparators(const std::string& path) {
        std::string fixed = path;
        std::replace(fixed.begin(), fixed.end(), '\\', '/');

        // Remove leading slash/backslash if present
        if (!fixed.empty() && (fixed[0] == '/' || fixed[0] == '\\')) {
            fixed = fixed.substr(1);
        }

        return fixed;
    }

    uint32_t TextureCompiler::calculateCRC32(const unsigned char* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;

        for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320;
                }
                else {
                    crc = crc >> 1;
                }
            }
        }

        return ~crc;
    }

    void TextureCompiler::log(const char* format, ...) {
        if (!verbose_) return;

        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        std::cout << "  [TextureCompiler] " << buffer << "\n";
    }

} // end of namespace AssetCompiler