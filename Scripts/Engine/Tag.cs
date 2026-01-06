using System.Runtime.CompilerServices;

namespace Engine
{
    public class Tag : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern string Tag_GetTag(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Tag_SetTag(uint entityID, string tag);

        // Botnet-friendly static helpers
        public static string TagGetTag(uint entityID) => Tag_GetTag(entityID);
        public static void TagSetTag(uint entityID, string tag) => Tag_SetTag(entityID, tag ?? string.Empty);

        private uint ID => Entity.EntityID;

        public string Value
        {
            get => Tag_GetTag(ID);
            set => Tag_SetTag(ID, value ?? string.Empty);
        }

        public override string ToString() => Value;
    }
}
