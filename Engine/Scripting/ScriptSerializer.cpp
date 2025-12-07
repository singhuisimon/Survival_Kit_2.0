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


namespace Engine
{
    std::vector<SerializedFieldInfo> GetSerializedFields(MonoObject* instance)
    {
        std::vector<SerializedFieldInfo> serializedFields;

        if (!instance)
            return serializedFields;
        MonoScriptEngine::GetInstance().EnsureCorrectDomain();

        MonoClass* klass = mono_object_get_class(instance);
        if (!klass)
            return serializedFields;

        // Iterate through all fields in the class
        void* iter = nullptr;
        MonoClassField* field;

        while ((field = mono_class_get_fields(klass, &iter)))
        {
            // Skip static and private fields (only get public and serialized)
            int flags = mono_field_get_flags(field);

            // Skip if it's static
            if (flags & MONO_FIELD_ATTR_STATIC)
                continue;

            // Get field attributes
            MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(klass, field);

            bool hasSerializeField = false;

            if (attrInfo)
            {
                // Check if field has SerializeField attribute
                for (int i = 0; i < attrInfo->num_attrs; i++)
                {
                    MonoMethod* ctor = attrInfo->attrs[i].ctor;
                    if (!ctor) continue;  // Safety check

                    MonoClass* attrClass = mono_method_get_class(ctor);
                    if (!attrClass) continue;  // Safety check

                    const char* attrName = mono_class_get_name(attrClass);
                    if (!attrName) continue;  // Safety check

                    // Check if this is our SerializeField attribute
                    if (strcmp(attrName, "SerializeFieldAttribute") == 0)
                    {
                        hasSerializeField = true;
                        break;
                    }
                }

                mono_custom_attrs_free(attrInfo);
            }

            // Only add if it has SerializeField attribute
            if (hasSerializeField)
            {
                SerializedFieldInfo info;
                info.name = mono_field_get_name(field);
                info.field = field;
                info.type = mono_field_get_type(field);

                // Determine field type for easier ImGui rendering
                int typeEnum = mono_type_get_type(info.type);
                switch (typeEnum)
                {
                case MONO_TYPE_I4:  // int32
                    info.fieldType = SerializedFieldInfo::FieldType::Int;
                    break;
                case MONO_TYPE_R4:  // float
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

                // Get DisplayName property from attribute if set
                info.displayName = info.name; // Default to field name

                serializedFields.push_back(info);
            }
        }

        return serializedFields;
    }


    // ========================================
    // Get field value from Mono instance
    // ========================================

    FieldValue GetFieldValue(MonoObject* instance, const SerializedFieldInfo& fieldInfo)
    {
        FieldValue result;
        result.type = fieldInfo.fieldType;
        result.intValue = 0;
        result.floatValue = 0.0f;
        result.boolValue = false;
        result.stringValue = "";

        if (!instance)
            return result;

        switch (fieldInfo.fieldType)
        {
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
            MonoString* monoStr = nullptr;
            mono_field_get_value(instance, fieldInfo.field, &monoStr);
            if (monoStr)
            {
                char* cstr = mono_string_to_utf8(monoStr);
                result.stringValue = cstr ? cstr : "";
                if (cstr)
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

    void SetFieldValue(MonoObject* instance, const SerializedFieldInfo& fieldInfo, const FieldValue& value)
    {
        if (!instance)
            return;

        switch (value.type)
        {
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
            // Get the domain from the instance object itself
            MonoDomain* domain = mono_object_get_domain(instance);
            MonoString* monoStr = mono_string_new(domain, value.stringValue.c_str());
            mono_field_set_value(instance, fieldInfo.field, monoStr);
            break;
        }
        }
    }

    // ========================================
    // Render serialized fields in ImGui
    // ========================================

    void RenderSerializedFieldsInImGui(MonoObject* scriptInstance)
    {
        if (!scriptInstance)
            return;

        auto fields = GetSerializedFields(scriptInstance);

        if (fields.empty())
        {
            ImGui::TextDisabled("No serialized fields");
            return;
        }

        ImGui::Text("Script Properties:");
        ImGui::Separator();

        for (const auto& fieldInfo : fields)
        {
            FieldValue value = GetFieldValue(scriptInstance, fieldInfo);
            bool changed = false;

            switch (value.type)
            {
            case SerializedFieldInfo::FieldType::Int:
            {
                int temp = value.intValue;
                if (ImGui::InputInt(fieldInfo.displayName.c_str(), &temp))
                {
                    value.intValue = temp;
                    changed = true;
                }
                break;
            }

            case SerializedFieldInfo::FieldType::Float:
            {
                float temp = value.floatValue;
                if (ImGui::InputFloat(fieldInfo.displayName.c_str(), &temp))
                {
                    value.floatValue = temp;
                    changed = true;
                }
                break;
            }

            case SerializedFieldInfo::FieldType::Bool:
            {
                bool temp = value.boolValue;
                if (ImGui::Checkbox(fieldInfo.displayName.c_str(), &temp))
                {
                    value.boolValue = temp;
                    changed = true;
                }
                break;
            }

            case SerializedFieldInfo::FieldType::String:
            {
                char buffer[256];
                strncpy_s(buffer, sizeof(buffer), value.stringValue.c_str(), _TRUNCATE);
                if (ImGui::InputText(fieldInfo.displayName.c_str(), buffer, sizeof(buffer)))
                {
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
            if (changed)
            {
                SetFieldValue(scriptInstance, fieldInfo, value);
            }
        }
    }


    void SerializeScriptFieldsToRapidJSON(MonoObject* instance, rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator)
    {
        auto fields = GetSerializedFields(instance);
        for (const auto& fieldInfo : fields)
        {
            FieldValue value = GetFieldValue(instance, fieldInfo);
            const std::string& name = fieldInfo.name;
            switch (value.type)
            {
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

    void SerializeScriptToDiskRapidJSON(MonoObject* instance, const std::string& filePath)
    {
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
    void DeserializeScriptFieldsFromRapidJSON(MonoObject* instance, const rapidjson::Value& obj)
    {
        auto fields = GetSerializedFields(instance);
        for (const auto& fieldInfo : fields)
        {
            if (!obj.HasMember(fieldInfo.name.c_str()))
                continue;

            FieldValue value = GetFieldValue(instance, fieldInfo);

            const rapidjson::Value& fieldValue = obj[fieldInfo.name.c_str()];

            switch (value.type)
            {
            case SerializedFieldInfo::FieldType::Int:
                if (fieldValue.IsInt())
                    value.intValue = fieldValue.GetInt();
                break;
            case SerializedFieldInfo::FieldType::Float:
                if (fieldValue.IsNumber())
                    value.floatValue = static_cast<float>(fieldValue.GetDouble());
                break;
            case SerializedFieldInfo::FieldType::Bool:
                if (fieldValue.IsBool())
                    value.boolValue = fieldValue.GetBool();
                break;
            case SerializedFieldInfo::FieldType::String:
                if (fieldValue.IsString())
                    value.stringValue = fieldValue.GetString();
                break;
            default:
                break;
            }
            SetFieldValue(instance, fieldInfo, value);
        }
    }

    void DeserializeScriptFromDiskRapidJSON(MonoObject* instance, const std::string& filePath)
    {
        std::ifstream inFile(filePath);
        if (!inFile.is_open())
            return;
        std::stringstream buffer;
        buffer << inFile.rdbuf();

        rapidjson::Document doc;
        doc.Parse(buffer.str().c_str());

        if (!doc.IsObject())
            return;

        DeserializeScriptFieldsFromRapidJSON(instance, doc);
    }

}