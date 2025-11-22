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
#include <array>

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

	// 2D keyframe for UV-related tracks (tiling / offset)
	struct UVKeyframe
	{
		float time = 0.0f;
		std::array<float, 2> value{ 0.0f, 0.0f }; // [u, v]
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

		// Base filename (without extension) this clip was last loaded/saved from.
		std::string fileName;

		// Transform tracks
		std::vector<PositionKeyframe> positionKeys;
		std::vector<RotationKeyframe> rotationKeys;
		std::vector<ScaleKeyframe>    scaleKeys;

		// UV Transform keyframes
		std::vector<UVKeyframe>       uvTilingKeys; // Tiling (U, V)
		std::vector<UVKeyframe>       uvOffsetKeys; // Offset (U, V)

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

		// Base filename (without extension) this clip was last loaded/saved from.
		std::string fileName;

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

	// Save a single clip and animator controller
	void SaveAnimationClipAsset(AnimationClip& clip);
	void SaveAnimatorControllerAsset(AnimatorController& controller);


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

		inline std::array<float, 2> ParseFloat2(const std::string& arr)
		{
			std::array<float, 2> result{ 0.0f, 0.0f };
			size_t start = 0;
			for (int i = 0; i < 2; ++i) {
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

		// ---------------- positionKeys ----------------
		oss << "  \"positionKeys\": [\n";
		for (size_t i = 0; i < clip.positionKeys.size(); ++i) {
			const auto& k = clip.positionKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.position.x << ", " << k.position.y << ", " << k.position.z << "] }";
			if (i + 1 < clip.positionKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// ---------------- rotationKeys (store as [w,x,y,z]) ----------------
		oss << "  \"rotationKeys\": [\n";
		for (size_t i = 0; i < clip.rotationKeys.size(); ++i) {
			const auto& k = clip.rotationKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.rotation.w << ", " << k.rotation.x << ", "
				<< k.rotation.y << ", " << k.rotation.z << "] }";
			if (i + 1 < clip.rotationKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// ---------------- scaleKeys ----------------
		oss << "  \"scaleKeys\": [\n";
		for (size_t i = 0; i < clip.scaleKeys.size(); ++i) {
			const auto& k = clip.scaleKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.scale.x << ", " << k.scale.y << ", " << k.scale.z << "] }";
			if (i + 1 < clip.scaleKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// ---------------- uvTilingKeys (value = [u,v]) ----------------
		oss << "  \"uvTilingKeys\": [\n";
		for (size_t i = 0; i < clip.uvTilingKeys.size(); ++i) {
			const auto& k = clip.uvTilingKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.value[0] << ", " << k.value[1] << "] }";
			if (i + 1 < clip.uvTilingKeys.size()) oss << ",";
			oss << "\n";
		}
		oss << "  ],\n";

		// ---------------- uvOffsetKeys (value = [u,v]) ----------------
		oss << "  \"uvOffsetKeys\": [\n";
		for (size_t i = 0; i < clip.uvOffsetKeys.size(); ++i) {
			const auto& k = clip.uvOffsetKeys[i];
			oss << "    { \"time\": " << k.time
				<< ", \"value\": [" << k.value[0] << ", " << k.value[1] << "] }";
			if (i + 1 < clip.uvOffsetKeys.size()) oss << ",";
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

		// Remember base file name (without extension) for editor use
		{
			std::filesystem::path p(filePath);
			outClip.fileName = p.stem().string();
		}

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
		// so nested [x,y,z] / [w,x,y,z] / [u,v] don't break us.
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

		// ---------------- positionKeys ----------------
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

		// ---------------- rotationKeys ----------------
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

		// ---------------- scaleKeys ----------------
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

		// ---------------- uvTilingKeys ----------------
		outClip.uvTilingKeys.clear();
		parseKeyArray("uvTilingKeys",
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
				auto v = ParseFloat2(arrStr);

				UVKeyframe k;
				k.time = time;
				k.value = { v[0], v[1] };
				outClip.uvTilingKeys.push_back(k);
			});

		// ---------------- uvOffsetKeys ----------------
		outClip.uvOffsetKeys.clear();
		parseKeyArray("uvOffsetKeys",
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
				auto v = ParseFloat2(arrStr);

				UVKeyframe k;
				k.time = time;
				k.value = { v[0], v[1] };
				outClip.uvOffsetKeys.push_back(k);
			});

		// Sort all tracks by time just in case
		auto sortByTime = [](auto& keys)
			{
				std::sort(keys.begin(), keys.end(),
					[](const auto& a, const auto& b) { return a.time < b.time; });
			};

		sortByTime(outClip.positionKeys);
		sortByTime(outClip.rotationKeys);
		sortByTime(outClip.scaleKeys);
		sortByTime(outClip.uvTilingKeys);
		sortByTime(outClip.uvOffsetKeys);

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

		// Remember base file name (without extension) for editor use
		{
			std::filesystem::path p(filePath);
			outController.fileName = p.stem().string();
		}

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

	// -----------------------------------------------------------------------------
	// Helper: Save a clip asset. "Save" semantics:
	// - If clip.fileName is empty -> first save: use clip.name (or UnnamedClip).
	// - If clip.fileName != clip.name -> delete old file (both paths) and then save
	//   under the new name.
	// - Always updates clip.fileName to the new base name.
	// -----------------------------------------------------------------------------
	inline void SaveAnimationClipAsset(AnimationClip& clip)
	{
		namespace fs = std::filesystem;

		std::string newBase = clip.name.empty() ? "UnnamedClip" : clip.name;

		// Delete old file if the asset has been renamed
		if (!clip.fileName.empty() && clip.fileName != newBase)
		{
			std::string oldPath1 = "../../Resources/Sources/AnimationClips/" + clip.fileName + ".animclip";
			std::string oldPath2 = "../bin/Debug/Resources/Sources/AnimationClips/" + clip.fileName + ".animclip";

			std::error_code ec;
			fs::remove(oldPath1, ec);
			fs::remove(oldPath2, ec);
		}

		// Update runtime file name and write the new file
		clip.fileName = newBase;

		std::string path1 = "../../Resources/Sources/AnimationClips/" + newBase + ".animclip";
		std::string path2 = "../bin/Debug/Resources/Sources/AnimationClips/" + newBase + ".animclip";

		SerializeAnimationClip(clip, path1);
		SerializeAnimationClip(clip, path2);
	}

	// -----------------------------------------------------------------------------
	// Helper: Save a controller asset with same semantics as above.
	// -----------------------------------------------------------------------------
	inline void SaveAnimatorControllerAsset(AnimatorController& controller)
	{
		namespace fs = std::filesystem;

		std::string newBase = controller.name.empty() ? "NewController" : controller.name;

		if (!controller.fileName.empty() && controller.fileName != newBase)
		{
			std::string oldPath1 = "../../Resources/Sources/AnimationControllers/" + controller.fileName + ".animcontroller";
			std::string oldPath2 = "../bin/Debug/Resources/Sources/AnimationControllers/" + controller.fileName + ".animcontroller";

			std::error_code ec;
			fs::remove(oldPath1, ec);
			fs::remove(oldPath2, ec);
		}

		controller.fileName = newBase;

		std::string path1 = "../../Resources/Sources/AnimationControllers/" + newBase + ".animcontroller";
		std::string path2 = "../bin/Debug/Resources/Sources/AnimationControllers/" + newBase + ".animcontroller";

		SerializeAnimationController(controller, path1);
		SerializeAnimationController(controller, path2);
	}


} // namespace Engine