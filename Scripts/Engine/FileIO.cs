/**
 * @file FileIO.cs
 * @brief Simple file I/O wrapper for engine scripts
 * @author Jack
 * @date January 2026
 */

using System.Runtime.CompilerServices;

namespace Engine
{
	/// <summary>
	/// Provides basic file I/O operations for scripts
	/// </summary>
	public static class FileIO
	{
		/// <summary>
		/// Checks if a file exists at the given path
		/// </summary>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern bool FileExists(string path);

		/// <summary>
		/// Reads entire file content as a string
		/// </summary>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern string ReadAllText(string path);

		/// <summary>
		/// Writes string content to a file (creates directories if needed)
		/// </summary>
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern bool WriteAllText(string path, string content);
	}
}
