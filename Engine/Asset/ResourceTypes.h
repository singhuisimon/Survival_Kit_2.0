/**
 * @file ResourceTypes.h
 * @brief Resource type definitions and utilities for the asset pipeline
 * @details
 * Provides the ResourceType enum and utility functions for type identification,
 * conversion, and file extension mapping. Integrates with the xresource::guid
 * system for compile-time type-safe resource identification.
 * 
 * @author Wai Lwin Thit
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
	/*ENTITY_PREFAB,
	SCENE_PREFAB,*/
	PREFAB,
	SCENE,
	ANIMATIONCLIPS,
	AUDIOSETTING,
	FONTS,
	ANIMATIONCONTROLLERS,
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
	/*case ResourceType::ENTITY_PREFAB:
		return "EntityPrefab";
	case ResourceType::SCENE_PREFAB:
		return "ScenePrefab";*/
	case ResourceType::PREFAB:
		return "Prefab";
	case ResourceType::SCENE:
		return "Scene";
	case ResourceType::ANIMATIONCLIPS:
		return "AnimationClips";
	case ResourceType::ANIMATIONCONTROLLERS:
		return "AnimationControllers";
	case ResourceType::FONTS:
		return "Fonts";
	case ResourceType::AUDIOSETTING:
		return "AudioSetting";
	default:
		return "Unknown";
	}
}
/**
 * @brief Convert string to ResourceType enum.
 * @param type_str The string to convert.
 * @return ResourceType enum value.
 */
inline ResourceType stringToResourceType(const std::string& type_str) {
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
	/*if (type_str == "EntityPrefab")
		return ResourceType::ENTITY_PREFAB;
	if (type_str == "ScenePrefab")
		return ResourceType::SCENE_PREFAB;*/
	if (type_str == "Prefab")
		return ResourceType::PREFAB;
	if (type_str == "Scene")
		return ResourceType::SCENE;
	if (type_str == "AnimationClips")
		return ResourceType::ANIMATIONCLIPS;
	if (type_str == "AnimationControllers")
		return ResourceType::ANIMATIONCONTROLLERS;
	if (type_str == "Fonts")
		return ResourceType::FONTS;
	if (type_str == "AudioSetting")
		return ResourceType::AUDIOSETTING;
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

	/*case ResourceType::ENTITY_PREFAB:
		return { "eprefab" };

	case ResourceType::SCENE_PREFAB:
		return { "sprefab" };*/
	case ResourceType::PREFAB:
		return { "prefab" };
	case ResourceType::SCENE:
		return { "json" };
	case ResourceType::ANIMATIONCLIPS:
		return { "animclip" };
	case ResourceType::ANIMATIONCONTROLLERS:
		return { "animcontroller" };
	case ResourceType::FONTS:
		return { "ttf" };
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
	for (int i = 0; i <= static_cast<int>(ResourceType::UNKNOWN); ++i) {
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
	inline static constexpr auto entity_prefab_type_guid_v = xresource::type_guid("entity_prefab");
	inline static constexpr auto scene_prefab_type_guid_v = xresource::type_guid("scene_prefab");

	//get the type guid for a resource type
	inline xresource::type_guid getTypeGUID(ResourceType type) {
		switch (type) {
		case ResourceType::TEXTURE:  return texture_type_guid_v;
		case ResourceType::MESH:     return mesh_type_guid_v;
		case ResourceType::MATERIAL: return material_type_guid_v;
		case ResourceType::AUDIO:    return audio_type_guid_v;
		case ResourceType::SHADER:   return shader_type_guid_v;
		/*case ResourceType::ENTITY_PREFAB: return entity_prefab_type_guid_v;
		case ResourceType::SCENE_PREFAB:  return scene_prefab_type_guid_v;*/
		default:                     return xresource::type_guid{};
		}
	}
}


}//end of namespace Engine


#endif // !_RESOURCE_TYPES_H