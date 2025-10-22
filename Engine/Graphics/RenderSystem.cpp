#include "../Graphics/RenderSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"
#include "../Component/MeshRendererComponent.h"

namespace Engine {

	RenderSystem::RenderSystem(Renderer& renderer_ref) : System(), renderer(renderer_ref) {
		m_drawitems.reserve(1000);
	}

	void RenderSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		m_drawitems.clear();

		auto view = scene->GetRegistry().view<TransformComponent, MeshRendererComponent>();

		for (auto entity : view) {
			auto& renderable = view.get<MeshRendererComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);

			// Only render visible meshes
			if (renderable.Visible)
			{
				m_drawitems.push_back({
					renderable.MeshType,
					renderable.Material,
					renderable.Texture,
					transform.WorldTransform
					});
			}
		}

		std::span<DrawItem> drawitem_span(m_drawitems.data(), m_drawitems.size());
		renderer.render_frame(drawitem_span);
	}

	int RenderSystem::GetPriority() const { return 100; }

	const char* RenderSystem::GetName() const { return "RenderSystem"; }
}