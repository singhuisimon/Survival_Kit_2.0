using System.Runtime.CompilerServices;

namespace Engine
{
    /// <summary>
    /// Managed wrapper over CameraComponent.
    /// Native bindings are private; public API is exposed via properties + helpers.
    /// </summary>
    public class Camera : Component
    {
        // -------------------------
        // Native bindings (private)
        // Register as: "Engine.Camera::Camera_*"
        // -------------------------
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Camera_GetEnabled(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetEnabled(uint entityID, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern bool Camera_GetPrimary(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetPrimary(uint entityID, bool primary);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Camera_GetFOV(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetFOV(uint entityID, float fov);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Camera_GetNear(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetNear(uint entityID, float nearPlane);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern float Camera_GetFar(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetFar(uint entityID, float farPlane);

        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_GetTarget(uint entityID, out Vector3 target);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Camera_SetTarget(uint entityID, ref Vector3 target);

        private uint ID => Entity.EntityID;

        // -------------------------
        // Instance properties
        // -------------------------
        public bool Enabled
        {
            get => Camera_GetEnabled(ID);
            set => Camera_SetEnabled(ID, value);
        }

        public bool Primary
        {
            get => Camera_GetPrimary(ID);
            set => Camera_SetPrimary(ID, value);
        }

        public float FOV
        {
            get => Camera_GetFOV(ID);
            set => Camera_SetFOV(ID, value);
        }

        public float NearPlane
        {
            get => Camera_GetNear(ID);
            set => Camera_SetNear(ID, value);
        }

        public float FarPlane
        {
            get => Camera_GetFar(ID);
            set => Camera_SetFar(ID, value);
        }

        public Vector3 Target
        {
            get
            {
                Camera_GetTarget(ID, out Vector3 t);
                return t;
            }
            set
            {
                Camera_SetTarget(ID, ref value);
            }
        }

        // -------------------------
        // Static helpers (ID-only)
        // -------------------------
        public static bool CameraGetEnabled(uint entityID) => Camera_GetEnabled(entityID);
        public static void CameraSetEnabled(uint entityID, bool enabled) => Camera_SetEnabled(entityID, enabled);

        public static bool CameraGetPrimary(uint entityID) => Camera_GetPrimary(entityID);
        public static void  CameraSetPrimary(uint entityID, bool primary) => Camera_SetPrimary(entityID, primary);

        public static float  CameraGetFOV(uint entityID) => Camera_GetFOV(entityID);
        public static void  CameraSetFOV(uint entityID, float fov) => Camera_SetFOV(entityID, fov);

        public static float  CameraGetNear(uint entityID) => Camera_GetNear(entityID);
        public static void  CameraSetNear(uint entityID, float nearPlane) => Camera_SetNear(entityID, nearPlane);

        public static float  CameraGetFar(uint entityID) => Camera_GetFar(entityID);
        public static void  CameraSetFar(uint entityID, float farPlane) => Camera_SetFar(entityID, farPlane);

        public static Vector3 GetTarget(uint entityID)
        {
            Camera_GetTarget(entityID, out Vector3 t);
            return t;
        }

        public static void SetTarget(uint entityID, ref Vector3 target) => Camera_SetTarget(entityID, ref target);
    }
}
