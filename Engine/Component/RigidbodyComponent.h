/**
 * @file RigidbodyComponent.h
 * @brief Rigidbody component - physics properties for dynamic objects
 * @author
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "../Asset/ResourceTypes.h"
#include <glm/glm.hpp>

namespace Engine {

    /**
     * @brief Rigidbody component - defines physics properties for dynamic objects
     * @details Contains mass, velocity, and physics flags that control how an
     *          entity behaves in the physics simulation. Works with Jolt Physics
     *          or can be used for simple custom physics implementations.
     */
    struct RigidbodyComponent {
        /// Unique identifier for this component instance
        xresource::instance_guid ComponentGUID;

        /// Mass of the object in kilograms
        float Mass;

        /// Whether this body is kinematic (moved by code, not physics)
        bool IsKinematic;

        /// Whether gravity affects this body
        bool UseGravity;

        /// Current velocity in world space (units per second)
        glm::vec3 Velocity;

        // Future fields (planned for Jolt Physics integration):
        // glm::vec3 AngularVelocity;     // Rotation velocity
        // float LinearDamping;            // Air resistance for linear motion
        // float AngularDamping;           // Air resistance for rotation
        // float Restitution;              // Bounciness (0 = no bounce, 1 = perfect bounce)
        // float Friction;                 // Surface friction coefficient
        // bool IsSleeping;                // Whether physics body is sleeping
        // CollisionShape Shape;           // Collision shape type

        /**
         * @brief Default constructor - creates a standard dynamic rigidbody
         */
        RigidbodyComponent()
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Mass(1.0f)
            , IsKinematic(false)
            , UseGravity(true)
            , Velocity(0.0f, 0.0f, 0.0f) {
        }

        /**
         * @brief Constructor with custom mass
         * @param mass Initial mass in kilograms
         */
        explicit RigidbodyComponent(float mass)
            : ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
            , Mass(mass)
            , IsKinematic(false)
            , UseGravity(true)
            , Velocity(0.0f, 0.0f, 0.0f) {
        }

        /**
         * @brief Set the mass
         * @param mass New mass in kilograms
         */
        void SetMass(float mass) {
            Mass = mass;
        }

        /**
         * @brief Get the mass
         * @return Current mass in kilograms
         */
        float GetMass() const {
            return Mass;
        }

        /**
         * @brief Set kinematic mode
         * @param kinematic Whether this body should be kinematic
         */
        void SetKinematic(bool kinematic) {
            IsKinematic = kinematic;
        }

        /**
         * @brief Check if body is kinematic
         * @return True if kinematic (controlled by code, not physics)
         */
        bool IsKinematicBody() const {
            return IsKinematic;
        }

        /**
         * @brief Enable/disable gravity
         * @param enabled Whether gravity should affect this body
         */
        void SetGravityEnabled(bool enabled) {
            UseGravity = enabled;
        }

        /**
         * @brief Check if gravity is enabled
         * @return True if gravity affects this body
         */
        bool IsGravityEnabled() const {
            return UseGravity;
        }

        /**
         * @brief Set the velocity
         * @param velocity New velocity in world space
         */
        void SetVelocity(const glm::vec3& velocity) {
            Velocity = velocity;
        }

        /**
         * @brief Get the velocity
         * @return Current velocity in world space
         */
        const glm::vec3& GetVelocity() const {
            return Velocity;
        }

        /**
         * @brief Add force to the velocity (impulse)
         * @param force Force vector to add
         */
        void AddForce(const glm::vec3& force) {
            Velocity += force / Mass;
        }

        /**
         * @brief Add velocity directly
         * @param deltaVelocity Velocity to add
         */
        void AddVelocity(const glm::vec3& deltaVelocity) {
            Velocity += deltaVelocity;
        }

        /**
         * @brief Stop all movement
         */
        void Stop() {
            Velocity = glm::vec3(0.0f);
        }

        /**
         * @brief Get speed (magnitude of velocity)
         * @return Current speed
         */
        float GetSpeed() const {
            return glm::length(Velocity);
        }

        /**
         * @brief Check if body is moving
         * @return True if velocity is non-zero
         */
        bool IsMoving() const {
            return glm::length(Velocity) > 0.001f; // Small epsilon for floating point comparison
        }

        /**
         * @brief Check if this is a static body (zero mass, non-kinematic)
         * @return True if body should be treated as static
         */
        bool IsStatic() const {
            return Mass <= 0.0f && !IsKinematic;
        }
    };

} // namespace Engine