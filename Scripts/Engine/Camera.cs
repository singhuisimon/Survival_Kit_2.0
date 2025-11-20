using System;

namespace Engine
{
	/// <summary>
	/// Managed wrapper over CameraComponent.
	/// </summary>
	public class Camera : Component
	{
		public bool Enabled
		{
			get { return InternalCalls.Camera_GetEnabled(Entity.EntityID); }
			set { InternalCalls.Camera_SetEnabled(Entity.EntityID, value); }
		}

		public bool Primary
		{
			get { return InternalCalls.Camera_GetPrimary(Entity.EntityID); }
			set { InternalCalls.Camera_SetPrimary(Entity.EntityID, value); }
		}

		public float FOV
		{
			get { return InternalCalls.Camera_GetFOV(Entity.EntityID); }
			set { InternalCalls.Camera_SetFOV(Entity.EntityID, value); }
		}

		public float NearPlane
		{
			get { return InternalCalls.Camera_GetNear(Entity.EntityID); }
			set { InternalCalls.Camera_SetNear(Entity.EntityID, value); }
		}

		public float FarPlane
		{
			get { return InternalCalls.Camera_GetFar(Entity.EntityID); }
			set { InternalCalls.Camera_SetFar(Entity.EntityID, value); }
		}

		public Vector3 Target
		{
			get
			{
				Vector3 t;
				InternalCalls.Camera_GetTarget(Entity.EntityID, out t);
				return t;
			}
			set
			{
				InternalCalls.Camera_SetTarget(Entity.EntityID, ref value);
			}
		}
	}
}
