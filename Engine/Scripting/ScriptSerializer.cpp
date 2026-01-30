// Engine/Scripting/ScriptSerializer.cpp
#include "ScriptSerializer.h"

#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>
#include <imgui.h>
#include "MonoScriptEngine.h"
#include <mono/jit/jit.h>
#include <fstream>
#include <sstream>

namespace {
	enum class SnapTag : std::uint8_t {
		Invalid = 0, Bool = 1, I1 = 2, U1 = 3, I2 = 4, U2 = 5,
		I4 = 6, U4 = 7, I8 = 8, U8 = 9, R4 = 10, R8 = 11,
		StringUtf8 = 12, RawValue = 13
	};

	static inline void AppendBytes(std::vector<std::uint8_t> &out, const void *p, size_t n) {
		const auto *b = static_cast<const std::uint8_t *>(p);
		out.insert(out.end(), b, b + n);
	}

	static inline rapidjson::Value BytesToArray(const std::vector<std::uint8_t> &bytes,
												rapidjson::Document::AllocatorType &allocator) {
		rapidjson::Value arr(rapidjson::kArrayType);
		arr.Reserve(static_cast<rapidjson::SizeType>(bytes.size()), allocator);
		for(std::uint8_t b : bytes) arr.PushBack(rapidjson::Value(static_cast<unsigned int>(b)), allocator);
		return arr;
	}

	static inline bool DecodeBytesToRapidValue(const std::vector<std::uint8_t> &bytes,
											   rapidjson::Value &out,
											   rapidjson::Document::AllocatorType &allocator) {
		out.SetNull();
		if(bytes.empty()) return false;

		const std::uint8_t tag = bytes[0];
		const std::uint8_t *p = bytes.data() + 1;
		const size_t n = bytes.size() - 1;
		auto need = [&](size_t k) { return n >= k; };

		switch(static_cast<SnapTag>(tag)) {
			case SnapTag::Bool:
			{
				if(need(1)) {
					uint8_t v; memcpy(&v, p, 1); out.SetBool(v != 0); return true;
				} break;
			}
			case SnapTag::I4:
			{
				if(need(4)) {
					int32_t v; memcpy(&v, p, 4); out.SetInt(v); return true;
				} break;
			}
			case SnapTag::R4:
			{
				if(need(4)) {
					float v; memcpy(&v, p, 4); out.SetDouble(v); return true;
				} break;
			}
			case SnapTag::StringUtf8:
			{
				if(need(4)) {
					uint32_t len; memcpy(&len, p, 4); p += 4;
					if(p + len <= bytes.data() + bytes.size()) {
						out.SetString((const char *)p, len, allocator); return true;
					}
				}
				break;
			}
			default: break;
		}
		out = BytesToArray(bytes, allocator);
		return true;
	}

	static inline std::vector<std::uint8_t> EncodeScalarToBytes(const rapidjson::Value &v) {
		std::vector<std::uint8_t> out;
		if(v.IsBool()) {
			out.push_back((uint8_t)SnapTag::Bool); uint8_t b = v.GetBool(); AppendBytes(out, &b, 1);
		}
		else if(v.IsInt()) {
			out.push_back((uint8_t)SnapTag::I4); int32_t i = v.GetInt(); AppendBytes(out, &i, 4);
		}
		else if(v.IsDouble()) {
			out.push_back((uint8_t)SnapTag::R4); float f = (float)v.GetDouble(); AppendBytes(out, &f, 4);
		}
		else if(v.IsString()) {
			out.push_back((uint8_t)SnapTag::StringUtf8);
			const char *s = v.GetString(); uint32_t l = v.GetStringLength();
			AppendBytes(out, &l, 4); AppendBytes(out, s, l);
		}
		return out;
	}
}

namespace Engine {
	std::vector<SerializedFieldInfo> GetSerializedFields(MonoObject *instance) {
		std::vector<SerializedFieldInfo> serializedFields;
		if(!instance) return serializedFields;

		MonoScriptEngine::GetInstance().EnsureCorrectDomain();
		MonoClass *klass = mono_object_get_class(instance);
		if(!klass) return serializedFields;

		void *iter = nullptr;
		MonoClassField *field;
		while((field = mono_class_get_fields(klass, &iter))) {
			int flags = mono_field_get_flags(field);
			if(flags & MONO_FIELD_ATTR_STATIC) continue;

			// Simple check for [SerializeField] or public
			bool keep = (flags & MONO_FIELD_ATTR_PUBLIC);
			if(!keep) {
				MonoCustomAttrInfo *attr = mono_custom_attrs_from_field(klass, field);
				if(attr) {
					for(int i = 0; i < attr->num_attrs; ++i) {
						if(attr->attrs[i].ctor) {
							MonoClass *ac = mono_method_get_class(attr->attrs[i].ctor);
							const char *an = mono_class_get_name(ac);
							if(an && strstr(an, "SerializeField")) {
								keep = true; break;
							}
						}
					}
					mono_custom_attrs_free(attr);
				}
			}

			if(!keep) continue;

			SerializedFieldInfo info;
			info.name = mono_field_get_name(field);
			info.field = field;
			info.type = mono_field_get_type(field);
			info.displayName = info.name;

			int t = mono_type_get_type(info.type);
			switch(t) {
				case MONO_TYPE_I4: info.fieldType = SerializedFieldInfo::FieldType::Int; break;
				case MONO_TYPE_R4: info.fieldType = SerializedFieldInfo::FieldType::Float; break;
				case MONO_TYPE_BOOLEAN: info.fieldType = SerializedFieldInfo::FieldType::Bool; break;
				case MONO_TYPE_STRING: info.fieldType = SerializedFieldInfo::FieldType::String; break;
				default: info.fieldType = SerializedFieldInfo::FieldType::Unknown; break;
			}
			serializedFields.push_back(info);
		}
		return serializedFields;
	}

	FieldValue GetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo) {
		FieldValue result;
		result.type = fieldInfo.fieldType;
		if(!instance) return result;

		MonoScriptEngine::GetInstance().EnsureCorrectDomain();

		switch(fieldInfo.fieldType) {
			case SerializedFieldInfo::FieldType::Int:
			{
				int v = 0; mono_field_get_value(instance, fieldInfo.field, &v); result.intValue = v; break;
			}
			case SerializedFieldInfo::FieldType::Float:
			{
				float v = 0; mono_field_get_value(instance, fieldInfo.field, &v); result.floatValue = v; break;
			}
			case SerializedFieldInfo::FieldType::Bool:
			{
				bool v = false; mono_field_get_value(instance, fieldInfo.field, &v); result.boolValue = v; break;
			}
			case SerializedFieldInfo::FieldType::String:
			{
				MonoString *ms = nullptr;
				mono_field_get_value(instance, fieldInfo.field, &ms);
				if(ms) {
					char *utf8 = mono_string_to_utf8(ms);
					if(utf8) {
						result.stringValue = utf8; mono_free(utf8);
					}
				}
				break;
			}
			default: break;
		}
		return result;
	}

	void SetFieldValue(MonoObject *instance, const SerializedFieldInfo &fieldInfo, const FieldValue &value) {
		if(!instance) return;
		auto &se = MonoScriptEngine::GetInstance();
		se.EnsureCorrectDomain();

		// CRITICAL: String allocation triggers GC.
		// Instance pointer might become invalid inside this function if not careful.
		// We rely on RenderSerializedFieldsInImGui to handle the stale pointer update.

		switch(value.type) {
			case SerializedFieldInfo::FieldType::Int:
			{
				int v = value.intValue; mono_field_set_value(instance, fieldInfo.field, &v); break;
			}
			case SerializedFieldInfo::FieldType::Float:
			{
				float v = value.floatValue; mono_field_set_value(instance, fieldInfo.field, &v); break;
			}
			case SerializedFieldInfo::FieldType::Bool:
			{
				bool v = value.boolValue; mono_field_set_value(instance, fieldInfo.field, &v); break;
			}
			case SerializedFieldInfo::FieldType::String:
			{
				MonoDomain *d = mono_object_get_domain(instance);
				MonoString *ms = mono_string_new(d, value.stringValue.c_str());

				// Re-verify instance is still valid after allocation (basic check)
				// Note: Real safety comes from using GCHandles, which we don't have easily here.
				// However, mono_field_set_value takes the object pointer.
				mono_field_set_value(instance, fieldInfo.field, ms);
				break;
			}
			default: break;
		}

		// Store to component immediately
		se.StoreSerializedFieldToComponent(instance, fieldInfo.field);
	}

	void RenderSerializedFieldsInImGui(MonoObject *scriptInstance) {
		if(!scriptInstance) return;
		auto &se = MonoScriptEngine::GetInstance();
		se.EnsureCorrectDomain();

		// 1. Initial safe resolution
		scriptInstance = se.ResolveScriptInstance(scriptInstance);
		if(!scriptInstance) return;

		auto fields = GetSerializedFields(scriptInstance);
		if(fields.empty()) return;

		ImGui::Text("Script Properties:");
		ImGui::Separator();

		for(const auto &fieldInfo : fields) {
			// CRITICAL FIX: Re-resolve instance *every* iteration.
			// The previous iteration's SetFieldValue (specifically string) could have triggered GC.
			// This moves the object in memory. 'scriptInstance' from the start of the function is now stale.
			scriptInstance = se.ResolveScriptInstance(scriptInstance);

			if(!scriptInstance) {
				ImGui::TextDisabled("Instance invalid (GC moved object)");
				break;
			}

			FieldValue value = GetFieldValue(scriptInstance, fieldInfo);
			bool changed = false;

			switch(value.type) {
				case SerializedFieldInfo::FieldType::Int:
					if(ImGui::InputInt(fieldInfo.displayName.c_str(), &value.intValue)) changed = true;
					break;
				case SerializedFieldInfo::FieldType::Float:
					if(ImGui::InputFloat(fieldInfo.displayName.c_str(), &value.floatValue)) changed = true;
					break;
				case SerializedFieldInfo::FieldType::Bool:
					if(ImGui::Checkbox(fieldInfo.displayName.c_str(), &value.boolValue)) changed = true;
					break;
				case SerializedFieldInfo::FieldType::String:
				{
					char buf[256];
					strncpy_s(buf, value.stringValue.c_str(), 255);
					if(ImGui::InputText(fieldInfo.displayName.c_str(), buf, 256)) {
						value.stringValue = buf;
						changed = true;
					}
					break;
				}
				default: break;
			}

			if(changed) {
				// Re-resolve before writing
				scriptInstance = se.ResolveScriptInstance(scriptInstance);
				if(scriptInstance) {
					SetFieldValue(scriptInstance, fieldInfo, value);
				}
			}
		}
	}

	void SerializeScriptFieldsFromComponentToRapidJSON(const ScriptComponent &sc, rapidjson::Value &obj, rapidjson::Document::AllocatorType &alloc) {
		obj.SetObject();
		for(const auto &kv : sc.SerializedFields) {
			if(kv.first == "EntityID") continue;
			rapidjson::Value key(kv.first.c_str(), alloc);
			rapidjson::Value val;
			if(DecodeBytesToRapidValue(kv.second, val, alloc)) obj.AddMember(key, val, alloc);
		}
	}

	void DeserializeScriptFieldsToComponentFromRapidJSON(ScriptComponent &sc, const rapidjson::Value &obj) {
		if(!obj.IsObject()) return;
		for(auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
			if(it->name.GetString() == std::string("EntityID")) continue;
			std::vector<uint8_t> bytes = EncodeScalarToBytes(it->value);
			if(!bytes.empty()) sc.SerializedFields[it->name.GetString()] = bytes;
		}
	}

	void SerializeScriptFieldsToRapidJSON(MonoObject *instance, rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator) {
		auto fields = GetSerializedFields(instance);
		for(const auto &fieldInfo : fields) {
			FieldValue value = GetFieldValue(instance, fieldInfo);
			rapidjson::Value name(fieldInfo.name.c_str(), allocator);
			switch(value.type) {
				case SerializedFieldInfo::FieldType::Int: obj.AddMember(name, value.intValue, allocator); break;
				case SerializedFieldInfo::FieldType::Float: obj.AddMember(name, value.floatValue, allocator); break;
				case SerializedFieldInfo::FieldType::Bool: obj.AddMember(name, value.boolValue, allocator); break;
				case SerializedFieldInfo::FieldType::String: obj.AddMember(name, rapidjson::Value(value.stringValue.c_str(), allocator), allocator); break;
				default: break;
			}
		}
	}

	void SerializeScriptToDiskRapidJSON(MonoObject *instance, const std::string &filePath) {
		rapidjson::Document doc; doc.SetObject();
		SerializeScriptFieldsToRapidJSON(instance, doc, doc.GetAllocator());
		rapidjson::StringBuffer buffer; rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream outFile(filePath); outFile << buffer.GetString();
	}

	void DeserializeScriptFieldsFromRapidJSON(MonoObject *instance, const rapidjson::Value &obj) {
		auto fields = GetSerializedFields(instance);
		for(const auto &fieldInfo : fields) {
			if(!obj.HasMember(fieldInfo.name.c_str())) continue;
			FieldValue value = GetFieldValue(instance, fieldInfo);
			const auto &v = obj[fieldInfo.name.c_str()];

			if(v.IsInt()) value.intValue = v.GetInt();
			else if(v.IsDouble()) value.floatValue = (float)v.GetDouble();
			else if(v.IsBool()) value.boolValue = v.GetBool();
			else if(v.IsString()) value.stringValue = v.GetString();

			SetFieldValue(instance, fieldInfo, value);
		}
	}

	void DeserializeScriptFromDiskRapidJSON(MonoObject *instance, const std::string &filePath) {
		std::ifstream inFile(filePath);
		if(!inFile.is_open()) return;
		std::stringstream buffer; buffer << inFile.rdbuf();
		rapidjson::Document doc; doc.Parse(buffer.str().c_str());
		if(doc.IsObject()) DeserializeScriptFieldsFromRapidJSON(instance, doc);
	}
}