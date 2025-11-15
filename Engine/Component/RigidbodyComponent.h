/*****************************************************************************/
/*!
\file       RigidbodyComponent.h
\author
\date       2025
\brief      Rigidbody component - physics properties for dynamic objects.

			Stores mass, damping, collision shape selection and basic motion
			state used by the physics system. Designers configure high-level
			behaviour (kinematic, gravity, shape), while gameplay code can
			manipulate velocities and forces directly.

(C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#pragma once

#include "../Asset/ResourceTypes.h"
#include <glm/glm.hpp>

namespace Engine
{
	/*****************************************************************************/
	/*!
	\brief  Collider shape types that can be assigned to a rigidbody.

			Some shapes (AABB, SPHERE, MESH) may be derived from mesh data
			when available, while others use explicit parameters stored on
			the component.
	*/
	/*****************************************************************************/
	enum ColliderType
	{
		AABB = 0, // Automatically builds based on mesh
		BOX = 1,
		SPHERE = 2, // Automatically builds based on mesh
		MESH = 3
	};

	/*****************************************************************************/
	/*!
	\brief  Rigidbody component - defines physics properties for dynamic objects.

	\details
			Contains mass, velocity, and physics flags that control how an
			entity behaves in the physics simulation. Works with the Jolt
			Physics backend or simpler custom physics implementations.

			Typical usage:
			- Designers set mass, damping, collider type and gravity/kinematic
			  flags in the editor.
			- The physics system mirrors this data into the runtime physics
			  world and keeps velocities / poses in sync.
			- Gameplay code calls helpers like AddForce, Stop, or SetVelocity
			  to affect motion at a high level.
	*/
	/*****************************************************************************/
	struct RigidbodyComponent
	{
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

		/// Current angular velocity in world space (radians per second)
		glm::vec3 AngularVelocity;

		/// Linear damping (air resistance for translation)
		float LinearDamping;

		/// Angular damping (air resistance for rotation)
		float AngularDamping;

		/// Bounciness (0 = inelastic, 1 = perfectly elastic)
		float Restitution;

		/// Collider shape selection for designers
		ColliderType Shape;

		/// Box half extents (used when Shape == BOX or as AABB fallback)
		glm::vec3 BoxHalfExtents;

		/// Sphere radius (used as fallback if mesh is missing)
		float SphereRadius;

		/*****************************************************************************/
		/*!
		\brief  Default constructor - creates a standard dynamic rigidbody.

		\details
				Initializes a 1 kg body with gravity enabled, small linear and
				angular damping, low restitution, and a BOX collider with
				0.5 unit half extents on all axes.
		*/
		/*****************************************************************************/
		RigidbodyComponent()
			: ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
			, Mass(1.0f)
			, IsKinematic(false)
			, UseGravity(true)
			, Velocity(0.0f, 0.0f, 0.0f)
			, AngularVelocity(0.0f, 0.0f, 0.0f)
			, LinearDamping(0.05f)
			, AngularDamping(0.05f)
			, Restitution(0.1f)
			, Shape(BOX)                                // default: simple box
			, BoxHalfExtents(0.5f, 0.5f, 0.5f)
			, SphereRadius(0.5f)
		{
		}

		/*****************************************************************************/
		/*!
		\brief  Constructor with custom mass.

		\param  mass
				Initial mass in kilograms.

		\details
				All other properties use the same defaults as the default
				constructor.
		*/
		/*****************************************************************************/
		explicit RigidbodyComponent(float mass)
			: ComponentGUID(xresource::instance_guid::GenerateGUIDCopy())
			, Mass(mass)
			, IsKinematic(false)
			, UseGravity(true)
			, Velocity(0.0f, 0.0f, 0.0f)
			, AngularVelocity(0.0f, 0.0f, 0.0f)
			, LinearDamping(0.05f)
			, AngularDamping(0.05f)
			, Restitution(0.1f)
			, Shape(BOX)
			, BoxHalfExtents(0.5f, 0.5f, 0.5f)
			, SphereRadius(0.5f)
		{
		}

		// ---- existing helpers below unchanged ----

		/*****************************************************************************/
		/*!
		\brief  Sets the mass of the rigidbody.

		\param  mass
				New mass value in kilograms.
		*/
		/*****************************************************************************/
		void SetMass(float mass)
		{
			Mass = mass;
		}

		/*****************************************************************************/
		/*!
		\brief  Returns the current mass of the rigidbody.

		\return Mass in kilograms.
		*/
		/*****************************************************************************/
		float GetMass() const
		{
			return Mass;
		}

		/*****************************************************************************/
		/*!
		\brief  Sets whether this body is kinematic.

		\param  kinematic
				True to make the body kinematic (driven by code), false for
				dynamic (driven by physics).
		*/
		/*****************************************************************************/
		void SetKinematic(bool kinematic)
		{
			IsKinematic = kinematic;
		}

		/*****************************************************************************/
		/*!
		\brief  Checks if this body is kinematic.

		\return True if the body is kinematic, false otherwise.
		*/
		/*****************************************************************************/
		bool IsKinematicBody() const
		{
			return IsKinematic;
		}

		/*****************************************************************************/
		/*!
		\brief  Enables or disables gravity for this body.

		\param  enabled
				True to enable gravity, false to disable.
		*/
		/*****************************************************************************/
		void SetGravityEnabled(bool enabled)
		{
			UseGravity = enabled;
		}

		/*****************************************************************************/
		/*!
		\brief  Checks if gravity is enabled for this body.

		\return True if gravity is enabled, false otherwise.
		*/
		/*****************************************************************************/
		bool IsGravityEnabled() const
		{
			return UseGravity;
		}

		/*****************************************************************************/
		/*!
		\brief  Sets the linear velocity of the body.

		\param  velocity
				New linear velocity in world space (units per second).
		*/
		/*****************************************************************************/
		void SetVelocity(const glm::vec3 &velocity)
		{
			Velocity = velocity;
		}

		/*****************************************************************************/
		/*!
		\brief  Gets the current linear velocity of the body.

		\return Reference to the linear velocity vector.
		*/
		/*****************************************************************************/
		const glm::vec3 &GetVelocity() const
		{
			return Velocity;
		}

		/*****************************************************************************/
		/*!
		\brief  Immediately stops the body.

		\details
				Sets linear and angular velocity to zero.
		*/
		/*****************************************************************************/
		void Stop()
		{
			Velocity = glm::vec3(0.0f);
			AngularVelocity = glm::vec3(0.0f);
		}

		/*****************************************************************************/
		/*!
		\brief  Applies a simple force-like impulse to the body.

		\param  force
				Force vector to apply.

		\details
				Updates velocity using v += force / Mass. This assumes a unit
				timestep and is intended as a lightweight helper rather than a
				full force integration API.
		*/
		/*****************************************************************************/
		void AddForce(const glm::vec3 &force)
		{
			Velocity += force / Mass;
		}

		/*****************************************************************************/
		/*!
		\brief  Adds a delta to the current linear velocity.

		\param  deltaVelocity
				Change in velocity to add.
		*/
		/*****************************************************************************/
		void AddVelocity(const glm::vec3 &deltaVelocity)
		{
			Velocity += deltaVelocity;
		}

		/*****************************************************************************/
		/*!
		\brief  Returns the scalar speed of the body.

		\return Magnitude (length) of the linear velocity vector.
		*/
		/*****************************************************************************/
		float GetSpeed() const
		{
			return glm::length(Velocity);
		}

		/*****************************************************************************/
		/*!
		\brief  Checks if the body is currently moving.

		\return True if |Velocity| > 0.001f, false otherwise.
		*/
		/*****************************************************************************/
		bool IsMoving() const
		{
			return glm::length(Velocity) > 0.001f; // Small epsilon for floating point comparison
		}

		/*****************************************************************************/
		/*!
		\brief  Checks if the body is effectively static.

		\details
				A body is considered static here if its mass is non-positive and
				it is not kinematic. Kinematic bodies are controlled explicitly
				and thus are not treated as static even if mass <= 0.
		\return True if the body is static, false otherwise.
		*/
		/*****************************************************************************/
		bool IsStatic() const
		{
			return Mass <= 0.0f && !IsKinematic;
		}
	};

} // namespace Engine
