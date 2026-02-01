using System.Runtime.CompilerServices;

namespace Engine
{
	/// <summary>
	/// TextComponent bindings for controlling UI text
	/// </summary>
	public static class Text
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Text_SetText(uint entityID, string text);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Text_GetText(uint entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Text_SetFontSize(uint entityID, float size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Text_GetFontSize(uint entityID);

		/// <summary>
		/// Set the text content
		/// </summary>
		public static void SetText(uint entityID, string text)
		{
			Text_SetText(entityID, text);
		}

		/// <summary>
		/// Get the current text content
		/// </summary>
		public static string GetText(uint entityID)
		{
			return Text_GetText(entityID);
		}

		/// <summary>
		/// Set the font size (clamped 1-200)
		/// </summary>
		public static void SetFontSize(uint entityID, float size)
		{
			Text_SetFontSize(entityID, size);
		}

		/// <summary>
		/// Get the current font size
		/// </summary>
		public static float GetFontSize(uint entityID)
		{
			return Text_GetFontSize(entityID);
		}
	}
}