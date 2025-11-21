/**
 * @file AnimationStorage.h
 * @brief	Declaration of Camera System that manages Camera components in the ECS
 * @details Updates and rebuild component's View and Perpsective transform upon modification
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cctype>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {

	/*---------------------- Animation assets ------------------------*/
	struct PositionKeyframe {
		float     time;      // seconds
		glm::vec3 position;
	};

	struct RotationKeyframe {
		float time;          // seconds from start of clip
		glm::quat rotation;  // or glm::vec3 euler if you prefer
	};

	struct ScaleKeyframe {
		float     time;
		glm::vec3 scale;
	};

	// Support interpolation modes (Store per-track instead of per-key)
	enum class InterpMode {
		Step,       // hold previous value until next key
		Linear,     // simple linear (or slerp for rotation)
		// Smooth,  // for later: cubic / Hermite / Bezier etc.
	};

	// Clip asset
	struct AnimationClip {

		// Stable asset id to match the handle used in m_AnimationClipStorage. (Will eventually port to assets pipeline)
		u32         id = 0;

		std::string name;
		float duration = 0.0f;          // total length in seconds
		bool loop = true;               // loop or not

		// Transform tracks
		std::vector<PositionKeyframe> positionKeys;
		std::vector<RotationKeyframe> rotationKeys;
		std::vector<ScaleKeyframe>    scaleKeys;

		InterpMode positionInterp = InterpMode::Linear;
		InterpMode rotationInterp = InterpMode::Linear;  // implies slerp
		InterpMode scaleInterp = InterpMode::Linear;
	};
	/*---------------------- Animation assets ------------------------*/

	// Animator controller that handles all animation clips in the scene
	struct AnimatorController {

		// Stable asset id to match the handle used in m_AnimatorControllerStorage. (Will eventually port to assets pipeline)
		u32         id = 0;

		std::string name;

		// For now, just a list of clips and a single active one
		std::vector<u32> clips; // Stores handles for each clip asset
		int defaultClipIndex = 0;
	};

	// Serialize / deserialize a single clip to a JSON-ish text file.
	bool SerializeAnimationClip(const AnimationClip& clip, const std::string& filePath);
	bool DeserializeAnimationClip(const std::string& filePath, AnimationClip& outClip);

	// Serialize / deserialize a single controller to a JSON-ish text file.
	bool SerializeAnimationController(const AnimatorController& controller, const std::string& filePath);
	bool DeserializeAnimationController(const std::string& filePath, AnimatorController& outController);

	// Temporary storage
	extern std::unordered_map<u32, AnimatorController> m_AnimatorControllerStorage;
	extern std::unordered_map<u32, AnimationClip> m_AnimationClipStorage;

	// =====================================================================
	//  Inline implementation section
	//  (Small ad-hoc JSON writer/reader sufficient for temp asset files.)
	// =====================================================================

	namespace Detail
	{
		inline const char* InterpModeToString(InterpMode mode)
		{
			switch (mode) {
			case InterpMode::Step:   return "Step";
			case InterpMode::Linear: return "Linear";
			default:                 return "Linear";
			}
		}

		inline InterpMode InterpModeFromString(const std::string& s)
		{
			if (s == "Step")   return InterpMode::Step;
			if (s == "Linear") return InterpMode::Linear;
			return InterpMode::Linear;
		}

		inline bool WriteTextFile(const std::string& path, const std::string& text)
		{
			std::ofstream ofs(path, std::ios::binary);
			if (!ofs) return false;
			ofs << text;
			return ofs.good();
		}

		inline bool ReadTextFile(const std::string& path, std::string& out)
		{
			std::ifstream ifs(path, std::ios::binary);
			if (!ifs) return false;
			std::ostringstream ss;
			ss << ifs.rdbuf();
			out = ss.str();
			return true;
		}

		inline std::string Trim(const std::string& s)
		{
			size_t b = 0, e = s.size();
			while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
			while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
			return s.substr(b, e - b);
		}

		inline bool ExtractStringField(const std::string& src, const std::string& key, std::string& out)
		{
			const std::string pattern = "\"" + key + "\"";
			size_t pos = src.find(pattern);
			if (pos == std::string::npos) return false;
			pos = src.find(':', pos);
			if (pos == std::string::npos) return false;
			pos = src.find('"', pos);
			if (pos == std::string::npos) return false;
			size_t end = src.find('"', pos + 1);
			if (end == std::string::npos) return false;
			out = src.substr(pos + 1, end - pos - 1);
			return true;
		}

		inline bool ExtractFloatField(const std::string& src, const std::string& key, float& out)
		{
			const std::string pattern = "\"" + key + "\"";
			size_t pos = src.find(pattern);
			if (pos == std::string::npos) return false;
			pos = src.find(':', pos);
			if (pos == std::string::npos) return false;
			++pos;
			while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;

			size_t end = pos;
			while (end < src.size()) {
				char c = src[end];
				if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E'))
					break;
				++end;
			}
			if (end == pos) return false;
			out = std::stof(src.substr(pos, end - pos));
			return true;
		}

		inline bool ExtractIntField(const std::string& src, const std::string& key, int& out)
		{
			float f = 0.0f;
			if (!ExtractFloatField(src, key, f)) return false;
			out = static_cast<int>(f);
			return true;
		}

		inline bool ExtractBoolField(const std::string& src, const std::string& key, bool& out)
		{
			const std::string pattern = "\"" + key + "\"";
			size_t pos = src.find(pattern);
			if (pos == std::string::npos) return false;
			pos = src.find(':', pos);
			if (pos == std::string::npos) return false;
			++pos;
			while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;

			if (src.compare(pos, 4, "true") == 0) {
				out = true;
				return true;
			}
			if (src.compare(pos, 5, "false") == 0) {
				out = false;
				return true;
			}
			return false;
		}

		inline bool ExtractU32Field(const std::string& src, const std::string& key, u32& out)
		{
			int tmp = 0;
			if (!ExtractIntField(src, key, tmp)) return false;
			out = static_cast<u32>(tmp);
			return true;
		}

		inline std::array<float, 3> ParseFloat3(const std::string& arr)
		{
			std::array<float, 3> result{ 0.0f, 0.0f, 0.0f };
			size_t start = 0;
			for (int i = 0; i < 3; ++i) {
				size_t comma = arr.find(',', start);
				std::string token = (comma == std::string::npos)
					? arr.substr(start)
					: arr.substr(start, comma - start);
				token = Trim(token);
				if (!token.empty())
					result[static_cast<size_t>(i)] = std::stof(token);
				if (comma == std::string::npos) break;
				start = comma + 1;
			}
			return result;
		}

		inline std::array<float, 4> ParseFloat4(const std::string& arr)
		{
			std::array<float, 4> result{ 0.0f, 0.0f, 0.0f, 0.0f };
			size_t start = 0;
			for (int i = 0; i < 4; ++i) {
				size_t comma = arr.find(',', start);
				std::string token = (comma == std::string::npos)
					? arr.substr(start)
					: arr.substr(start, comma - start);
				token = Trim(token);
				if (!token.empty())
					result[static_cast<size_t>(i)] = std::stof(token);
				if (comma == std::string::npos) break;
				start = comma + 1;
			}
			return result;
		}

		inline std::vector<u32> ParseU32Array(const std::string& arr)
		{
			std::vector<u32> out;
			size_t start = 0;
			while (start < arr.size()) {
				size_t comma = arr.find(',', start);
				std::string token = (comma == std::string::npos)
					? arr.substr(start)
					: arr.substr(start, comma - start);
				token = Trim(token);
				if (!token.empty()) {
					u32 v = static_cast<u32>(std::stoul(token));
					out.push_back(v);
				}
				if (comma == std::string::npos) break;
				start = comma + 1;
			}
			return out;
		}

		inline std::string EscapeString(const std::string& s)
		{
			// Very simple escape: only backslash and double-quote.
			std::string out;
			out.reserve(s.size());
			for (char c : s) {
				if (c == '\\' || c == '\"')
					out.push_back('\\');
				out.push_back(c);
			}
			return out;
		}

	} // namespace Detail

	/*---------------------- Serializers and Deserializers ----------------------*/
	inline bool SerializeAnimationClip(const AnimationClip& clip, const std::string& filePath)
	{
		using namespace Detail;

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"id\": " << clip.id << ",\n";
		oss << "  \"name\": \"" << EscapeString(clip.name) << "\",\n";
		oss << "  \"duration\": " << clip.duration << ",\n";
		oss << "  \"loop\": " << (clip.loop ? "true" : "false") << ",\n";
		oss << "  \"positionInterp\": \"" << InterpModeToString(clip.positionInterp) << "\",\n";
		oss << "  \"rotationInterp\": \"" << InterpModeToString(clip.rotationInterp) << "\",\n";
		oss << "  \"scaleInterp\": \"" << InterpModeToString(clip.scaleInterp) << "\",\n";

		// positionKeys
		oss << "  \"positionKeys\": [\n";
		for (size_t i = 0; i < clip.positionKeys.size(); ++i) {
			const auto& k = clip.positionKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.position.x << ", " << k.position.y << ", " << k.position.z << "] }";
			if (i + 1 < clip.positionKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// rotationKeys (store as [w,x,y,z])
		oss << "  \"rotationKeys\": [\n";
		for (size_t i = 0; i < clip.rotationKeys.size(); ++i) {
			const auto& k = clip.rotationKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.rotation.w << ", " << k.rotation.x << ", " << k.rotation.y << ", " << k.rotation.z << "] }";
			if (i + 1 < clip.rotationKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// scaleKeys
		oss << "  \"scaleKeys\": [\n";
		for (size_t i = 0; i < clip.scaleKeys.size(); ++i) {
			const auto& k = clip.scaleKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.scale.x << ", " << k.scale.y << ", " << k.scale.z << "] }";
			if (i + 1 < clip.scaleKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ]\n";
		oss << "}\n";

		return WriteTextFile(filePath, oss.str());
	}


	inline bool DeserializeAnimationClip(const std::string& filePath, AnimationClip& outClip)
	{
		using namespace Detail;

		std::string text;
		if (!ReadTextFile(filePath, text))
			return false;

		outClip = AnimationClip{}; // reset

		// Basic fields
		ExtractU32Field(text, "id", outClip.id);
		ExtractStringField(text, "name", outClip.name);
		ExtractFloatField(text, "duration", outClip.duration);
		ExtractBoolField(text, "loop", outClip.loop);

		std::string interpStr;
		if (ExtractStringField(text, "positionInterp", interpStr))
			outClip.positionInterp = InterpModeFromString(interpStr);
		if (ExtractStringField(text, "rotationInterp", interpStr))
			outClip.rotationInterp = InterpModeFromString(interpStr);
		if (ExtractStringField(text, "scaleInterp", interpStr))
			outClip.scaleInterp = InterpModeFromString(interpStr);

		// ----------------------------------------------------------------
		// Helper to parse key arrays: find the *matching* closing ']' 
		// so nested [x,y,z] / [w,x,y,z] don't break us.
		// ----------------------------------------------------------------
		auto parseKeyArray = [&text](const std::string& key, auto addKeyFn)
			{
				const std::string pattern = "\"" + key + "\"";
				size_t pos = text.find(pattern);
				if (pos == std::string::npos) return;

				// Find the '[' that starts the array
				pos = text.find('[', pos);
				if (pos == std::string::npos) return;

				// Find the matching ']' with bracket depth
				int depth = 0;
				size_t end = std::string::npos;
				for (size_t i = pos; i < text.size(); ++i)
				{
					char c = text[i];
					if (c == '[')
						++depth;
					else if (c == ']')
					{
						--depth;
						if (depth == 0)
						{
							end = i;
							break;
						}
					}
				}
				if (end == std::string::npos) return;

				// Extract the contents between the outer '[' and ']'
				std::string arr = text.substr(pos + 1, end - pos - 1);

				size_t cursor = 0;
				while (true)
				{
					size_t objStart = arr.find('{', cursor);
					if (objStart == std::string::npos) break;
					size_t objEnd = arr.find('}', objStart);
					if (objEnd == std::string::npos) break;

					std::string obj = arr.substr(objStart + 1, objEnd - objStart - 1);
					addKeyFn(obj);

					cursor = objEnd + 1;
				}
			};

		// positionKeys
		outClip.positionKeys.clear();
		parseKeyArray("positionKeys",
			[&](const std::string& obj)
			{
				float time = 0.0f;
				ExtractFloatField(obj, "time", time);

				size_t vPos = obj.find("\"value\"");
				if (vPos == std::string::npos) return;
				vPos = obj.find('[', vPos);
				if (vPos == std::string::npos) return;
				size_t vEnd = obj.find(']', vPos);
				if (vEnd == std::string::npos) return;

				std::string arrStr = obj.substr(vPos + 1, vEnd - vPos - 1);
				auto v = ParseFloat3(arrStr);

				PositionKeyframe k;
				k.time = time;
				k.position = glm::vec3(v[0], v[1], v[2]);
				outClip.positionKeys.push_back(k);
			});

		// rotationKeys (w,x,y,z)
		outClip.rotationKeys.clear();
		parseKeyArray("rotationKeys",
			[&](const std::string& obj)
			{
				float time = 0.0f;
				ExtractFloatField(obj, "time", time);

				size_t vPos = obj.find("\"value\"");
				if (vPos == std::string::npos) return;
				vPos = obj.find('[', vPos);
				if (vPos == std::string::npos) return;
				size_t vEnd = obj.find(']', vPos);
				if (vEnd == std::string::npos) return;

				std::string arrStr = obj.substr(vPos + 1, vEnd - vPos - 1);
				auto v = ParseFloat4(arrStr);

				RotationKeyframe k;
				k.time = time;
				k.rotation = glm::quat(v[0], v[1], v[2], v[3]); // w,x,y,z
				outClip.rotationKeys.push_back(k);
			});

		// scaleKeys
		outClip.scaleKeys.clear();
		parseKeyArray("scaleKeys",
			[&](const std::string& obj)
			{
				float time = 0.0f;
				ExtractFloatField(obj, "time", time);

				size_t vPos = obj.find("\"value\"");
				if (vPos == std::string::npos) return;
				vPos = obj.find('[', vPos);
				if (vPos == std::string::npos) return;
				size_t vEnd = obj.find(']', vPos);
				if (vEnd == std::string::npos) return;

				std::string arrStr = obj.substr(vPos + 1, vEnd - vPos - 1);
				auto v = ParseFloat3(arrStr);

				ScaleKeyframe k;
				k.time = time;
				k.scale = glm::vec3(v[0], v[1], v[2]);
				outClip.scaleKeys.push_back(k);
			});

		// (Optional but nice) sort keys by time just in case
		auto sortByTime = [](auto& keys)
			{
				std::sort(keys.begin(), keys.end(),
					[](const auto& a, const auto& b) { return a.time < b.time; });
			};
		sortByTime(outClip.positionKeys);
		sortByTime(outClip.rotationKeys);
		sortByTime(outClip.scaleKeys);

		return true;
	}


	inline bool SerializeAnimationController(const AnimatorController& controller, const std::string& filePath)
	{
		using namespace Detail;

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"id\": " << controller.id << ",\n";
		oss << "  \"name\": \"" << EscapeString(controller.name) << "\",\n";
		oss << "  \"clips\": [";

		for (size_t i = 0; i < controller.clips.size(); ++i) {
			oss << controller.clips[i];
			if (i + 1 < controller.clips.size())
				oss << ", ";
		}
		oss << "],\n";
		oss << "  \"defaultClipIndex\": " << controller.defaultClipIndex << "\n";
		oss << "}\n";

		return WriteTextFile(filePath, oss.str());
	}


	inline bool DeserializeAnimationController(const std::string& filePath, AnimatorController& outController)
	{
		using namespace Detail;

		std::string text;
		if (!ReadTextFile(filePath, text))
			return false;

		outController = AnimatorController{};

		ExtractU32Field(text, "id", outController.id);
		ExtractStringField(text, "name", outController.name);
		ExtractIntField(text, "defaultClipIndex", outController.defaultClipIndex);

		// clips array
		const std::string pattern = "\"clips\"";
		size_t pos = text.find(pattern);
		if (pos != std::string::npos) {
			pos = text.find('[', pos);
			if (pos != std::string::npos) {
				size_t end = text.find(']', pos);
				if (end != std::string::npos) {
					std::string arrStr = text.substr(pos + 1, end - pos - 1);
					outController.clips = ParseU32Array(arrStr);
				}
			}
		}

		return true;
	}

} // namespace Engine