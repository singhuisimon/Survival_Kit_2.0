/**
 * @file RegisterBehaviourTreeComponent.cpp
 * @brief Register BehaviourTreeComponent with the reflection system
 * @author AI System Team
 * @date 2025
 * 
 * Add this code to your existing ComponentRegistry.cpp file in the
 * RegisterAllComponents() function
 */

#include "../Serialization/ComponentRegistry.h"
#include "../Component/BehaviourTreeComponent.h"

// Add this to ComponentRegistry::RegisterAllComponents()
namespace Engine {

void RegisterBehaviourTreeComponent() {
    // Register BehaviourTreeComponent
    auto& meta = REGISTER_COMPONENT(BehaviourTreeComponent);

    // Active property
    meta.AddProperty<BehaviourTreeComponent, bool>(
        "Active",
        PropertyType::Bool,
        [](const BehaviourTreeComponent& c) { return c.Active; },
        [](BehaviourTreeComponent& c, const bool& v) { c.Active = v; }
    );

    // ResetOnComplete property
    meta.AddProperty<BehaviourTreeComponent, bool>(
        "ResetOnComplete",
        PropertyType::Bool,
        [](const BehaviourTreeComponent& c) { return c.ResetOnComplete; },
        [](BehaviourTreeComponent& c, const bool& v) { c.ResetOnComplete = v; }
    );

    // TreeAssetGUID property (as string for editor display)
    meta.AddProperty<BehaviourTreeComponent, std::string>(
        "TreeAssetGUID",
        PropertyType::String,
        [](const BehaviourTreeComponent& c) { 
            return std::to_string(c.TreeAssetGUID.m_Value); 
        },
        [](BehaviourTreeComponent& c, const std::string& v) { 
            c.TreeAssetGUID = xresource::instance_guid{ std::stoull(v) }; 
        }
    );

    // LastStatus property (read-only, for debugging)
    meta.AddProperty<BehaviourTreeComponent, std::string>(
        "LastStatus",
        PropertyType::String,
        [](const BehaviourTreeComponent& c) { 
            switch (c.LastStatus) {
                case BTStatus::Success: return std::string("Success");
                case BTStatus::Failure: return std::string("Failure");
                case BTStatus::Running: return std::string("Running");
                default: return std::string("Unknown");
            }
        },
        [](BehaviourTreeComponent& c, const std::string& v) { 
            // Read-only property, no setter logic needed
            (void)c; (void)v;
        }
    );
}

} // namespace Engine
