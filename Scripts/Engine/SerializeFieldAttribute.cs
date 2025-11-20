using System;

namespace Engine
{
	/// <summary>
	/// Marks a private field to be exposed and editable in the engine editor.
	/// Similar to Unity's [SerializeField] attribute.
	/// </summary>
	[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property, AllowMultiple = false)]
	public class SerializeFieldAttribute : Attribute
	{
		/// <summary>
		/// Optional display name in the editor. If not set, uses the field name.
		/// </summary>
		public string DisplayName { get; set; }

		public SerializeFieldAttribute() { }

		public SerializeFieldAttribute(string displayName)
		{
			DisplayName = displayName;
		}
	}
}
