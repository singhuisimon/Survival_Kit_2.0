// Engine/Scripting/ScriptSerializer.h
#pragma once

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <vector>

#include "../Component/ScriptComponent.h"

// ========================================
// SerializedFieldInfo Structure
// ========================================
namespace Engine {
	struct SerializedFieldInfo {
		std::string name;
		std::string displayName;
		MonoClassField *field;
		MonoType *type;

		enum class FieldType {
			Int, Float, Bool, String, Unknown
		};
		FieldType fieldType;
	};

	// ========================================
	// FieldValue Structure for ImGui
	// ========================================

	struct FieldValue {
		SerializedFieldInfo::FieldType type;
		union {
			int32_t intValue;
			float floatValue;
			bool boolValue;
		};
		std::string stringValue;
	};

	// ========================================
	// Function Declarations
	// ========================================

	/**
	 * Get all fields marked with [SerializeField] from a MonoObject
	 */
	std::vector<SerializedFieldInfo> GetSerializedFields(MonoObject *instance);

	/**
	 * Get field value from Mono instance
	 */
	FieldValue GetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo);

	/**
	 * Set field value on Mono instance
	 */
	void SetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo, const FieldValue &value);

	/**
	 * Render serialized fields in ImGui editor
	 */
	void RenderSerializedFieldsInImGui(MonoObject *scriptInstance);

	void SerializeScriptFieldsFromComponentToRapidJSON(const ScriptComponent &scriptComp, rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator);
	void DeserializeScriptFieldsToComponentFromRapidJSON(ScriptComponent &scriptComp, const rapidjson::Value &obj);
	void SerializeScriptFieldsToRapidJSON(MonoObject *instance, rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator);
	void SerializeScriptToDiskRapidJSON(MonoObject *instance, const std::string &filePath);

	void DeserializeScriptFieldsFromRapidJSON(MonoObject *instance, const rapidjson::Value &obj);
	void DeserializeScriptFromDiskRapidJSON(MonoObject *instance, const std::string &filePath);
}