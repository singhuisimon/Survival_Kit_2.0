/**
 * @file Texture.h
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
#pragma once

#include <cstdint>
#include <optional>
#include <filesystem>
#include <string>

namespace Engine {

	/**
	 * @brief Configuration options for texture loading
	 */
	struct TextureDesc {
		bool srgb = true;
		bool generate_mips = true;
		bool flip_verticals = true;
	};

	/**
	 * @brief RAII wrapper for GPU texture resources
	 * @details Move-only class that manages the lifetime of OpenGL texture objects.
	 *          Textures must be created via factory methods to ensure valid state.
	 */
	class Texture {
	public:

		// Disable default ctor, construct textures only via factory method
		Texture() = delete;

		/**
		 * @brief Loads a texture from an image file on disk
		 * @param path Filesystem path to the image file (supports PNG, JPG, etc.)
		 * @param desc Loading options (color space, mipmaps, flip)
		 * @return Optional containing the texture if successful, nullopt on failure
		 */
		static std::optional<Texture> load_from_file(const std::filesystem::path& path,
													 const TextureDesc& desc);

		/**
		 * @brief Allocates empty texture storage on the GPU
		 * @param w Width of the texture in pixels
		 * @param h Height of the texture in pixels
		 * @return Optional containing the texture if successful, nullopt on failure
		 */
		static std::optional<Texture> alloc_storage_on_gpu(const int w, const int h);

		Texture(Texture&& other) noexcept { move_from(other); }
		Texture& operator=(Texture&& other) noexcept {
			if (this != &other) { destroy(); move_from(other); }
			return *this;
		}

		/**
		 * @brief Destructor that releases GPU texture resources
		 */
		~Texture() noexcept { destroy(); }

		/**
		 * @brief Gets the texture width in pixels
		 * @return Width of the texture
		 */
		uint32_t width()  const noexcept { return m_width; }

		/**
		 * @brief Gets the texture height in pixels
		 * @return Height of the texture
		 */
		uint32_t height() const noexcept { return m_height; }

		/**
		 * @brief Gets the number of mipmap levels
		 * @return Mipmap level count (1 if no mipmaps)
		 */
		uint32_t mips()   const noexcept { return m_mip_levels; }

		/**
		 * @brief Checks if the texture uses sRGB color space
		 * @return True if sRGB, false if linear
		 */
		bool     is_srgb()const noexcept { return m_srgb; }

		/**
		 * @brief Checks if the texture has a valid GPU handle
		 * @return True if valid, false if moved-from or failed to create
		 */
		bool     valid()  const noexcept { return m_handle != kInvalid; }

		/**
		 * @brief Gets the underlying OpenGL texture handle
		 * @return 64-bit texture handle (can be cast to GLuint)
		 */
		uint64_t handle() const noexcept { return m_handle; }

	private:

		/**
		 * @brief Private constructor used by factory methods
		 * @param handle OpenGL texture handle
		 * @param w Width in pixels
		 * @param h Height in pixels
		 * @param mip_levels Number of mipmap levels
		 * @param srgb Whether texture uses sRGB color space
		 */
		Texture(uint64_t handle, uint32_t w, uint32_t h, uint32_t mip_levels, bool srgb) noexcept
			: m_handle(handle), m_width(w), m_height(h), m_mip_levels(mip_levels), m_srgb(srgb) { }

		/**
		 * @brief Loads image file into CPU memory as RGBA8 pixels
		 * @param path Path to image file
		 * @param flip_vertical Whether to flip image vertically
		 * @param out_w Output width
		 * @param out_h Output height
		 * @param out_channels Output channel count
		 * @return Optional containing pixel data if successful
		 */
		static std::optional<std::vector<uint8_t>> load_pixels_rgba8(const std::filesystem::path& path,
																	 bool flip_vertical,
																	 int& out_w, int& out_h, int& out_channels);

		/**
		 * @brief Creates an OpenGL texture and uploads pixel data
		 * @param pixels RGBA8 pixel data
		 * @param w Width in pixels
		 * @param h Height in pixels
		 * @param srgb Use sRGB internal format
		 * @param gen_mips Generate mipmap chain
		 * @param out_mip_levels Output number of mip levels created
		 * @return OpenGL texture handle
		 */
		static uint64_t create_gpu_texture_rgba8(const uint8_t* pixels, uint32_t w, uint32_t h,
												 bool srgb, bool gen_mips, uint32_t& out_mip_levels);

		/**
		 * @brief Destroys an OpenGL texture object
		 * @param handle OpenGL texture handle to delete
		 */
		static void     destroy_gpu_texture(uint64_t handle);

		/**
		 * @brief Releases GPU resources and resets to invalid state
		 */
		void destroy() noexcept {
			if (m_handle != kInvalid) { destroy_gpu_texture(m_handle); m_handle = kInvalid; }
			m_width = m_height = m_mip_levels = 0; m_srgb = false;
		}

		/**
		 * @brief Transfers ownership from another texture (move helper)
		 * @param o Source texture to move from
		 */
		void move_from(Texture& o) noexcept {
			m_handle = o.m_handle; m_width = o.m_width; m_height = o.m_height;
			m_mip_levels = o.m_mip_levels; m_srgb = o.m_srgb;
			o.m_handle = kInvalid; o.m_width = o.m_height = o.m_mip_levels = 0; o.m_srgb = false;
		}

		// Data
		static constexpr uint64_t kInvalid = 0;
		uint64_t m_handle = kInvalid;      // e.g., GL texture id, D3D SRV handle, VkImageView+layout token, etc.
		uint32_t m_width = 0, m_height = 0, m_mip_levels = 0;
		bool     m_srgb = false;
	};
}

