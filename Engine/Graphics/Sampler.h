/**
 * @file Sampler.h
 * @brief GPU sampler state management for texture filtering and wrapping
 * @details Provides RAII wrappers for OpenGL 4.6 sampler objects, which define
 *          how textures are sampled (filtering, wrapping, anisotropy). Sampler
 *          objects separate sampling parameters from texture data, allowing
 *          the same texture to be sampled differently in various contexts.
 * @author Tan Jun Rui
 * @date 05 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include <optional>

#include "../Utility/Types.h"

namespace Engine {

    /**
     * @brief Texture coordinate wrapping modes
     */
	enum class Wrap : uint8_t {
		CLAMP = 0,
		REPEAT,
		MIRROR
	};

    /**
     * @brief Texture minification and magnification filter modes
     */
	enum class Filter : uint8_t {
		NEAREST = 0,
		LINEAR
	};

    /**
     * @brief Mipmap filtering modes
     */
	enum class MipFilter : uint8_t {
		NONE = 0,
		NEAREST,
		LINEAR
	};

    /**
     * @brief Configuration for sampler state creation
     */
    struct SamplerDesc {
        Wrap  wrap_u = Wrap::REPEAT;
        Wrap  wrap_v = Wrap::REPEAT;
        Wrap  wrap_w = Wrap::REPEAT; // used for 3D textures/cubemaps
        Filter min_filter = Filter::LINEAR;
        Filter mag_filter = Filter::LINEAR;
        MipFilter mip_filter = MipFilter::LINEAR;
        uint32_t max_anisotropy = 1; // 1 = off
    };

    /**
     * @brief RAII wrapper for GPU sampler objects
     * @details Move-only class managing OpenGL sampler state lifetime. Samplers
     *          define texture filtering and wrapping independently from texture data.
     */
    class Sampler {
    public:
        Sampler() = delete;

        /**
         * @brief Creates a sampler with the specified configuration
         * @param desc Sampler state description (wrapping, filtering, anisotropy)
         * @return Optional containing sampler if successful, nullopt on failure
         */
        static std::optional<Sampler> create(const SamplerDesc& desc);

        // Move semantics only (no copying GPU resources)
        Sampler(Sampler&& o) noexcept { move_from(o); }
        Sampler& operator=(Sampler&& o) noexcept {
            if (this != &o) { destroy(); move_from(o); }
            return *this;
        }

        /**
         * @brief Destructor that releases GPU sampler resources
         */
        ~Sampler() noexcept { destroy(); }

        /**
         * @brief Gets the underlying OpenGL sampler handle
         * @return 64-bit sampler handle (can be cast to GLuint)
         */
        uint64_t handle() const noexcept { return m_handle; }

        /**
         * @brief Checks if the sampler has a valid GPU handle
         * @return True if valid, false if moved-from or failed to create
         */
        bool     valid()  const noexcept { return m_handle != kInvalid; }

    private:

        /**
         * @brief Private constructor used by factory method
         * @param h OpenGL sampler handle
         */
        explicit Sampler(uint64_t h) noexcept : m_handle(h) {}

        /**
         * @brief Creates an OpenGL sampler object with specified state
         * @param d Sampler description defining filtering and wrapping
         * @return OpenGL sampler handle
         */
        static uint64_t create_gpu_sampler(const SamplerDesc& d);

        /**
         * @brief Destroys an OpenGL sampler object
         * @param handle OpenGL sampler handle to delete
         */
        static void     destroy_gpu_sampler(uint64_t handle);

        /**
         * @brief Releases GPU resources and resets to invalid state
         */
        void destroy() noexcept {
            if (m_handle != kInvalid) { destroy_gpu_sampler(m_handle); m_handle = kInvalid; }
        }

        /**
         * @brief Transfers ownership from another sampler (move helper)
         * @param o Source sampler to move from
         */
        void move_from(Sampler& o) noexcept {
            m_handle = o.m_handle; o.m_handle = kInvalid;
        }

        // Member variables
        static constexpr uint64_t kInvalid = 0;
        uint64_t m_handle = kInvalid;  // e.g., GL sampler object, D3D sampler state, VkSampler
    };
}

