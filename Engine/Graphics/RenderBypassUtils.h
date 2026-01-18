/**
 * @file RenderBypassUtils.h
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

#pragma once
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/MeshData.h"
#include <vector>
#include <cfloat> // FLT_MAX, etc

namespace Engine {

	namespace RenderBypassUtils {

		/**
		 * @brief	  Loads shader programs from strings in the form of shader filepath.
		 * @param[in] shaders 
					  An array containing tuples where each tuple contains the respective
		 *			  vertex and fragment shader filepaths.
		 * @return	  An array of ShaderPrograms.
		 */
		std::vector<ShaderProgram> loadShaderFromStrings(std::vector<std::pair<std::string, std::string>> shaders);

		/**
		 * @brief	      Constructs an array of string tuples and loads all shader programs from filepaths.
		 * @param[in,out] shd
		 *				  An array of compiled ShaderPrograms.
		 * @return		  None.
		 */
		void loadAllShaderPrograms(std::vector<ShaderProgram>& shd);

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
		void loadBasicPrimitives(std::vector<MeshGL>& ms, std::vector<MeshData>& md, std::vector<MeshData2D>& md2d);

		/**
		 * @brief	  Loads a cubemap.
		 * @param[in] faces
		 *			  An array of strings that contains filepaths of the six textures 
		 *		      used to construct the cubemap.
		 * @return	  An unsigned integer that refers to the texture handle to the cubemap.
		 */
		unsigned int loadCubemap(std::vector<std::string> faces);

		/**
		 * @brief	  Loads a cubemap in high dynamic color range.
		 * @return	  An unsigned integer that refers to the texture handle to the cubemap.
		 */
		unsigned int loadCubemapHDR();
	};
}