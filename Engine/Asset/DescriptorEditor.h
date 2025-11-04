/**
 * @file DescriptorEditor.h
 * @brief descriptor editing interface for editor ImGui integration
 * @details Provides simple property-based API for editing descriptor settings
 * @author 
 * @date 03/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#pragma once

#ifndef __DESCRIPTOR_EDITOR_H__
#define __DESCRIPTOR_EDITOR_H__

#include <string>
#include <variant>
#include "AssetDescriptorGenerator.h"
#include "AssetDatabase.h"

namespace Engine {

	class DescriptorEditor {
	public: 
		DescriptorEditor() = default;
		~DescriptorEditor() = default;

		//=======CORE============ 
		 /**
		 * @brief Load a descriptor for editing by asset GUID
		 * @param guid The asset's instance GUID
		 * @return true if descriptor loaded successfully
		 */
		bool Load(xresource::instance_guid guid);

		/**
		* @brief Load a descriptor by filename (e.g., "rock.png", "cube.fbx")
		* @param filename Just the filename with extension
		* @return true if descriptor loaded successfully
		*/
		bool LoadByFilename(const std::string& filename);

		/**
		* @brief Save current descriptor back to file
		* @return true if save succeeded
		*/
		bool Save();

		/**
		 * @brief Clear currently loaded descriptor
		 */
		void Clear();

		/**
		 * @brief Check if a descriptor is currently loaded
		 */
		bool IsLoaded() const { return m_isLoaded; }

		/**
		 * @brief Check if descriptor has unsaved changes
		 */
		bool IsModified() const { return m_isModified; }

		/**
		* @brief Mark descriptor as modified (call after changing settings)
		*/
		void MarkModified() { m_isModified = true; }

		//==================Property Getters=================
		 /**
		 * @brief Get the resource type of current descriptor
		 */
		ResourceType GetType() const { return m_currentType; }

		/**
		 * @brief Get the asset GUID
		 */
		xresource::instance_guid GetGuid() const { return m_currentGuid; }


		/**
		* @brief Get the source file path
		*/
		const std::string& GetSourcePath() const { return m_sourcePath; }

		/**
		 * @brief Get the filename (with extension) of the current asset
		 * @return Filename like "rock.png" or "cube.fbx"
		 */
		std::string GetFilename() const;

		/**
		 * @brief Get the display name from Info.txt
		 * @return Asset name (same as filename usually)
		 */
		std::string GetDisplayName() const { return m_displayName; }

		/**
		 * @brief Get GUID as hex string (from Info.txt)
		 * @return GUID instance hex string like "0E12B064084A801F"
		 */
		std::string GetGuidString() const { return m_guidString; }

		/**
		 * @brief Get tags from Info.txt
		 * @return Vector of tag strings
		 */
		const std::vector<std::string>& GetTags() const { return m_tags; }

		/**
		 * @brief Get last imported timestamp from Info.txt
		 * @return Timestamp value
		 */
		std::time_t GetLastImported() const { return m_lastImported; }

		/**
	   * @brief Get mutable TextureSettings pointer (nullptr if wrong type)
	   * @return Pointer to modify settings directly
	   */
		TextureSettings* GetTextureSettings();

		/**
		 * @brief Get mutable MeshSettings pointer (nullptr if wrong type)
		 */
		MeshSettings* GetMeshSettings();

		///**
		// * @brief Get mutable AudioSettings pointer (nullptr if wrong type)
		// */
		//AudioSettings* GetAudioSettings();

		///**
		// * @brief Get mutable ShaderSettings pointer (nullptr if wrong type)
		// */
		//ShaderSettings* GetShaderSettings();

		// ==================== VALIDATION OPTIONS ====================

		/**
		 * @brief Get valid options for texture usage type
		 * @return Vector of valid usage type strings
		 */
		static std::vector<std::string> GetUsageTypeOptions();

		/**
		 * @brief Get valid options for texture compression
		 */
		static std::vector<std::string> GetCompressionOptions();

		/**
		 * @brief Get valid options for mesh index type
		 */
		static std::vector<std::string> GetIndexTypeOptions();

		///**
		// * @brief Get valid options for audio output format
		// */
		//static std::vector<std::string> GetAudioFormatOptions();

		///**
		// * @brief Get valid options for audio compression
		// */
		//static std::vector<std::string> GetAudioCompressionOptions();

		///**
		// * @brief Get valid options for audio channel mode
		// */
		//static std::vector<std::string> GetChannelModeOptions();

		/**
		 * @brief Validate current settings (checks if values are valid)
		 * @param outErrors Optional vector to collect error messages
		 * @return true if settings are valid
		 */
		bool Validate(std::vector<std::string>* outErrors = nullptr) const;

		private: 

			bool LoadDescriptorFile(); 
			bool LoadInfoFile(); 
			bool SaveDescriptorFile(); 

			//JSON Parsing
			bool ParseTextureSettings(const std::string& json);
			bool ParseMeshSettings(const std::string& json);
			bool ParseAudioSettings(const std::string& json);
			bool ParseShaderSettings(const std::string& json);

			// JSON serialization
			std::string BuildTextureJson() const;
			std::string BuildMeshJson() const;
			std::string BuildAudioJson() const;
			std::string BuildShaderJson() const;

			// Validation
			bool ValidateTextureSettings(std::vector<std::string>* errors) const;
			bool ValidateMeshSettings(std::vector<std::string>* errors) const;
			bool ValidateAudioSettings(std::vector<std::string>* errors) const;
			bool ValidateShaderSettings(std::vector<std::string>* errors) const;

			//===============STATE=============================
			bool m_isLoaded = false;
			bool m_isModified = false;

			xresource::instance_guid m_currentGuid = xresource::instance_guid(0);
			ResourceType m_currentType = ResourceType::UNKNOWN;
			std::string m_sourcePath;
			std::string m_descriptorPath;
			std::string m_infoPath; 

			std::string m_displayName; 
			std::string m_guidString; 
			std::vector<std::string> m_tags;
			std::time_t m_lastImported = 0;

			// Settings storage (variant holds one type at a time)
			std::variant<
				std::monostate,      // Empty state
				TextureSettings,
				MeshSettings,
				AudioSettings,
				ShaderSettings
			> m_settings;
	};


}// end of namespace Engine 

#endif