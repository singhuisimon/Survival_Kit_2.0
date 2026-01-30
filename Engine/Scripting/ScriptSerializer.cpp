// Engine/Scripting/ScriptSerializer.cpp
#include "ScriptSerializer.h"

#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>
#include <imgui.h>
#include "MonoScriptEngine.h"

#include <fstream>
#include <sstream>
// ========================================
// Get all fields marked with [SerializeField]
// ========================================

namespace {
	enum class SnapTag : std::uint8_t {
		Invalid = 0,
		Bool = 1,
		I1 = 2,
		U1 = 3,
		I2 = 4,
		U2 = 5,
		I4 = 6,
		U4 = 7,
		I8 = 8,
		U8 = 9,
		R4 = 10,
		R8 = 11,
		StringUtf8 = 12,
		RawValue = 13
	};

	static inline void AppendBytes(std::vector<std::uint8_t> &out, const void *p, size_t n) {
		const auto *b = static_cast<const std::uint8_t *>(p);
		out.insert(out.end(), b, b + n);
	}

	static inline rapidjson::Value BytesToArray(const std::vector<std::uint8_t> &bytes,
												rapidjson::Document::AllocatorType &allocator) {
		rapidjson::Value arr(rapidjson::kArrayType);
		arr.Reserve(static_cast<rapidjson::SizeType>(bytes.size()), allocator);
		for(std::uint8_t b : bytes)
			arr.PushBack(rapidjson::Value(static_cast<unsigned int>(b)), allocator);
		return arr;
	}

	static inline bool DecodeBytesToRapidValue(const std::vector<std::uint8_t> &bytes,
											   rapidjson::Value &out,
											   rapidjson::Document::AllocatorType &allocator) {
		out.SetNull();
		if(bytes.empty())
			return false;

		const std::uint8_t tag = bytes[0];
		const std::uint8_t *p = bytes.data() + 1;
		const size_t n = bytes.size() - 1;

		auto need = [&](size_t k) { return n >= k; };

		switch(static_cast<SnapTag>(tag)) {
			case SnapTag::Bool:
			{
				if(!need(sizeof(std::uint8_t))) return false;
				std::uint8_t v = 0; std::memcpy(&v, p, sizeof(v));
				out.SetBool(v != 0);
				return true;
			}
			case SnapTag::I4:
			{
				if(!need(sizeof(std::int32_t))) return false;
				std::int32_t v = 0; std::memcpy(&v, p, sizeof(v));
				out.SetInt(v);
				return true;
			}
			case SnapTag::U4:
			{
				if(!need(sizeof(std::uint32_t))) return false;
				std::uint32_t v = 0; std::memcpy(&v, p, sizeof(v));
				out.SetUint(v);
				return true;
			}
			case SnapTag::I8:
			{
				if(!need(sizeof(std::int64_t))) return false;
				std::int64_t v = 0; std::memcpy(&v, p, sizeof(v));
				out.SetInt64(v);
				return true;
			}
			case SnapTag::U8:
			{
				if(!need(sizeof(std::uint64_t))) return false;
				std::uint64_t v = 0; std::memcpy(&v, p, sizeof(v));
				out.SetUint64(v);
				return true;
			}
			case SnapTag::R4:
			{
				if(!need(sizeof(float))) return false;
				float v = 0.0f; std::memcpy(&v, p, sizeof(v));
				out.SetDouble(static_cast<double>(v));
				return true;
			}
			case SnapTag::R8:
			{
				if(!need(sizeof(double))) return false;
				double v = 0.0; std::memcpy(&v, p, sizeof(v));
				out.SetDouble(v);
				return true;
			}
			case SnapTag::StringUtf8:
			{
				if(!need(sizeof(std::uint32_t))) return false;
				std::uint32_t len = 0;
				std::memcpy(&len, p, sizeof(len));
				p += sizeof(std::uint32_t);

				const std::uint8_t *end = bytes.data() + bytes.size();
				if(p + len > end) return false;

				out.SetString(reinterpret_cast<const char *>(p), len, allocator);
				return true;
			}
			case SnapTag::RawValue:
			default:
				// Preserve unknown payloads losslessly.
				out = BytesToArray(bytes, allocator);
				return true;
		}
	}

	static inline std::vector<std::uint8_t> EncodeScalarToBytes(const rapidjson::Value &v) {
		std::vector<std::uint8_t> out;

		if(v.IsBool()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::Bool));
			std::uint8_t b = v.GetBool() ? 1u : 0u;
			AppendBytes(out, &b, sizeof(b));
			return out;
		}

		if(v.IsInt()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::I4));
			std::int32_t i = v.GetInt();
			AppendBytes(out, &i, sizeof(i));
			return out;
		}

		if(v.IsUint()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::U4));
			std::uint32_t u = v.GetUint();
			AppendBytes(out, &u, sizeof(u));
			return out;
		}

		if(v.IsInt64()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::I8));
			std::int64_t i = v.GetInt64();
			AppendBytes(out, &i, sizeof(i));
			return out;
		}

		if(v.IsUint64()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::U8));
			std::uint64_t u = v.GetUint64();
			AppendBytes(out, &u, sizeof(u));
			return out;
		}

		if(v.IsNumber()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::R4));
			float f = static_cast<float>(v.GetDouble());
			AppendBytes(out, &f, sizeof(f));
			return out;
		}

		if(v.IsString()) {
			out.push_back(static_cast<std::uint8_t>(SnapTag::StringUtf8));
			const char *s = v.GetString();
			const std::uint32_t len = static_cast<std::uint32_t>(v.GetStringLength());
			AppendBytes(out, &len, sizeof(len));
			if(len && s) AppendBytes(out, s, len);
			return out;
		}

		return {};
	}
}

namespace Engine {
	std::vector<SerializedFieldInfo> GetSerializedFields(MonoObject *instance) {
		std::vector<SerializedFieldInfo> serializedFields;

		if(!instance)
			return serializedFields;

		MonoScriptEngine::GetInstance().EnsureCorrectDomain();

		MonoClass *klass = mono_object_get_class(instance);
		if(!klass)
			return serializedFields;

		void *iter = nullptr;
		MonoClassField *field;

		while((field = mono_class_get_fields(klass, &iter))) {
			int flags = mono_field_get_flags(field);

			if(flags & MONO_FIELD_ATTR_STATIC)
				continue;

			const bool isPublic = (flags & MONO_FIELD_ATTR_PUBLIC) != 0;

			bool hasSerializeField = false;
			MonoCustomAttrInfo *attrInfo = mono_custom_attrs_from_field(klass, field);
			if(attrInfo) {
				for(int i = 0; i < attrInfo->num_attrs; i++) {
					MonoMethod *ctor = attrInfo->attrs[i].ctor;
					if(!ctor) continue;

					MonoClass *attrClass = mono_method_get_class(ctor);
					if(!attrClass) continue;

					const char *attrName = mono_class_get_name(attrClass);
					if(!attrName) continue;

					if(strcmp(attrName, "SerializeFieldAttribute") == 0 ||
					   strcmp(attrName, "SerializedFieldAttribute") == 0 ||
					   strcmp(attrName, "SerializeField") == 0 ||
					   strcmp(attrName, "SerializedField") == 0) {
						hasSerializeField = true;
						break;
					}
				}
				mono_custom_attrs_free(attrInfo);
			}

			// Public fields are serialized by default; non-public only if it has the attribute.
			if(!(isPublic || hasSerializeField))
				continue;

			SerializedFieldInfo info;
			info.name = mono_field_get_name(field);
			info.field = field;
			info.type = mono_field_get_type(field);

			int typeEnum = mono_type_get_type(info.type);
			switch(typeEnum) {
				case MONO_TYPE_I4:
					info.fieldType = SerializedFieldInfo::FieldType::Int;
					break;
				case MONO_TYPE_R4:
					info.fieldType = SerializedFieldInfo::FieldType::Float;
					break;
				case MONO_TYPE_BOOLEAN:
					info.fieldType = SerializedFieldInfo::FieldType::Bool;
					break;
				case MONO_TYPE_STRING:
					info.fieldType = SerializedFieldInfo::FieldType::String;
					break;
				default:
					info.fieldType = SerializedFieldInfo::FieldType::Unknown;
					break;
			}

			info.displayName = info.name;
			serializedFields.push_back(info);
		}

		return serializedFields;
	}


	// ========================================
	// Get field value from Mono instance
	// ========================================

	FieldValue GetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo) {
		FieldValue result;
		result.type = fieldInfo.fieldType;
		result.intValue = 0;
		result.floatValue = 0.0f;
		result.boolValue = false;
		result.stringValue = "";

		if(!instance)
			return result;

		switch(fieldInfo.fieldType) {
			case SerializedFieldInfo::FieldType::Int:
				mono_field_get_value(instance, fieldInfo.field, &result.intValue);
				break;

			case SerializedFieldInfo::FieldType::Float:
				mono_field_get_value(instance, fieldInfo.field, &result.floatValue);
				break;

			case SerializedFieldInfo::FieldType::Bool:
				mono_field_get_value(instance, fieldInfo.field, &result.boolValue);
				break;

			case SerializedFieldInfo::FieldType::String:
			{
				MonoString *monoStr = nullptr;
				mono_field_get_value(instance, fieldInfo.field, &monoStr);
				if(monoStr) {
					char *cstr = mono_string_to_utf8(monoStr);
					result.stringValue = cstr ? cstr : "";
					if(cstr)
						mono_free(cstr);
				}
				break;
			}

			default:
				break;
		}

		return result;
	}

	// ========================================
	// Set field value on Mono instance
	// ========================================

	void SetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo, const FieldValue &value) {
		if(!instance)
			return;

		switch(value.type) {
			case SerializedFieldInfo::FieldType::Int:
			{
				int32_t temp = value.intValue;
				mono_field_set_value(instance, fieldInfo.field, &temp);
				break;
			}

			case SerializedFieldInfo::FieldType::Float:
			{
				float temp = value.floatValue;
				mono_field_set_value(instance, fieldInfo.field, &temp);
				break;
			}

			case SerializedFieldInfo::FieldType::Bool:
			{
				bool temp = value.boolValue;
				mono_field_set_value(instance, fieldInfo.field, &temp);
				break;
			}

			case SerializedFieldInfo::FieldType::String:
			{
				MonoDomain *domain = mono_object_get_domain(instance);
				MonoString *monoStr = mono_string_new(domain, value.stringValue.c_str());

				// NOTE: mono_field_set_value expects a pointer to the value to assign.
				mono_field_set_value(instance, fieldInfo.field, &monoStr);
				break;
			}
			default:
				break;
		}

		// Persist editor-authored value into ScriptComponent storage so it survives hot reload/build.
		MonoScriptEngine::GetInstance().StoreSerializedFieldToComponent(instance, fieldInfo.field);
	}

	// ========================================
	// Render serialized fields in ImGui
	// ========================================

	void RenderSerializedFieldsInImGui(MonoObject *scriptInstance) {
		if(!scriptInstance)
			return;

		auto fields = GetSerializedFields(scriptInstance);

		if(fields.empty()) {
			ImGui::TextDisabled("No serialized fields");
			return;
		}

		ImGui::Text("Script Properties:");
		ImGui::Separator();

		for(const auto &fieldInfo : fields) {
			FieldValue value = GetFieldValue(scriptInstance, fieldInfo);
			bool changed = false;

			switch(value.type) {
				case SerializedFieldInfo::FieldType::Int:
				{
					int temp = value.intValue;
					if(ImGui::InputInt(fieldInfo.displayName.c_str(), &temp)) {
						value.intValue = temp;
						changed = true;
					}
					break;
				}

				case SerializedFieldInfo::FieldType::Float:
				{
					float temp = value.floatValue;
					if(ImGui::InputFloat(fieldInfo.displayName.c_str(), &temp)) {
						value.floatValue = temp;
						changed = true;
					}
					break;
				}

				case SerializedFieldInfo::FieldType::Bool:
				{
					bool temp = value.boolValue;
					if(ImGui::Checkbox(fieldInfo.displayName.c_str(), &temp)) {
						value.boolValue = temp;
						changed = true;
					}
					break;
				}

				case SerializedFieldInfo::FieldType::String:
				{
					char buffer[256];
					strncpy_s(buffer, sizeof(buffer), value.stringValue.c_str(), _TRUNCATE);
					if(ImGui::InputText(fieldInfo.displayName.c_str(), buffer, sizeof(buffer))) {
						value.stringValue = buffer;
						changed = true;
					}
					break;
				}

				default:
					ImGui::TextDisabled("%s: <unsupported type>", fieldInfo.displayName.c_str());
					break;
			}

			// Write back to Mono if changed
			if(changed) {
				SetFieldValue(scriptInstance, fieldInfo, value);
			}
		}
	}

	void SerializeScriptFieldsFromComponentToRapidJSON(
		const ScriptComponent &scriptComp,
		rapidjson::Value &obj,
		rapidjson::Document::AllocatorType &allocator) {
		obj.SetObject();

		for(const auto &kv : scriptComp.SerializedFields) {
			const std::string &fieldName = kv.first;
			if(fieldName == "EntityID")
				continue;

			const std::vector<std::uint8_t> &bytes = kv.second;
			if(bytes.empty())
				continue;

			rapidjson::Value key(fieldName.c_str(), allocator);
			rapidjson::Value val;
			if(!DecodeBytesToRapidValue(bytes, val, allocator))
				continue;

			obj.AddMember(key, val, allocator);
		}
	}

	void DeserializeScriptFieldsToComponentFromRapidJSON(
		ScriptComponent &scriptComp,
		const rapidjson::Value &obj) {
		if(!obj.IsObject())
			return;

		for(auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
			if(!it->name.IsString())
				continue;

			const std::string fieldName = it->name.GetString();
			if(fieldName == "EntityID")
				continue;

			const rapidjson::Value &v = it->value;

			// Lossless byte-array form for unknown/struct payloads
			if(v.IsArray()) {
				std::vector<std::uint8_t> bytes;
				bytes.reserve(v.Size());

				for(rapidjson::SizeType i = 0; i < v.Size(); ++i) {
					const rapidjson::Value &b = v[i];
					if(!b.IsUint()) {
						bytes.clear(); break;
					}
					unsigned int ui = b.GetUint();
					if(ui > 255u) {
						bytes.clear(); break;
					}
					bytes.push_back(static_cast<std::uint8_t>(ui));
				}

				if(!bytes.empty())
					scriptComp.SerializedFields[fieldName] = std::move(bytes);

				continue;
			}

			std::vector<std::uint8_t> bytes = EncodeScalarToBytes(v);
			if(!bytes.empty())
				scriptComp.SerializedFields[fieldName] = std::move(bytes);
		}
	}

	void SerializeScriptFieldsToRapidJSON(MonoObject *instance, rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator) {
		auto fields = GetSerializedFields(instance);
		for(const auto &fieldInfo : fields) {
			FieldValue value = GetFieldValue(instance, fieldInfo);
			const std::string &name = fieldInfo.name;
			switch(value.type) {
				case SerializedFieldInfo::FieldType::Int:
					obj.AddMember(
						rapidjson::Value(name.c_str(), allocator),              // Key
						rapidjson::Value(value.intValue),                       // **Value converted to RapidJSON Value**
						allocator);
					break;
				case SerializedFieldInfo::FieldType::Float:
					obj.AddMember(rapidjson::Value(name.c_str(), allocator), rapidjson::Value(value.floatValue), allocator);
					break;
				case SerializedFieldInfo::FieldType::Bool:
					obj.AddMember(rapidjson::Value(name.c_str(), allocator), rapidjson::Value(value.boolValue), allocator);
					break;
				case SerializedFieldInfo::FieldType::String:
					obj.AddMember(
						rapidjson::Value(name.c_str(), allocator),                            // key
						rapidjson::Value(value.stringValue.c_str(), allocator),               // value as rapidjson::Value
						allocator);
					break;
				default:
					break;
			}
		}
	}

	void SerializeScriptToDiskRapidJSON(MonoObject *instance, const std::string &filePath) {
		rapidjson::Document doc;
		doc.SetObject();
		SerializeScriptFieldsToRapidJSON(instance, doc, doc.GetAllocator());

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		std::ofstream outFile(filePath);
		outFile << buffer.GetString();
		outFile.close();
	}

	// ----------- DESERIALIZE FROM RAPIDJSON -----------
	void DeserializeScriptFieldsFromRapidJSON(MonoObject *instance, const rapidjson::Value &obj) {
		auto fields = GetSerializedFields(instance);
		for(const auto &fieldInfo : fields) {
			if(!obj.HasMember(fieldInfo.name.c_str()))
				continue;

			FieldValue value = GetFieldValue(instance, fieldInfo);

			const rapidjson::Value &fieldValue = obj[fieldInfo.name.c_str()];

			switch(value.type) {
				case SerializedFieldInfo::FieldType::Int:
					if(fieldValue.IsInt())
						value.intValue = fieldValue.GetInt();
					break;
				case SerializedFieldInfo::FieldType::Float:
					if(fieldValue.IsNumber())
						value.floatValue = static_cast<float>(fieldValue.GetDouble());
					break;
				case SerializedFieldInfo::FieldType::Bool:
					if(fieldValue.IsBool())
						value.boolValue = fieldValue.GetBool();
					break;
				case SerializedFieldInfo::FieldType::String:
					if(fieldValue.IsString())
						value.stringValue = fieldValue.GetString();
					break;
				default:
					break;
			}
			SetFieldValue(instance, fieldInfo, value);
		}
	}

	void DeserializeScriptFromDiskRapidJSON(MonoObject *instance, const std::string &filePath) {
		std::ifstream inFile(filePath);
		if(!inFile.is_open())
			return;
		std::stringstream buffer;
		buffer << inFile.rdbuf();

		rapidjson::Document doc;
		doc.Parse(buffer.str().c_str());

		if(!doc.IsObject())
			return;

		DeserializeScriptFieldsFromRapidJSON(instance, doc);
	}

}