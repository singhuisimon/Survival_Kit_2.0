#pragma once

#include <glm/glm.hpp>                  // Core types: vec, mat
#include <glm/gtc/matrix_transform.hpp> // glm::lookAt, glm::perspective, translate/scale/rotate
#include <glm/common.hpp>               // glm::clamp
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr

// Math Utility
#include "../Utility/MathUtils.h"

// Support setting of camera uniforms
#include "../Graphics/ShaderProgram.h"


namespace Engine {

    /**
     * @brief Camera component
     */
    struct CameraComponent {

        // Toggles and flags
        bool Enabled;           // Enable/disable a camera    
        // bool Projection;     // (Future): 0 = perspective, 1 = orthographic
        bool autoAspect;        // Toggle true allows resizing by system
        bool isDirty;           // Dirty if modified from outside
        
        // Projection data
        // glm::vec2 Size;      // (Future): viewport size of Camera when projection is orthographic 
        u32 Depth;              // Camera draw order
        float Aspect;           // Aspect ratio (Default = 16:9)
        float FOV;              // Field of view angle of camera in degrees
        float NearPlane;        // Near clipping plane
        float FarPlane;         // Far clipping plane

        // Output targets
        // u32 TargetTexture       // (Future): Reference to texture where camera output will be drawn
        //enum class ClearFlags {Skybox, Color, DepthOnly, Nothing}; // Determine what to clear
        glm::vec3 Target;       // target the camera is looking at (Allow transform to do it)

        // Derived data
        glm::mat4 View = glm::mat4(1.0f);
        glm::mat4 Persp = glm::mat4(1.0f);
    
        // Default constructor for a default 3D camera
        CameraComponent() : 
            Enabled { true },
            autoAspect { true },
            isDirty { true },
            Depth { 0 },
            Aspect {16.0f / 9.0f},
            FOV{ 45.0f },
            NearPlane{ 0.5f },
            FarPlane{ 100.0f },
            Target{ 0.0f, 0.0f, 0.0f },
            View { glm::mat4{1.0f} },
            Persp { glm::mat4{1.0f} }
        {}

        // Constructor for a 3D camera with custom values
        CameraComponent(bool enabled,
            bool autoaspect,
            bool dirty,
            u32 depth,
            float aspect,
            float fov,
            float near,
            float far,
            glm::vec3 target,
            glm::mat4 v,
            glm::mat4 p) :

            Enabled { enabled },
            autoAspect { autoaspect },
            isDirty { dirty },
            Depth { depth },
            Aspect { aspect },
            FOV{ fov },
            NearPlane{ near },
            FarPlane{ far },
            Target{ target },
            View{ v },
            Persp{ p }
        {}

        //// Setters
        //void SetEnabled(bool enabled) { Enabled = enabled; isDirty = true; }
        //void SetAutoAspect(bool autoaspect) { autoAspect = autoaspect; isDirty = true; }
        //void SetDirty(bool dirty) { isDirty = dirty; }
        //void SetDepth(u32 depth) { Depth = depth; isDirty = true; }
        //void SetAspect(float aspect) { Aspect = aspect; isDirty = true; }
        //void SetFOV(float fov) { FOV = fov; isDirty = true; }
        //void SetNearPlane(float near) { NearPlane = near; isDirty = true; }
        //void SetFarPlane(float far) { FarPlane = far; isDirty = true; }
        //void SetTarget(const glm::vec3& target) { Target = target; isDirty = true; }
        //void SetView(const glm::mat4& view) { View = view; isDirty = true; }
        //void SetPerspective(const glm::mat4& persp) { Persp = persp; isDirty = true; }

        //// Getters
        //bool GetEnabled() const { return Enabled; }
        //bool GetAutoAspect() const { return autoAspect; }
        //bool GetDirty() const { return isDirty; }
        //u32 GetDepth() const { return Depth; }
        //float GetAspect() const { return Aspect; }
        //float GetFOV() const { return FOV; }
        //float GetNearPlane() const { return NearPlane; }
        //float GetFarPlane() const { return FarPlane; }
        //const glm::vec3& GetTarget() const { return Target; }
        //const glm::mat4& GetView() const { return View; }
        //const glm::mat4& GetPerspective() const { return Persp; }
    };

}

