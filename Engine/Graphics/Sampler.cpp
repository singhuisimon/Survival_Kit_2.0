/**
 * @file Sampler.cpp
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
#include "../Graphics/Sampler.h"

namespace Engine {

	static GLenum to_gl_wrap(Wrap w) {

		switch (w) {
		case Wrap::CLAMP:  return GL_CLAMP_TO_EDGE;
		case Wrap::REPEAT: return GL_REPEAT;
		case Wrap::MIRROR: return GL_MIRRORED_REPEAT;

		}
		return GL_REPEAT;
	}

	void set_gl_filter(GLuint s, Filter minf, Filter magf, MipFilter mipf) {
		// Compose MIN + MIP into one GL enum:
		GLint minParam = (minf == Filter::NEAREST) ? GL_NEAREST : GL_LINEAR;

		switch (mipf) {

		case MipFilter::NONE: break;
		case MipFilter::NEAREST: minParam = (minParam == GL_NEAREST) ? GL_NEAREST_MIPMAP_NEAREST
			: GL_LINEAR_MIPMAP_NEAREST; break;
		case MipFilter::LINEAR:  minParam = (minParam == GL_NEAREST) ? GL_NEAREST_MIPMAP_LINEAR
			: GL_LINEAR_MIPMAP_LINEAR;  break;
		}

		glSamplerParameteri(s, GL_TEXTURE_MIN_FILTER, minParam);
		glSamplerParameteri(s, GL_TEXTURE_MAG_FILTER, (magf == Filter::NEAREST) ? GL_NEAREST : GL_LINEAR);
	}

	void set_gl_anisotropy(GLuint s, u32 requested) {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
		if (reqeusted > 1) {
			GLfloat maxSupported = 1.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxSupported);
			const GLfloat val = std::min(static_cast<GLfloat>(requested), maxSupported);
			glSamplerParameterf(s, GL_TEXTURE_MAX_ANISOTROPY_EXT, val);
	}
#else
		(void)s, (void)requested;
#endif
	}

	std::optional<Sampler> Sampler::create(const SamplerDesc& d) {
		const u64 h = create_gpu_sampler(d);
		if (h == 0) return std::nullopt;
		return Sampler{ h };
	}

	uint64_t Sampler::create_gpu_sampler(const SamplerDesc& d) {

		GLuint s = 0;
		glCreateSamplers(1, &s);
		if (s == 0) return 0;

		// Wrap (S, T, R)
		glSamplerParameteri(s, GL_TEXTURE_WRAP_S, to_gl_wrap(d.wrap_u));
		glSamplerParameteri(s, GL_TEXTURE_WRAP_T, to_gl_wrap(d.wrap_v));
		glSamplerParameteri(s, GL_TEXTURE_WRAP_R, to_gl_wrap(d.wrap_w));

		// Filtering
		set_gl_filter(s, d.min_filter, d.mag_filter, d.mip_filter);

		// Anisotropy
		set_gl_anisotropy(s, d.max_anisotropy);

		return static_cast<uint64_t>(s);
	}

	void Sampler::destroy_gpu_sampler(uint64_t handle) {
		if (handle == kInvalid) return;
		const GLuint s = static_cast<GLuint>(handle);
		glDeleteSamplers(1, &s);
	}
}