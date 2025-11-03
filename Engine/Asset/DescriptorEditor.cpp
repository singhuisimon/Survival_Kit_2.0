/**
 * @file DescriptorEditor.cpp
 * @brief Implementation of  DescriptorEditor
 * @author
 * @date 03/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#include "DescriptorEditor.h"
#include "AssetManager.h"
#include "../Utility/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>

 //external libraries
 //#include "../rapidjson/document.h"
#include <rapidjson/document.h>
//#include "../rapidjson/writer.h"
#include <rapidjson/writer.h>
//#include "../rapidjson/stringbuffer.h"
#include <rapidjson/stringbuffer.h>
//#include "../rapidjson/istreamwrapper.h"
#include <rapidjson/istreamwrapper.h>
//#include "../rapidjson/prettywriter.h"
#include <rapidjson/prettywriter.h>

namespace fs = std::filesystem;

namespace Engine
{

	bool DescriptorEditor::Load(xresource::instance_guid guid)
	{
		//clear the previous load
		Clear();

		//get asset record from database
		const AssetRecord *rec = AM.db().Find(guid);
		if (!rec)
		{
			LOG_ERROR("DescriptorEditor: Asset GUID not found: ", std::hex, guid.m_Value);
			return false;
		}
		if (!rec->valid)
		{
			LOG_ERROR("DescriptorEditor: Asset is invalid: ", std::hex, guid.m_Value);
			return false;
		}

		//build path 
		 // Build descriptor path
		m_descriptorPath = AM.descriptorGenerator().GetDescriptorFolderPath(*rec) + "Descriptor.txt";
		m_infoPath = AM.descriptorGenerator().GetDescriptorFolderPath(*rec) + "Info.txt";

		// Check if files exist
		if (!fs::exists(m_descriptorPath))
		{
			LOG_ERROR("DescriptorEditor: Descriptor.txt not found: ", m_descriptorPath);
			return false;
		}

		if (!fs::exists(m_infoPath))
		{
			LOG_ERROR("DescriptorEditor: Info.txt not found: ", m_infoPath);
			return false;
		}

		// Store state
		m_currentGuid = guid;
		m_currentType = rec->type;
		m_sourcePath = rec->sourcePath;

		// Load Info.txt first (for display)
		if (!LoadInfoFile())
		{
			LOG_ERROR("DescriptorEditor: Failed to load Info.txt");
			Clear();
			return false;
		}

		// Load and parse Descriptor.txt
		if (!LoadDescriptorFile())
		{
			LOG_ERROR("DescriptorEditor: Failed to load descriptor");
			Clear();
			return false;
		}

		m_isLoaded = true;
		m_isModified = false;

		LOG_DEBUG("DescriptorEditor: Loaded descriptor for GUID ", std::hex, guid.m_Value);
		return true;
	}

	bool DescriptorEditor::LoadByFilename(const std::string &filename)
	{
		// Get GUID from filename using AssetManager
		xresource::instance_guid guid = AM.getGuidFromName(filename);

		if (guid.m_Value == 0)
		{
			LOG_ERROR("DescriptorEditor: Asset not found with filename: ", filename);
			return false;
		}

		// Use regular Load() method
		return Load(guid);
	}

	bool DescriptorEditor::Save()
	{
		if (!m_isLoaded)
		{
			LOG_ERROR("DescriptorEditor: No descriptor loaded");
			return false;
		}

		// Validate before saving
		std::vector<std::string> errors;
		if (!Validate(&errors))
		{
			LOG_ERROR("DescriptorEditor: Validation failed");
			for (const auto &err : errors)
			{
				LOG_ERROR("  - ", err);
			}
			return false;
		}

		// Save to file
		if (!SaveDescriptorFile())
		{
			LOG_ERROR("DescriptorEditor: Failed to save descriptor");
			return false;
		}

		// Update database timestamps
		AssetRecord *rec = AM.db().FindMutable(m_currentGuid);
		if (rec)
		{
			rec->descriptorModifiedTime = std::time(nullptr);
			rec->needsRecompile = true;
		}

		m_isModified = false;
		LOG_INFO("DescriptorEditor: Saved descriptor: ", m_descriptorPath);
		return true;
	}

	void DescriptorEditor::Clear()
	{
		m_isLoaded = false;
		m_isModified = false;
		m_currentGuid = xresource::instance_guid(0);
		m_currentType = ResourceType::UNKNOWN;
		m_sourcePath.clear();
		m_descriptorPath.clear();
		m_infoPath.clear();

		// Clear Info.txt data
		m_displayName.clear();
		m_guidString.clear();
		m_tags.clear();
		m_lastImported = 0;

		m_settings = std::monostate{};
	}

	// ==================== PROPERTY GETTERS ====================

	std::string DescriptorEditor::GetFilename() const
	{
		if (!m_isLoaded) return "";

		// Extract filename from source path
		size_t lastSlash = m_sourcePath.find_last_of("/\\");
		if (lastSlash != std::string::npos)
		{
			return m_sourcePath.substr(lastSlash + 1);
		}
		return m_sourcePath;
	}

	TextureSettings *DescriptorEditor::GetTextureSettings()
	{
		if (m_currentType != ResourceType::TEXTURE) return nullptr;
		return std::get_if<TextureSettings>(&m_settings);
	}

	MeshSettings *DescriptorEditor::GetMeshSettings()
	{
		if (m_currentType != ResourceType::MESH) return nullptr;
		return std::get_if<MeshSettings>(&m_settings);
	}

#if 0
	AudioSettings *DescriptorEditor::GetAudioSettings()
	{
		if (m_currentType != ResourceType::AUDIO) return nullptr;
		return std::get_if<AudioSettings>(&m_settings);
	}

	ShaderSettings *DescriptorEditor::GetShaderSettings()
	{
		if (m_currentType != ResourceType::SHADER) return nullptr;
		return std::get_if<ShaderSettings>(&m_settings);
	}

#endif
	// ==================== VALIDATION OPTIONS ====================

	std::vector<std::string> DescriptorEditor::GetUsageTypeOptions()
	{
		return { "COLOR", "NORMAL", "METALLIC", "ROUGHNESS", "AO", "EMISSIVE", "UI", "GENERIC" };
	}

	std::vector<std::string> DescriptorEditor::GetCompressionOptions()
	{
		return { "BC1", "BC3", "BC4", "BC5", "BC7", "None" };
	}

	std::vector<std::string> DescriptorEditor::GetIndexTypeOptions()
	{
		return { "UINT16", "UINT32" };
	}
#if 0
	std::vector<std::string> DescriptorEditor::GetAudioFormatOptions()
	{
		return { "OGG", "WAV" };
	}

	std::vector<std::string> DescriptorEditor::GetAudioCompressionOptions()
	{
		return { "VORBIS", "PCM" };
	}

	std::vector<std::string> DescriptorEditor::GetChannelModeOptions()
	{
		return { "MONO", "STEREO" };
	}
#endif 
	// ==================== VALIDATION ====================

	bool DescriptorEditor::Validate(std::vector<std::string> *outErrors) const
	{
		if (!m_isLoaded)
		{
			if (outErrors) outErrors->push_back("No descriptor loaded");
			return false;
		}

		switch (m_currentType)
		{
		case ResourceType::TEXTURE:
			return ValidateTextureSettings(outErrors);
		case ResourceType::MESH:
			return ValidateMeshSettings(outErrors);
		case ResourceType::AUDIO:
			return ValidateAudioSettings(outErrors);
		case ResourceType::SHADER:
			return ValidateShaderSettings(outErrors);
		default:
			if (outErrors) outErrors->push_back("Unknown resource type");
			return false;
		}
	}

	bool DescriptorEditor::ValidateTextureSettings(std::vector<std::string> *errors) const
	{
		const TextureSettings *settings = std::get_if<TextureSettings>(&m_settings);
		if (!settings)
		{
			if (errors) errors->push_back("Invalid settings variant");
			return false;
		}

		bool valid = true;

		// Validate quality
		if (settings->quality < 0.0f || settings->quality > 1.0f)
		{
			if (errors) errors->push_back("Quality must be between 0.0 and 1.0");
			valid = false;
		}

		// Validate usage type
		auto usageOptions = GetUsageTypeOptions();
		if (std::find(usageOptions.begin(), usageOptions.end(), settings->usageType) == usageOptions.end())
		{
			if (errors) errors->push_back("Invalid usage type: " + settings->usageType);
			valid = false;
		}

		// Validate compression
		auto compOptions = GetCompressionOptions();
		if (std::find(compOptions.begin(), compOptions.end(), settings->compression) == compOptions.end())
		{
			if (errors) errors->push_back("Invalid compression: " + settings->compression);
			valid = false;
		}

		return valid;
	}

	bool DescriptorEditor::ValidateMeshSettings(std::vector<std::string> *errors) const
	{
		const MeshSettings *settings = std::get_if<MeshSettings>(&m_settings);
		if (!settings)
		{
			if (errors) errors->push_back("Invalid settings variant");
			return false;
		}

		bool valid = true;

		// Validate index type
		auto indexOptions = GetIndexTypeOptions();
		if (std::find(indexOptions.begin(), indexOptions.end(), settings->indexType) == indexOptions.end())
		{
			if (errors) errors->push_back("Invalid index type: " + settings->indexType);
			valid = false;
		}

		// Validate scale
		if (settings->scale <= 0.0f)
		{
			if (errors) errors->push_back("Scale must be positive");
			valid = false;
		}

		return valid;
	}

#if 0

	bool DescriptorEditor::ValidateAudioSettings(std::vector<std::string> *errors) const
	{
		const AudioSettings *settings = std::get_if<AudioSettings>(&m_settings);
		if (!settings)
		{
			if (errors) errors->push_back("Invalid settings variant");
			return false;
		}

		bool valid = true;

		// Validate quality
		if (settings->quality < 0.0f || settings->quality > 1.0f)
		{
			if (errors) errors->push_back("Quality must be between 0.0 and 1.0");
			valid = false;
		}

		// Validate sample rate
		if (settings->sampleRate < 8000 || settings->sampleRate > 192000)
		{
			if (errors) errors->push_back("Sample rate must be between 8000 and 192000");
			valid = false;
		}

		// Validate output format
		auto formatOptions = GetAudioFormatOptions();
		if (std::find(formatOptions.begin(), formatOptions.end(), settings->outputFormat) == formatOptions.end())
		{
			if (errors) errors->push_back("Invalid output format: " + settings->outputFormat);
			valid = false;
		}

		return valid;
	}

	bool DescriptorEditor::ValidateShaderSettings(std::vector<std::string> *errors) const
	{
		const ShaderSettings *settings = std::get_if<ShaderSettings>(&m_settings);
		if (!settings)
		{
			if (errors) errors->push_back("Invalid settings variant");
			return false;
		}

		// Basic validation - shaders are less restrictive
		return true;
	}
#endif 

	// ==================== FILE I/O ====================

	bool DescriptorEditor::LoadInfoFile()
	{
		// Read Info.txt file
		std::ifstream file(m_infoPath);
		if (!file.is_open())
		{
			LOG_ERROR("DescriptorEditor: Cannot open Info.txt: ", m_infoPath);
			return false;
		}

		// Parse JSON
		rapidjson::IStreamWrapper isw(file);
		rapidjson::Document doc;
		doc.ParseStream(isw);
		file.close();

		if (doc.HasParseError())
		{
			LOG_ERROR("DescriptorEditor: JSON parse error in Info.txt at offset ", doc.GetErrorOffset());
			return false;
		}

		// Extract name
		if (doc.HasMember("name") && doc["name"].IsString())
		{
			m_displayName = doc["name"].GetString();
		}

		// Extract GUID instance as hex string
		if (doc.HasMember("guid") && doc["guid"].IsObject())
		{
			const auto &guidObj = doc["guid"];
			if (guidObj.HasMember("instance") && guidObj["instance"].IsString())
			{
				m_guidString = guidObj["instance"].GetString();
			}
		}

		// Extract tags
		if (doc.HasMember("tags") && doc["tags"].IsArray())
		{
			const auto &tagsArray = doc["tags"];
			m_tags.clear();
			for (rapidjson::SizeType i = 0; i < tagsArray.Size(); i++)
			{
				if (tagsArray[i].IsString())
				{
					m_tags.push_back(tagsArray[i].GetString());
				}
			}
		}

		// Extract lastImported (stored as hex)
		if (doc.HasMember("lastImported") && doc["lastImported"].IsString())
		{
			try
			{
				std::string hexTime = doc["lastImported"].GetString();
				m_lastImported = static_cast<std::time_t>(std::stoull(hexTime, nullptr, 16));
			}
			catch (...)
			{
				m_lastImported = 0;
			}
		}

		LOG_DEBUG("DescriptorEditor: Loaded Info.txt - Name: ", m_displayName);
		return true;
	}

	bool DescriptorEditor::LoadDescriptorFile()
	{
		// Read file
		std::ifstream file(m_descriptorPath);
		if (!file.is_open())
		{
			LOG_ERROR("DescriptorEditor: Cannot open file: ", m_descriptorPath);
			return false;
		}

		// Parse JSON
		rapidjson::IStreamWrapper isw(file);
		rapidjson::Document doc;
		doc.ParseStream(isw);
		file.close();

		if (doc.HasParseError())
		{
			LOG_ERROR("DescriptorEditor: JSON parse error at offset ", doc.GetErrorOffset());
			return false;
		}

		// Extract source path
		if (doc.HasMember("sourcePath") && doc["sourcePath"].IsString())
		{
			m_sourcePath = doc["sourcePath"].GetString();
		}

		// Convert document to string for type-specific parsing
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::string jsonStr = buffer.GetString();

		// Parse type-specific settings
		switch (m_currentType)
		{
		case ResourceType::TEXTURE:
			return ParseTextureSettings(jsonStr);
		case ResourceType::MESH:
			return ParseMeshSettings(jsonStr);
		case ResourceType::AUDIO:
			return ParseAudioSettings(jsonStr);
		case ResourceType::SHADER:
			return ParseShaderSettings(jsonStr);
		default:
			LOG_ERROR("DescriptorEditor: Unsupported resource type");
			return false;
		}
	}

	bool DescriptorEditor::SaveDescriptorFile()
	{
		// Build JSON based on type
		std::string json;
		switch (m_currentType)
		{
		case ResourceType::TEXTURE:
			json = BuildTextureJson();
			break;
		case ResourceType::MESH:
			json = BuildMeshJson();
			break;
		case ResourceType::AUDIO:
			json = BuildAudioJson();
			break;
		case ResourceType::SHADER:
			json = BuildShaderJson();
			break;
		default:
			LOG_ERROR("DescriptorEditor: Unsupported resource type");
			return false;
		}

		// Write to file
		std::ofstream file(m_descriptorPath);
		if (!file.is_open())
		{
			LOG_ERROR("DescriptorEditor: Cannot open file for writing: ", m_descriptorPath);
			return false;
		}

		file << json;
		file.close();

		return true;
	}

	// ==================== PARSING (Type-Specific) ====================

	bool DescriptorEditor::ParseTextureSettings(const std::string &json)
	{
		rapidjson::Document doc;
		doc.Parse(json.c_str());

		if (!doc.HasMember("textureSettings") || !doc["textureSettings"].IsObject())
		{
			LOG_ERROR("DescriptorEditor: No 'textureSettings' in descriptor");
			return false;
		}

		TextureSettings settings;
		const auto &ts = doc["textureSettings"];

		// Parse each field with defaults
		if (ts.HasMember("usageType") && ts["usageType"].IsString())
			settings.usageType = ts["usageType"].GetString();

		if (ts.HasMember("compression") && ts["compression"].IsString())
			settings.compression = ts["compression"].GetString();

		if (ts.HasMember("quality") && ts["quality"].IsNumber())
			settings.quality = ts["quality"].GetFloat();

		if (ts.HasMember("generateMipmaps") && ts["generateMipmaps"].IsBool())
			settings.generateMipmaps = ts["generateMipmaps"].GetBool();

		if (ts.HasMember("srgb") && ts["srgb"].IsBool())
			settings.srgb = ts["srgb"].GetBool();

		m_settings = settings;
		return true;
	}

	bool DescriptorEditor::ParseMeshSettings(const std::string &json)
	{
		rapidjson::Document doc;
		doc.Parse(json.c_str());

		if (!doc.HasMember("meshSettings") || !doc["meshSettings"].IsObject())
		{
			LOG_ERROR("DescriptorEditor: No 'meshSettings' in descriptor");
			return false;
		}

		MeshSettings settings;
		const auto &ms = doc["meshSettings"];

		if (ms.HasMember("outputFormat") && ms["outputFormat"].IsString())
			settings.outputFormat = ms["outputFormat"].GetString();

		if (ms.HasMember("includePos") && ms["includePos"].IsBool())
			settings.includePos = ms["includePos"].GetBool();

		if (ms.HasMember("includeNormals") && ms["includeNormals"].IsBool())
			settings.includeNormals = ms["includeNormals"].GetBool();

		if (ms.HasMember("includeColors") && ms["includeColors"].IsBool())
			settings.includeColors = ms["includeColors"].GetBool();

		if (ms.HasMember("includeTexCoords") && ms["includeTexCoords"].IsBool())
			settings.includeTexCoords = ms["includeTexCoords"].GetBool();

		if (ms.HasMember("indexType") && ms["indexType"].IsString())
			settings.indexType = ms["indexType"].GetString();

		if (ms.HasMember("scale") && ms["scale"].IsNumber())
			settings.scale = ms["scale"].GetFloat();

		if (ms.HasMember("optimizeVertices") && ms["optimizeVertices"].IsBool())
			settings.optimizeVertices = ms["optimizeVertices"].GetBool();

		if (ms.HasMember("generateNormals") && ms["generateNormals"].IsBool())
			settings.generateNormals = ms["generateNormals"].GetBool();

		m_settings = settings;
		return true;
	}

	bool DescriptorEditor::ParseAudioSettings(const std::string &json)
	{
		rapidjson::Document doc;
		doc.Parse(json.c_str());

		if (!doc.HasMember("audioSettings") || !doc["audioSettings"].IsObject())
		{
			LOG_ERROR("DescriptorEditor: No 'audioSettings' in descriptor");
			return false;
		}

		AudioSettings settings;
		const auto &as = doc["audioSettings"];

		if (as.HasMember("outputFormat") && as["outputFormat"].IsString())
			settings.outputFormat = as["outputFormat"].GetString();

		if (as.HasMember("compression") && as["compression"].IsString())
			settings.compression = as["compression"].GetString();

		if (as.HasMember("quality") && as["quality"].IsNumber())
			settings.quality = as["quality"].GetFloat();

		if (as.HasMember("sampleRate") && as["sampleRate"].IsInt())
			settings.sampleRate = as["sampleRate"].GetInt();

		if (as.HasMember("channelMode") && as["channelMode"].IsString())
			settings.channelMode = as["channelMode"].GetString();

		m_settings = settings;
		return true;
	}

	bool DescriptorEditor::ParseShaderSettings(const std::string &json)
	{
		rapidjson::Document doc;
		doc.Parse(json.c_str());

		if (!doc.HasMember("shaderSettings") || !doc["shaderSettings"].IsObject())
		{
			LOG_ERROR("DescriptorEditor: No 'shaderSettings' in descriptor");
			return false;
		}

		ShaderSettings settings;
		const auto &ss = doc["shaderSettings"];

		if (ss.HasMember("vertexShader") && ss["vertexShader"].IsString())
			settings.vertexShader = ss["vertexShader"].GetString();

		if (ss.HasMember("fragmentShader") && ss["fragmentShader"].IsString())
			settings.fragmentShader = ss["fragmentShader"].GetString();

		if (ss.HasMember("outputFormat") && ss["outputFormat"].IsString())
			settings.outputFormat = ss["outputFormat"].GetString();

		if (ss.HasMember("targetAPI") && ss["targetAPI"].IsString())
			settings.targetAPI = ss["targetAPI"].GetString();

		if (ss.HasMember("targetVersion") && ss["targetVersion"].IsString())
			settings.targetVersion = ss["targetVersion"].GetString();

		if (ss.HasMember("optimizationLevel") && ss["optimizationLevel"].IsString())
			settings.optimizationLevel = ss["optimizationLevel"].GetString();

		if (ss.HasMember("stripDebugInfo") && ss["stripDebugInfo"].IsBool())
			settings.stripDebugInfo = ss["stripDebugInfo"].GetBool();

		m_settings = settings;
		return true;
	}

	// ==================== SERIALIZATION (Type-Specific) ====================

	std::string DescriptorEditor::BuildTextureJson() const
	{
		const TextureSettings *settings = std::get_if<TextureSettings>(&m_settings);
		if (!settings) return "";

		rapidjson::Document doc;
		doc.SetObject();
		auto &allocator = doc.GetAllocator();

		// Add sourcePath
		doc.AddMember("sourcePath", rapidjson::Value(m_sourcePath.c_str(), allocator), allocator);

		// Add textureSettings
		rapidjson::Value tsObj(rapidjson::kObjectType);
		tsObj.AddMember("usageType", rapidjson::Value(settings->usageType.c_str(), allocator), allocator);
		tsObj.AddMember("compression", rapidjson::Value(settings->compression.c_str(), allocator), allocator);
		tsObj.AddMember("quality", settings->quality, allocator);
		tsObj.AddMember("generateMipmaps", settings->generateMipmaps, allocator);
		tsObj.AddMember("srgb", settings->srgb, allocator);

		doc.AddMember("textureSettings", tsObj, allocator);

		// Convert to string
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		return buffer.GetString();
	}

	std::string DescriptorEditor::BuildMeshJson() const
	{
		const MeshSettings *settings = std::get_if<MeshSettings>(&m_settings);
		if (!settings) return "";

		rapidjson::Document doc;
		doc.SetObject();
		auto &allocator = doc.GetAllocator();

		// Add sourcePath
		doc.AddMember("sourcePath", rapidjson::Value(m_sourcePath.c_str(), allocator), allocator);

		// Add meshSettings
		rapidjson::Value msObj(rapidjson::kObjectType);
		msObj.AddMember("outputFormat", rapidjson::Value(settings->outputFormat.c_str(), allocator), allocator);
		msObj.AddMember("includePos", settings->includePos, allocator);
		msObj.AddMember("includeNormals", settings->includeNormals, allocator);
		msObj.AddMember("includeColors", settings->includeColors, allocator);
		msObj.AddMember("includeTexCoords", settings->includeTexCoords, allocator);
		msObj.AddMember("indexType", rapidjson::Value(settings->indexType.c_str(), allocator), allocator);
		msObj.AddMember("scale", settings->scale, allocator);
		msObj.AddMember("optimizeVertices", settings->optimizeVertices, allocator);
		msObj.AddMember("generateNormals", settings->generateNormals, allocator);

		doc.AddMember("meshSettings", msObj, allocator);

		// Convert to string
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		return buffer.GetString();
	}

	std::string DescriptorEditor::BuildAudioJson() const
	{
		const AudioSettings *settings = std::get_if<AudioSettings>(&m_settings);
		if (!settings) return "";

		rapidjson::Document doc;
		doc.SetObject();
		auto &allocator = doc.GetAllocator();

		// Add sourcePath
		doc.AddMember("sourcePath", rapidjson::Value(m_sourcePath.c_str(), allocator), allocator);

		// Add audioSettings
		rapidjson::Value asObj(rapidjson::kObjectType);
		asObj.AddMember("outputFormat", rapidjson::Value(settings->outputFormat.c_str(), allocator), allocator);
		asObj.AddMember("compression", rapidjson::Value(settings->compression.c_str(), allocator), allocator);
		asObj.AddMember("quality", settings->quality, allocator);
		asObj.AddMember("sampleRate", settings->sampleRate, allocator);
		asObj.AddMember("channelMode", rapidjson::Value(settings->channelMode.c_str(), allocator), allocator);

		doc.AddMember("audioSettings", asObj, allocator);

		// Convert to string
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		return buffer.GetString();
	}

	std::string DescriptorEditor::BuildShaderJson() const
	{
		const ShaderSettings *settings = std::get_if<ShaderSettings>(&m_settings);
		if (!settings) return "";

		rapidjson::Document doc;
		doc.SetObject();
		auto &allocator = doc.GetAllocator();

		// Add sourcePath
		doc.AddMember("sourcePath", rapidjson::Value(m_sourcePath.c_str(), allocator), allocator);

		// Add shaderSettings
		rapidjson::Value ssObj(rapidjson::kObjectType);
		ssObj.AddMember("vertexShader", rapidjson::Value(settings->vertexShader.c_str(), allocator), allocator);
		ssObj.AddMember("fragmentShader", rapidjson::Value(settings->fragmentShader.c_str(), allocator), allocator);
		ssObj.AddMember("outputFormat", rapidjson::Value(settings->outputFormat.c_str(), allocator), allocator);
		ssObj.AddMember("targetAPI", rapidjson::Value(settings->targetAPI.c_str(), allocator), allocator);
		ssObj.AddMember("targetVersion", rapidjson::Value(settings->targetVersion.c_str(), allocator), allocator);
		ssObj.AddMember("optimizationLevel", rapidjson::Value(settings->optimizationLevel.c_str(), allocator), allocator);
		ssObj.AddMember("stripDebugInfo", settings->stripDebugInfo, allocator);

		doc.AddMember("shaderSettings", ssObj, allocator);

		// Convert to string
		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		return buffer.GetString();
	}


}//end of namespace Engine