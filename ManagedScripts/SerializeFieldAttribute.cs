using System;

namespace ScriptAPI
{
    /// <summary>
    /// Forces field serialization despite being private.
    /// Similar to Unity's [SerializeField] attribute.
    /// Fields marked with this attribute will be visible and editable in the editor.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class SerializeFieldAttribute : Attribute
    {
        /// <summary>
        /// Optional tooltip to display in the editor.
        /// </summary>
        public string Tooltip { get; set; }

        /// <summary>
        /// Optional display name in the editor (defaults to field name).
        /// </summary>
        public string DisplayName { get; set; }

        /// <summary>
        /// Default constructor.
        /// </summary>
        public SerializeFieldAttribute()
        {
            Tooltip = string.Empty;
            DisplayName = string.Empty;
        }

        /// <summary>
        /// Constructor with tooltip.
        /// </summary>
        /// <param name="tooltip">Tooltip text to display in editor</param>
        public SerializeFieldAttribute(string tooltip)
        {
            Tooltip = tooltip;
            DisplayName = string.Empty;
        }
    }

    /// <summary>
    /// Add range constraints for numeric fields.
    /// Will display as a slider in the editor with min/max values.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class RangeAttribute : Attribute
    {
        /// <summary>
        /// Minimum value for the field.
        /// </summary>
        public float Min { get; }

        /// <summary>
        /// Maximum value for the field.
        /// </summary>
        public float Max { get; }

        /// <summary>
        /// Constructor with min and max values.
        /// </summary>
        /// <param name="min">Minimum value</param>
        /// <param name="max">Maximum value</param>
        public RangeAttribute(float min, float max)
        {
            Min = min;
            Max = max;
        }
    }

    /// <summary>
    /// Hide field in editor even if it's public.
    /// Useful for public fields that shouldn't be modified in the editor.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class HideInInspectorAttribute : Attribute
    {
        public HideInInspectorAttribute() { }
    }

    /// <summary>
    /// Add a header/label above fields in the inspector.
    /// Useful for organizing related fields together.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class HeaderAttribute : Attribute
    {
        public string Text { get; }

        public HeaderAttribute(string text)
        {
            Text = text;
        }
    }

    /// <summary>
    /// Add spacing before this field in the inspector.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field, AllowMultiple = false, Inherited = true)]
    public class SpaceAttribute : Attribute
    {
        public float Height { get; }

        public SpaceAttribute(float height = 10f)
        {
            Height = height;
        }
    }
}