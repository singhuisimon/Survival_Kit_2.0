using System;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over native RigidbodyComponent.
    /// Exposes mass, kinematic/gravity flags and velocity helpers.
    /// </summary>
    public class Rigidbody : Component
    {
        /// <summary>
        /// Mass in kilograms.
        /// Backed by RigidbodyComponent::Mass.
        /// </summary>
        public float Mass
        {
            get => InternalCalls.Rigidbody_GetMass(Entity.EntityID);
            set => InternalCalls.Rigidbody_SetMass(Entity.EntityID, value);
        }

        /// <summary>
        /// Whether this body is kinematic (moved by scripts instead of physics).
        /// Backed by RigidbodyComponent::IsKinematic.
        /// </summary>
        public bool IsKinematic
        {
            get => InternalCalls.Rigidbody_GetIsKinematic(Entity.EntityID);
            set => InternalCalls.Rigidbody_SetIsKinematic(Entity.EntityID, value);
        }

        /// <summary>
        /// Whether gravity affects this body.
        /// Backed by RigidbodyComponent::UseGravity.
        /// </summary>
        public bool UseGravity
        {
            get => InternalCalls.Rigidbody_GetUseGravity(Entity.EntityID);
            set => InternalCalls.Rigidbody_SetUseGravity(Entity.EntityID, value);
        }

        /// <summary>
        /// Linear velocity in world space.
        /// Backed by RigidbodyComponent::Velocity.
        /// </summary>
        public Vector3 Velocity
        {
            get
            {
                Vector3 v;
                InternalCalls.Rigidbody_GetVelocity(Entity.EntityID, out v);
                return v;
            }
            set
            {
                InternalCalls.Rigidbody_SetVelocity(Entity.EntityID, ref value);
            }
        }

        /// <summary>
        /// Current speed (length of Velocity).
        /// Backed by RigidbodyComponent::GetSpeed().
        /// </summary>
        public float Speed => InternalCalls.Rigidbody_GetSpeed(Entity.EntityID);

        /// <summary>
        /// True if the body is currently moving.
        /// </summary>
        public bool IsMoving => InternalCalls.Rigidbody_IsMoving(Entity.EntityID);

        /// <summary>
        /// True if treated as static (zero mass and non-kinematic).
        /// </summary>
        public bool IsStatic => InternalCalls.Rigidbody_IsStatic(Entity.EntityID);

        /// <summary>
        /// Adds directly to the current velocity (v += delta).
        /// </summary>
        public void AddVelocity(Vector3 delta)
        {
            InternalCalls.Rigidbody_AddVelocity(Entity.EntityID, ref delta);
        }

        /// <summary>
        /// Applies a force (affects velocity based on mass).
        /// Backed by RigidbodyComponent::AddForce().
        /// </summary>
        public void AddForce(Vector3 force)
        {
            InternalCalls.Rigidbody_AddForce(Entity.EntityID, ref force);
        }

        /// <summary>
        /// Stops all linear motion (Velocity = 0).
        /// </summary>
        public void Stop()
        {
            InternalCalls.Rigidbody_Stop(Entity.EntityID);
        }
    }
}
