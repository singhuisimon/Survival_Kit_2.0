#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"

#include "../Graphics/Renderer.h" // Dependency injection
#include "../Graphics/DrawItem.h"

namespace Engine {

	class RenderSystem : public System {
	public:
		RenderSystem(Renderer& renderer_ref);

		void OnUpdate(Scene* scene, Timestep ts) override;
		int  GetPriority() const override;
		const char* GetName() const override;

	private:
		Renderer& renderer; // Holds a reference to the renderer -> which is owned by the Application class
		std::vector<DrawItem> m_drawitems;
	};

}