/**
 * @file RenderBypassUtils.cpp
 * @brief Contains utility functions that are essential in setting up the renderer.
 * @details This file contains a collection of utility functions that are essential
 *			in setting up the renderer. These functions bypass the asset pipeline for
 *			for fast iteration.
 * @author Tan Jun Rui
 * @date 09 January 2026
 * Copyright (C) 2026 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "../Graphics/stb_image.h"
#include "../Graphics/RenderBypassUtils.h"
#include "../Graphics/Primitives.h"
#include "../Utility/AssetPath.h"

namespace Engine {

	/**
	 * @brief	  Loads shader programs from strings in the form of shader filepath.
	 * @param[in] shaders
				  An array containing tuples where each tuple contains the respective
	 *			  vertex and fragment shader filepaths.
	 * @return	  An array of ShaderPrograms.
	 */
	std::vector<ShaderProgram> RenderBypassUtils::loadShaderFromStrings(std::vector<std::pair<std::string, std::string>> shaders) {
		std::vector<ShaderProgram> shadersStorage;

		for (auto const& file : shaders) {
			// Create the shader files vector with types 
			std::vector<std::pair<GLenum, std::string>> shader_files;
			shader_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, file.first));
			shader_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, file.second));

			// Create new shader program
			ShaderProgram shader_program;

			// Use Graphics_Manager to compile the shader
			if (!shader_program.compileShader(shader_files)) {
				throw std::runtime_error("failed to compile shaders");
			}

			// Insert shader program into vector
			shadersStorage.emplace_back(shader_program);
		}

		return shadersStorage;
	}

	/**
	 * @brief	      Constructs an array of string tuples and loads all shader programs from filepaths.
	 * @param[in,out] shd
	 *				  An array of compiled ShaderPrograms.
	 * @return		  None.
	 */
	void RenderBypassUtils::loadAllShaderPrograms(std::vector<ShaderProgram>& shd) {
		std::string vertex_obj_path{ getAssetFilePath("Sources/Shaders/survival_kit_obj.vert") };
		std::string fragment_obj_path{ getAssetFilePath("Sources/Shaders/survival_kit_obj.frag") };

		std::string vertex_debug_path{ getAssetFilePath("Sources/Shaders/debug.vert") };
		std::string fragment_debug_path{ getAssetFilePath("Sources/Shaders/debug.frag") };

		std::string vertex_obj_picking_path{ getAssetFilePath("Sources/Shaders/object_picking.vert") };
		std::string fragment_obj_picking_path{ getAssetFilePath("Sources/Shaders/object_picking.frag") };

		std::string vertex_skybox_path{ getAssetFilePath("Sources/Shaders/skybox.vert") };
		std::string fragment_skybox_path{ getAssetFilePath("Sources/Shaders/skybox.frag") };

		std::string vertex_hdr_path{ getAssetFilePath("Sources/Shaders/hdr.vert") };
		std::string fragment_hdr_path{ getAssetFilePath("Sources/Shaders/hdr.frag") };

		std::string vertex_ui_path{ getAssetFilePath("Sources/Shaders/ui.vert") };
		std::string fragment_ui_path{ getAssetFilePath("Sources/Shaders/ui.frag") };

		std::string fragment_bloom_downsample_path{ getAssetFilePath("Sources/Shaders/bloom_downsample.frag") };
		std::string fragment_bloom_upsample_path{ getAssetFilePath("Sources/Shaders/bloom_upsample.frag") };

		std::string vertex_shadow_depth_path{ Engine::getAssetFilePath("Sources/Shaders/shadow_depth.vert") };
		std::string fragment_shadow_depth_path{ Engine::getAssetFilePath("Sources/Shaders/shadow_depth.frag") };

		std::string vertex_font_path{ getAssetFilePath("Sources/Shaders/font.vert") };
		std::string fragment_font_path{ getAssetFilePath("Sources/Shaders/font.frag") };

		std::string vertex_trails_path{ getAssetFilePath("Sources/Shaders/trails.vert") };
		std::string fragment_trails_path{ getAssetFilePath("Sources/Shaders/trails.frag") };

		// Pair vertex and fragment shader files
		std::vector<std::pair<std::string, std::string>> shader_files{
			std::make_pair(vertex_obj_path, fragment_obj_path),
			std::make_pair(vertex_debug_path, fragment_debug_path),
			std::make_pair(vertex_obj_picking_path, fragment_obj_picking_path),
			std::make_pair(vertex_skybox_path, fragment_skybox_path),
			std::make_pair(vertex_hdr_path, fragment_hdr_path),
			std::make_pair(vertex_ui_path, fragment_ui_path),
			std::make_pair(vertex_hdr_path, fragment_bloom_downsample_path),
			std::make_pair(vertex_hdr_path, fragment_bloom_upsample_path),
			std::make_pair(vertex_shadow_depth_path, fragment_shadow_depth_path),
			std::make_pair(vertex_font_path, fragment_font_path),
			std::make_pair(vertex_trails_path, fragment_trails_path)
		};

		shd = loadShaderFromStrings(shader_files);
	}

	/**
	 * @brief	      Loads basic primitives.
	 * @param[in,out] ms
	 *				  An array of GPU-side 3D mesh data.
	 * @param[in,out] md
	 *				  An array of CPU-side mesh data of varying dimensions.
	 * @param[in,out] md2d
	 *				  An array of GPU-side 2D mesh data.
	 * @return		  None.
	 */
	void RenderBypassUtils::loadBasicPrimitives(std::vector<MeshGL>& ms, std::vector<MeshData>& md, std::vector<MeshData2D>& md2d) {
		MeshData cd = make_cube();
		MeshData pd = make_plane();
		MeshData sd = make_sphere();
		MeshData2D qd = make_quad();

		md.push_back(cd);
		md.push_back(pd);
		md.push_back(sd);
		md2d.push_back(qd);

		MeshGL c = upload_mesh_data(cd);
		MeshGL p = upload_mesh_data(pd);
		MeshGL s = upload_mesh_data(sd);
		MeshGL q = upload_mesh_data2D(qd);

		ms.push_back(std::move(c));
		ms.push_back(std::move(p));
		ms.push_back(std::move(s));
		ms.push_back(std::move(q));
	}

	/**
	 * @brief	  Loads a cubemap.
	 * @param[in] faces
	 *			  An array of strings that contains filepaths of the six textures
	 *		      used to construct the cubemap.
	 * @return	  An unsigned integer that refers to the texture handle to the cubemap.
	 */
	unsigned int RenderBypassUtils::loadCubemap(std::vector<std::string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

	/**
	 * @brief	  Loads a cubemap in high dynamic color range.
	 * @return	  An unsigned integer that refers to the texture handle to the cubemap.
	 */
	unsigned int RenderBypassUtils::loadCubemapHDR()
	{
		std::vector<std::string> faces =
		{
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_001.png"),
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_002.png"),
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_003.png"),
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_004.png"),
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_005.png"),
				getAssetFilePath("Sources/Textures/Skybox_Engine_v1_006.png")
		};

		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			// Load as float data instead of unsigned char
			float* data = stbi_loadf(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				// Use HDR internal format
				GLenum internalFormat = (nrChannels == 4) ? GL_RGBA16F : GL_RGB16F;
				GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, internalFormat, width, height, 0, format, GL_FLOAT, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap HDR tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}
}