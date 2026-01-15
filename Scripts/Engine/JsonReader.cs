/**
 * @file SimpleJsonReader.cs
 * @brief Simple JSON file reader for audio settings (no external libraries needed)
 * @author Jack
 * @date January 2026
 */

using System;
using System.IO;
using System.Collections.Generic;

namespace Engine
{
    /// <summary>
    /// Simple JSON reader - parses basic JSON without external libraries
    /// Supports: numbers, booleans, strings (no nested objects for simplicity)
    /// </summary>
    public static class JsonReader
    {
        /// <summary>
        /// Read a JSON file and return key-value pairs
        /// </summary>
        public static Dictionary<string, string> ReadJson(string filePath)
        {
            var result = new Dictionary<string, string>();

            try
            {
                // Read entire file
                string json = File.ReadAllText(filePath);

                // Remove whitespace and braces
                json = json.Trim();
                json = json.TrimStart('{').TrimEnd('}');

                // Split by commas (each key-value pair)
                string[] pairs = json.Split(',');

                foreach (string pair in pairs)
                {
                    // Split by colon (key : value)
                    string[] keyValue = pair.Split(':');

                    if (keyValue.Length == 2)
                    {
                        // Clean up key (remove quotes and whitespace)
                        string key = keyValue[0].Trim().Trim('"');

                        // Clean up value (remove quotes and whitespace)
                        string value = keyValue[1].Trim().Trim('"');

                        result[key] = value;
                    }
                }

                return result;
            }
            catch (Exception e)
            {
                //Debug.LogError($"Failed to read JSON file: {e.Message}");
                return result;
            }
        }

        /// <summary>
        /// Get a float value from JSON data
        /// </summary>
        public static float GetFloat(Dictionary<string, string> data, string key, float defaultValue = 0f)
        {
            if (data.ContainsKey(key))
            {
                if (float.TryParse(data[key], out float value))
                {
                    return value;
                }
            }
            return defaultValue;
        }

        /// <summary>
        /// Get a boolean value from JSON data
        /// </summary>
        public static bool GetBool(Dictionary<string, string> data, string key, bool defaultValue = false)
        {
            if (data.ContainsKey(key))
            {
                if (bool.TryParse(data[key], out bool value))
                {
                    return value;
                }
            }
            return defaultValue;
        }

        /// <summary>
        /// Get a string value from JSON data
        /// </summary>
        public static string GetString(Dictionary<string, string> data, string key, string defaultValue = "")
        {
            if (data.ContainsKey(key))
            {
                return data[key];
            }
            return defaultValue;
        }
    }
}