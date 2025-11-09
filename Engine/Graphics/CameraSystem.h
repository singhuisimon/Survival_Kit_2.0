/**
 * @file CameraSystem.h
 * @brief	Declaration of Camera System that manages Camera components in the ECS
 * @details Updates and rebuild component's View and Perpsective transform upon modification
 * @author Chua Wen Bin Kenny
 * @date 20 October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"

namespace Engine {

	class CameraSystem : public System {
	public:
		CameraSystem();

		void OnUpdate(Scene* scene, Timestep ts) override;
		int  GetPriority() const override;
		const char* GetName() const override;

	private:
	};

}