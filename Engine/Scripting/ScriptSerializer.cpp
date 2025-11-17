// Engine/Scripting/ScriptSerializer.cpp
#include "ScriptSerializer.h"

#include <mono/metadata/reflection.h>
#include <mono/metadata/attrdefs.h>
#include <imgui.h>

// ========================================
// Get all fields marked with [SerializeField]
// ========================================

std::vector<SerializedFieldInfo> GetSerializedFields(MonoObject* instance)
{
    std::vector<SerializedFieldInfo> serializedFields;

    if (!instance)
        return serializedFields;

    MonoClass* klass = mono_object_get_class(instance);
    if (!klass)
        return serializedFields;

    // Iterate through all fields in the class
    void* iter = nullptr;
    MonoClassField* field;

    while ((field = mono_class_get_fields(klass, &iter)))
    {
        // Get field attributes
        MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(klass, field);

        if (attrInfo)
        {
            // Check if field has SerializeField attribute
            for (int i = 0; i < attrInfo->num_attrs; i++)
            {
                MonoMethod* ctor = attrInfo->attrs[i].ctor;
                MonoClass* attrClass = mono_method_get_class(ctor);
                const char* attrName = mono_class_get_name(attrClass);

                // Check if this is our SerializeField attribute
                if (strcmp(attrName, "SerializeFieldAttribute") == 0)
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
                    break;
                }
            }

            mono_custom_attrs_free(attrInfo);
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
