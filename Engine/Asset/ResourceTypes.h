/**
* @file ResourceTypes.h
* @brief 
* @author 
* @date 15/09/2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/



#pragma once
#ifndef _RESOURCE_TYPES_H
#define _RESOURCE_TYPES_H

//external libraries
#include "../xresource_guid/include/xresource_guid.h"


//c++ libraries
#include <string>
#include <vector>
#include <algorithm>


namespace Engine {

	/*
	* @brief enum for resource types
	*/
	enum class ResourceType {
		TEXTURE = 0, 
		MESH,
		MATERIAL,
		AUDIO,
		SHADER,
		UNKNOWN 
	};

	/**
	 * @brief Convert ResourceType enum to string.
	 * @param type The resource type to convert.
	 * @return String representation of the resource type.
	 */
	inline std::string resourceTypeToString(ResourceType type) {
		switch (type) {
		case ResourceType::TEXTURE: 
			return "Texture";
		case ResourceType::MESH: 
			return "Mesh";
		case ResourceType::MATERIAL: 
			return "Material";
		case ResourceType::AUDIO: 
			return "Audio";
		case ResourceType::SHADER: 
			return "Shader";
		default: 
			return "Unknown";
		}
	}
	/**
	 * @brief Convert string to ResourceType enum.
	 * @param type_str The string to convert.
	 * @return ResourceType enum value.
	 */
	inline ResourceType stringToResourceType(const std::string& type_str)  {
		if (type_str == "Texture")
			return ResourceType::TEXTURE;
		if (type_str == "Mesh") 
			return ResourceType::MESH;
		if (type_str == "Material") 
			return ResourceType::MATERIAL;
		if (type_str == "Audio") 
			return ResourceType::AUDIO;
		if (type_str == "Shader") 
			return ResourceType::SHADER;
		return ResourceType::UNKNOWN;
	}
	/**
	 * @brief Get file extensions associated with a resource type.
	 * @param type The resource type.
	 * @return Vector of supported file extensions (without dots).
	 */
	inline std::vector<std::string> getResourceTypeExtension(ResourceType type) {

		switch (type) {

		case ResourceType::TEXTURE:
			return { "png", "jpg", "jpeg", "tga", "bmp", "psd" };

		case ResourceType::MESH:
			return { "obj", "fbx", "dae", "3ds", "blend" };

		case ResourceType::MATERIAL:
			return { "mtl", "mat" };

		case ResourceType::AUDIO:
			return { "wav", "mp3", "ogg", "flac" };

		case ResourceType::SHADER:
			return { "glsl", "vert", "frag", "hlsl" };

		default: 
			return {};

		}
	}
	/**
	 * @brief Detect resource type from file extension.
	 * @param file_path The file path to analyze.
	 * @return Detected resource type, or UNKNOWN if not recognized.
	 */
	inline ResourceType detectResourceTypeFromPath(const std::string& file_path) {
		//extract extension
		size_t dot_pos = file_path.find_last_of('.');
		if (dot_pos == std::string::npos) {
			return ResourceType::UNKNOWN;
		}

		//get the extension without the dot
		std::string file_extension = file_path.substr(dot_pos + 1);

		//convert to lowercase for comparison
		std::transform(file_extension.begin(), file_extension.end(), file_extension.begin(), ::tolower);
	
		//check resource type
		for (int i = 0; i <= static_cast<int>(ResourceType::SHADER); ++i) {
			ResourceType type = static_cast<ResourceType>(i);
			auto extensions = getResourceTypeExtension(type); //get the string of the extensions

			//compare with the file_extension
			for (const auto& ext : extensions) {
				if (file_extension == ext) {
					return type; //return the ResourceType
				}
			}
		}

		return ResourceType::UNKNOWN;
	
	}

	//type safe guid extension using xresource_guid
	namespace ResourceGUID {

		//generate compile time type guids for each resource type
		inline static constexpr auto texture_type_guid_v = xresource::type_guid("texture");
		inline static constexpr auto mesh_type_guid_v = xresource::type_guid("mesh");
		inline static constexpr auto material_type_guid_v = xresource::type_guid("material");
		inline static constexpr auto audio_type_guid_v = xresource::type_guid("audio");
		inline static constexpr auto shader_type_guid_v = xresource::type_guid("shader");

		//type safe guid alias 
		using texture_guid = xresource::def_guid<texture_type_guid_v>;
		using mesh_guid = xresource::def_guid<mesh_type_guid_v>;
		using material_guid = xresource::def_guid<material_type_guid_v>;
		using audio_guid = xresource::def_guid<audio_type_guid_v>;
		using shader_guid = xresource::def_guid<shader_type_guid_v>;

		//get the type guid for a resource type
		inline xresource::type_guid getTypeGUID(ResourceType type) {
			switch (type) {
				case ResourceType::TEXTURE:  return texture_type_guid_v;
				case ResourceType::MESH:     return mesh_type_guid_v;
				case ResourceType::MATERIAL: return material_type_guid_v;
				case ResourceType::AUDIO:    return audio_type_guid_v;
				case ResourceType::SHADER:   return shader_type_guid_v;
				default:                     return xresource::type_guid{};
			}
		}
	}

    //namespace std {
    //template<>
    //struct hash<xresource::instance_guid> {
    //    std::size_t operator()(const xresource::instance_guid& g) const noexcept {
    //        return std::hash<uint64_t>{}(g.m_Value);
    //    }
    //};


}//end of namespace Engine


#endif // !_RESOURCE_TYPES_H