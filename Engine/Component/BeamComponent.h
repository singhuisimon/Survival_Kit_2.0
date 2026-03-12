#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>

#include "../Serialization/ComponentRegistry.h"
#include "../../External/xresource_guid/include/xresource_guid.h"
#include "../Utility/Types.h"

namespace Engine
{

    struct BeamComponent
    {
        static constexpr ComponentTypeID TypeID = ComponentTypeID::Beam;
        static constexpr const char* TypeName = "BeamComponent";

        xresource::instance_guid ComponentGUID;

        // Endpoints (world space)
        glm::vec3 StartPoint = glm::vec3(0.f);
        glm::vec3 EndPoint = glm::vec3(0.f, 0.f, 1.f);

        // Endpoint resolution
        entt::entity TargetEntity = entt::null; // If set, EndPoint is resolved from its transform
        glm::vec3 EndPointOffset = glm::vec3(0.f); // Local offset applied to target (or used as fixed world-space end if no target)
        glm::vec3 StartOffset = glm::vec3(0.f); // Local offset from this entity's transform

        // Geometry
        u32   NumSegments = 2;       // 2 = straight laser, 8-16 = lightning
        float StartWidth = 0.2f;
        float EndWidth = 0.2f;

        // Visual
        glm::vec4 StartColor = glm::vec4(1.f);
        glm::vec4 EndColor = glm::vec4(1.f);

        // Noise (for lightning-style beams)
        float NoiseAmplitude = 0.f;     // 0 = clean beam, >0 = jagged
        float NoiseSpeed = 1.f;     // How fast the noise animates
        float NoiseAccumulator = 0.f;   // Driven by BeamSystem each frame

        // Material
        xresource::instance_guid MaterialGuid = 0;

        // UV scrolling (laser animation)
        float UVScrollSpeed = 0.f;
        float UVScrollOffset = 0.f;     // Driven by BeamSystem each frame

        // Control
        bool Active = true;

        BeamComponent() : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy()) { }
    };

}