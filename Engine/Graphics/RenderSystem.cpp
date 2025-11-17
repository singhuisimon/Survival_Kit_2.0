#include "../Graphics/RenderSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/LightComponent.h"   
#include "Asset/ResourceHelpers.h"

namespace Engine {

	RenderSystem::RenderSystem(Renderer& renderer_ref) : System(), renderer(renderer_ref) {
		m_drawitems.reserve(1000);
		m_cameralist.reserve(64);
		m_lightlist.reserve(64);
	}

	void RenderSystem::OnUpdate(Scene* scene, Timestep ts) {

		(void)ts;

		m_drawitems.clear();
		m_cameralist.clear();
		m_lightlist.clear();

		// Save all visible geometry 
		auto view = scene->GetRegistry().view<TransformComponent, MeshRendererComponent>();
		for (auto entity : view) {
			auto& renderable = view.get<MeshRendererComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);

			// Capture visible geometry information
			if (!renderable.Visible) continue;
			
			m_drawitems.push_back({
					.m_model_to_world_transform = transform.WorldTransform,
					.m_entity_id = static_cast<u32>(entity),
					.m_submesh_index = renderable.SubmeshIndex,
					.m_default_mesh_handle = renderable.MeshType,
					.m_default_material_handle = renderable.Material,
					.m_default_u32texture_handle = renderable.Texture,
					.m_mesh_guid = renderable.MeshGuid,
					.m_material_guid = renderable.MaterialGuid,
					.m_texture_guid = renderable.TextureGuid
				});
			
		}

		// Save all enabled cameras
		auto camView = scene->GetRegistry().view<CameraComponent, TransformComponent>();
		for (auto cam : camView) {

			auto& camera = camView.get<CameraComponent>(cam);
			if (!camera.Enabled) continue;

			// Capture camera information and it's position
			auto& transform = camView.get<TransformComponent>(cam);
			m_cameralist.emplace_back(std::make_pair(camera, transform.Position));
		}

		// Save all particle emitter information
		auto particleView = scene->GetRegistry().view<ParticleComponent>();
		for (auto entity : particleView) {
			auto& emitter = particleView.get<ParticleComponent>(entity);

			if (!emitter.Particles.empty()) {

				for (auto& particle : emitter.Particles) {

					// Don't collect information about dead particles
					if (!particle.Alive)
						continue;

					// Capture particle information
					m_drawitems.push_back({
						.m_model_to_world_transform = particle.Transform,
						.m_entity_id = u32_max, // Particles are not associated with an entity for rendering purposes
						.m_submesh_index = 0,
						.m_default_mesh_handle = emitter.ParticleType,
						.m_default_material_handle = 0,
						.m_default_u32texture_handle = 0,
						.m_mesh_guid = 0,
						.m_material_guid = 0
						});

				}
			}
		}

		// Save all enabled lights
		auto lightView = scene->GetRegistry().view<TransformComponent, LightComponent>();
		for (auto entity : lightView) {

			// Get transform and light component
			const auto& LgTransform = lightView.get<TransformComponent>(entity);
			const auto& LgLightComp = lightView.get<LightComponent>(entity);
			if (!LgLightComp.Enabled) continue;

			// Create CPU side LightBlock
			LightCPU L{};
			L.type = static_cast<uint32_t>(LgLightComp.Type);
			L.color = LgLightComp.Color;
			L.intensity = LgLightComp.Intensity;

			// World position and -Z forward as direction
			const glm::vec3 worldPos = glm::vec3(LgTransform.WorldTransform[3]);
			const glm::vec3 fwd = glm::normalize(glm::vec3(LgTransform.WorldTransform * glm::vec4(0, 0, -1, 0)));
			L.position = worldPos;
			L.direction = fwd;

			// Store range info (Necessary for Point and Spot lights)
			L.range = LgLightComp.Range;
			const float outerRad = glm::radians(LgLightComp.SpotAngleDeg);
			const float innerRad = glm::radians(LgLightComp.SpotAngleDeg * 0.85f);
			L.cosInner = std::cos(innerRad);
			L.cosOuter = std::cos(outerRad);

			// Save indirect multiplier
			L.indirectMultiplier = LgLightComp.IndirectMultiplier;
			m_lightlist.emplace_back(L);
		}
		
		std::span<DrawItem> drawitem_span(m_drawitems.data(), m_drawitems.size());
		std::span<std::pair<CameraComponent, glm::vec3>> cameralist_span(m_cameralist.data(), m_cameralist.size());
		std::span<LightCPU>			light_span(m_lightlist.data(), m_lightlist.size());

		renderer.render_frame(drawitem_span, cameralist_span, light_span);
	}

	int RenderSystem::GetPriority() const { return 151; }

	const char* RenderSystem::GetName() const { return "RenderSystem"; }
}
