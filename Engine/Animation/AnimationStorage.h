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
//#include "../ECS/System.h"
//#include "../ECS/Components.h"

namespace Engine {

	/*---------------------- Animation assets ------------------------*/
// Asked to store these like other assets (similar to meshes/materials)
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
		std::string name;

		// For now, just a list of clips and a single active one
		std::vector<u32> clips; // Stores handles for each clip asset
		int defaultClipIndex = 0;
	};


	// Temporary storage
	extern std::unordered_map<u32, AnimatorController> m_AnimatorControllerStorage;
	extern std::unordered_map<u32, AnimationClip> m_AnimationClipStorage;

}