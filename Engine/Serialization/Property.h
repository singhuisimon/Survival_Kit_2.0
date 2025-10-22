#pragma once

// Standard Library
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>

// GLM Math Library
#include <glm/glm.hpp>

namespace Engine {

    // Forward declarations
    class Entity;

    /**
     * @brief Property types supported by the reflection system
     */
    enum class PropertyType {
        Bool,
        U32,
        Int,
        Float,
        String,
        Vec2,
        Vec3,
        Vec4,
        Quat,
        Entity
    };

    /**
     * @brief Base property class - type-erased property access
     */
    class PropertyBase {
    public:
        PropertyBase(const std::string& name, PropertyType type)
            : m_Name(name), m_Type(type) {
        }

        virtual ~PropertyBase() = default;

        const std::string& GetName() const { return m_Name; }
        PropertyType GetType() const { return m_Type; }

        // Serialization helpers
        virtual std::string ToString(void* instance) const = 0;
        virtual void FromString(void* instance, const std::string& value) const = 0;

    protected:
        std::string m_Name;
        PropertyType m_Type;
    };

    /**
     * @brief Typed property - provides type-safe access to component members
     */
    template<typename ClassType, typename ValueType>
    class Property : public PropertyBase {
    public:
        using Getter = std::function<ValueType(const ClassType&)>;
        using Setter = std::function<void(ClassType&, const ValueType&)>;

        Property(const std::string& name, PropertyType type, Getter getter, Setter setter)
            : PropertyBase(name, type)
            , m_Getter(getter)
            , m_Setter(setter) {
        }

        ValueType Get(const ClassType& instance) const {
            return m_Getter(instance);
        }

        void Set(ClassType& instance, const ValueType& value) const {
            m_Setter(instance, value);
        }

        // Serialization implementation
        std::string ToString(void* instance) const override;
        void FromString(void* instance, const std::string& value) const override;

    private:
        Getter m_Getter;
        Setter m_Setter;
    };

    /**
     * @brief Component metadata - stores all properties of a component type
     */
    class ComponentMetadata {
    public:
        ComponentMetadata(const std::string& name)
            : m_Name(name) {
        }

        // Delete copy constructor and assignment to prevent issues
        ComponentMetadata(const ComponentMetadata&) = delete;
        ComponentMetadata& operator=(const ComponentMetadata&) = delete;

        // Allow move operations
        ComponentMetadata(ComponentMetadata&&) = default;
        ComponentMetadata& operator=(ComponentMetadata&&) = default;

        template<typename ClassType, typename ValueType>
        void AddProperty(const std::string& name, PropertyType type,
            typename Property<ClassType, ValueType>::Getter getter,
            typename Property<ClassType, ValueType>::Setter setter) {
            m_Properties.push_back(
                std::make_unique<Property<ClassType, ValueType>>(name, type, getter, setter)
            );
        }

        const std::string& GetName() const { return m_Name; }
        const std::vector<std::unique_ptr<PropertyBase>>& GetProperties() const {
            return m_Properties;
        }

    private:
        std::string m_Name;
        std::vector<std::unique_ptr<PropertyBase>> m_Properties;
    };

    // Template implementations for ToString/FromString
    template<typename ClassType, typename ValueType>
    std::string Property<ClassType, ValueType>::ToString(void* instance) const {
        ClassType* obj = static_cast<ClassType*>(instance);
        ValueType value = Get(*obj);

        // Handle different types
        if constexpr (std::is_same_v<ValueType, bool>) {
            return value ? "true" : "false";
        }
        else if constexpr (std::is_same_v<ValueType, int>) {
            return std::to_string(value);
        }
        else if constexpr (std::is_same_v<ValueType, float>) {
            return std::to_string(value);
        }
        else if constexpr (std::is_same_v<ValueType, std::string>) {
            return value;
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec2>) {
            return std::to_string(value.x) + "," + std::to_string(value.y);
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec3>) {
            return std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z);
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec4>) {
            return std::to_string(value.x) + "," + std::to_string(value.y) + "," +
                std::to_string(value.z) + "," + std::to_string(value.w);
        }
        return "";
    }

    template<typename ClassType, typename ValueType>
    void Property<ClassType, ValueType>::FromString(void* instance, const std::string& value) const {
        ClassType* obj = static_cast<ClassType*>(instance);

        // Handle different types
        if constexpr (std::is_same_v<ValueType, bool>) {
            Set(*obj, value == "true" || value == "1");
        }
        else if constexpr (std::is_same_v<ValueType, int>) {
            Set(*obj, std::stoi(value));
        }
        else if constexpr (std::is_same_v<ValueType, float>) {
            Set(*obj, std::stof(value));
        }
        else if constexpr (std::is_same_v<ValueType, std::string>) {
            Set(*obj, value);
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec2>) {
            glm::vec2 vec;
            size_t pos = value.find(',');
            vec.x = std::stof(value.substr(0, pos));
            vec.y = std::stof(value.substr(pos + 1));
            Set(*obj, vec);
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec3>) {
            glm::vec3 vec;
            size_t pos1 = value.find(',');
            size_t pos2 = value.find(',', pos1 + 1);
            vec.x = std::stof(value.substr(0, pos1));
            vec.y = std::stof(value.substr(pos1 + 1, pos2 - pos1 - 1));
            vec.z = std::stof(value.substr(pos2 + 1));
            Set(*obj, vec);
        }
        else if constexpr (std::is_same_v<ValueType, glm::vec4>) {
            glm::vec4 vec;
            size_t pos1 = value.find(',');
            size_t pos2 = value.find(',', pos1 + 1);
            size_t pos3 = value.find(',', pos2 + 1);
            vec.x = std::stof(value.substr(0, pos1));
            vec.y = std::stof(value.substr(pos1 + 1, pos2 - pos1 - 1));
            vec.z = std::stof(value.substr(pos2 + 1, pos3 - pos2 - 1));
            vec.w = std::stof(value.substr(pos3 + 1));
            Set(*obj, vec);
        }
    }

} // namespace Engine