/**
 * @file Texture.cpp
 * @brief GPU texture resource management with RAII semantics
 * @details Provides a safe, move-only wrapper around OpenGL 4.6 texture objects.
 *          Supports loading textures from files with configurable color space,
 *          mipmap generation, and vertical flip options. Implements RAII to
 *          ensure proper GPU resource cleanup.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Utility/Types.h"
#include "../Utility/Logger.h"
#include "../Graphics/Texture.h"
#include "../Graphics/stb_image.h"

namespace Engine {

	std::optional<std::vector<uint8_t>> Texture::load_pixels_rgba8(const std::filesystem::path& path,
																   bool flip_vertical,
																   int& out_w, 
																   int& out_h, 
																   int& out_channels) {

		stbi_set_flip_vertically_on_load(flip_vertical ? 1 : 0);
		unsigned char* data = stbi_load(path.string().c_str(), &out_w, &out_h, &out_channels, 4); 
		if (!data) return std::nullopt; 

		size_t size = static_cast<size_t>(out_w) * static_cast<size_t>(out_h) * 4; 
		std::vector<uint8_t> pixels(size);
		std::memcpy(pixels.data(), data, size);
		stbi_image_free(data);
		return pixels;
	}

	static inline uint32_t calc_mip_count(uint32_t w, uint32_t h) {
		uint32_t m = 1; // Start with base level

		while (w > 1 || h > 1) { 

			// continues dividing width and height by half using bitwise shift
			// every division increase the mipmap level, std::max to ensure w and h never go below 1

			w = std::max(1u, w >> 1); 
			h = std::max(1u, h >> 1); 
			++m; 
		}

		return m;
	}

	uint64_t Texture::create_gpu_texture_rgba8(const uint8_t* pixels, uint32_t w, uint32_t h,
											   bool srgb, bool gen_mips, uint32_t& out_mip_levels) {

		//GLuint tex = 0;
		//glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		//const GLenum internalFmt = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		//const u32 mips = gen_mips ? calc_mip_count(w, h) : 1;
		//glTextureStorage2D(tex, mips, internalFmt, w, h);

		//glTextureSubImage2D(tex, 0, 0, 0, w, h, GL_RGBA8, GL_UNSIGNED_BYTE, pixels);

		//if (gen_mips && mips > 1) {
		//	glGenerateTextureMipmap(tex);
		//}

		//out_mip_levels = mips;
		//return static_cast<u64>(tex);


		GLuint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		const GLenum internalFmt = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		const u32 mips = gen_mips ? calc_mip_count(w, h) : 1;
		glTexStorage2D(GL_TEXTURE_2D, mips, internalFmt, w, h);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (gen_mips && mips > 1) {
			glGenerateTextureMipmap(tex);
		}

		out_mip_levels = mips;

		return static_cast<u64>(tex);
	}

	void Texture::destroy_gpu_texture(uint64_t handle) {
		if (handle == 0) return;

		GLuint tex = static_cast<GLuint>(handle);
		glDeleteTextures(1, &tex);
	}

    std::optional<Texture> Texture::load_from_file(const std::filesystem::path& path,
												   const TextureDesc& desc) {

		int w = 0, h = 0, c;
		auto pixels = load_pixels_rgba8(path, desc.flip_verticals, w, h, c);
		if (!pixels) {
			LOG_TRACE("Failed to load texture from: %s", path.string().c_str());
			return std::nullopt;
		}

		u32 mip_levels = 0;
		const u64 handle = create_gpu_texture_rgba8(pixels->data(), 
												    static_cast<u32>(w), 
													static_cast<u32>(h), 
													desc.srgb, 
													desc.generate_mips,
													mip_levels);

		if (handle == 0) {
			LOG_TRACE("Failed to generate texture handle");
			return std::nullopt;
		}

		return Texture{ handle, static_cast<u32>(w), static_cast<u32>(h), mip_levels, desc.srgb };
	}

	std::optional<Texture> Texture::alloc_storage_on_gpu(const int w, const int h) {

		GLuint tex = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		glTextureStorage2D(tex, 1, GL_RGB8, w, h);

		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return Texture{ tex, static_cast<u32>(w), static_cast<u32>(h), static_cast<u32>(1), true};
	}
}