#include "../Graphics/RenderSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/ParticleComponent.h"
#include "Asset/ResourceHelpers.h"

namespace Engine {

	RenderSystem::RenderSystem(Renderer& renderer_ref) : System(), renderer(renderer_ref) {
		m_drawitems.reserve(1000);
	}

	void RenderSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		m_drawitems.clear();
		m_cameralist.clear();

		auto view = scene->GetRegistry().view<TransformComponent, MeshRendererComponent>();

		for (auto entity : view) {
			auto& renderable = view.get<MeshRendererComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);
			

			// Only render visible meshes
			if (renderable.Visible)
			{
				m_drawitems.push_back({
					transform.WorldTransform,
					static_cast<u32>(entity),
					renderable.SubmeshIndex,
					renderable.MeshType,
					renderable.Material,
					renderable.Texture,
					renderable.MeshGuid,
					renderable.MaterialGuid,
					renderable.TextureGuid
					});
			}

			
		}

		// Save all enabled cameras
		auto camView = scene->GetRegistry().view<CameraComponent, TransformComponent>();
		for (auto cam : camView) {

			auto& camera = camView.get<CameraComponent>(cam);
			if (camera.Enabled) {
				m_cameralist.emplace_back(camera);
			}

		}

		auto particleView = scene->GetRegistry().view<ParticleComponent>();

		for (auto entity : particleView) {
			auto& emitter = particleView.get<ParticleComponent>(entity);

			if (!emitter.Particles.empty()) {

				for (auto& particle : emitter.Particles) {

					// Don't render dead particles
					if (!particle.Alive)
						continue;

					m_drawitems.push_back({
						particle.Transform,
						u32_max, // Particles are not associated with an entity for rendering purposes
						0,
						emitter.ParticleType,
						0,
						0,
						0,
						0
						});

				}
			}
		}
		
		std::span<DrawItem> drawitem_span(m_drawitems.data(), m_drawitems.size());
		std::span<CameraComponent> cameralist_span(m_cameralist.data(), m_cameralist.size());
		renderer.render_frame(drawitem_span, cameralist_span);
	}

	int RenderSystem::GetPriority() const { return 151; }

	const char* RenderSystem::GetName() const { return "RenderSystem"; }
}
