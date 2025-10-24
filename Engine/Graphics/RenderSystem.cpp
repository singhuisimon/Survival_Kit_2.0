#include "../Graphics/RenderSystem.h"
#include "../ECS/Scene.h"

namespace Engine {

	RenderSystem::RenderSystem(Renderer& renderer_ref) : System(), renderer(renderer_ref) { m_drawitems.reserve(1000); }

	void RenderSystem::OnUpdate(Scene* scene, Timestep ts) {
		
		(void)ts;

		m_drawitems.clear();
		m_cameralist.clear();

		auto view = scene->GetRegistry().view<TransformComponent, MeshRendererComponent>();

		for (auto entity : view) {
			auto& renderable = view.get<MeshRendererComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);

			m_drawitems.push_back({
				renderable.MeshType,
				renderable.Material,
				renderable.Texture,
				transform.WorldTransform
				});
		}

		// Save all enabled cameras
		auto camView = scene->GetRegistry().view<CameraComponent, TransformComponent>();
		for (auto cam : camView) {

			auto& camera = camView.get<CameraComponent>(cam);
			if (camera.Enabled) {
				m_cameralist.emplace_back(camera);
			}

		}
		
		std::span<DrawItem> drawitem_span(m_drawitems.data(), m_drawitems.size());
		std::span<CameraComponent> cameralist_span(m_cameralist.data(), m_cameralist.size());
		renderer.render_frame(drawitem_span, cameralist_span);
	}

	int RenderSystem::GetPriority() const { return 101; }

	const char* RenderSystem::GetName() const { return "RenderSystem"; }
}