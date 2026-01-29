#include "../Graphics/RenderSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"
#include "../Component/MeshRendererComponent.h"
#include "../Component/ParticleComponent.h"
#include "../Component/LightComponent.h"   
#include "../Component/SpriteRendererComponent.h"
#include "../Component/TrailComponent.h"
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

		// Save all visible geometry and shadow casters
		auto view = scene->GetRegistry().view<TransformComponent, MeshRendererComponent>();
		for (auto entity : view) {
			auto& renderable = view.get<MeshRendererComponent>(entity);
			auto& transform = view.get<TransformComponent>(entity);

			// Determine shadow cast type (and check if entity casts shadow) and if shadow is casted without entity
			const uint32_t castType = static_cast<uint32_t>(renderable.CastType);
			const bool isShadowCaster = (castType != 0u);
			const bool renderMainPass = renderable.Visible && (castType != static_cast<uint32_t>(ShadowCastType::ShadowsOnly));

			// Skip completely disabled objects
			if (!renderable.Visible && !isShadowCaster) { continue; }
			
			m_drawitems.push_back({
					.m_model_to_world_transform = transform.WorldTransform,
					.m_drawitem_type = DrawItemType::MESH3D,
					.m_entity_id = static_cast<u32>(entity),
					.m_submesh_index = renderable.SubmeshIndex,
					.m_default_mesh_handle = renderable.MeshType,
					.m_default_material_handle = renderable.Material,
					.m_default_u32texture_handle = renderable.Texture,
					.m_mesh_guid = renderable.MeshGuid,
					.m_material_guid = renderable.MaterialGuid,
					.m_texture_guid = renderable.TextureGuid,
					.m_render_main_pass = renderMainPass,
					.m_receive_shadows = renderable.ShadowReceive,
					.m_cast_shadow_type = castType
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
						.m_drawitem_type = DrawItemType::Particle,
						.m_entity_id = u32_max, // Particles are not associated with an entity for rendering purposes
						.m_submesh_index = 0,
						.m_default_mesh_handle = emitter.ParticleType,
						.m_default_material_handle = 0,
						.m_default_u32texture_handle = 0,
						.m_color = particle.Color,
						.m_mesh_guid = emitter.ParticleTypeAdvanced,
						.m_material_guid = emitter.MaterialType,
						.m_render_main_pass = true, // Particles don't cast/receive shadows
						.m_receive_shadows = false,
						.m_cast_shadow_type = 0
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

			// Shadow settings (used by renderer)
			L.shadowType = static_cast<uint32_t>(LgLightComp.TypeShadow);
			L.shadowResolution = LgLightComp.Resolution;
			L.shadowStrength = LgLightComp.Strength;
			//L.shadowBias = LgLightComp.Bias; // Individual light bias, for now only support global bias
			L.shadowBias = renderer.getGlobalBias();
			L.shadowNearPlane = LgLightComp.NearPlane;

			m_lightlist.emplace_back(L);
		}

		// Save all 2D items
		//auto spriteView = scene->GetRegistry().view<TransformComponent, SpriteRendererComponent>();

		auto spriteView = scene->GetRegistry().group<TransformComponent, SpriteRendererComponent>();

		spriteView.sort<SpriteRendererComponent>([](SpriteRendererComponent const& a, SpriteRendererComponent const& b) {return a.SpriteLayer < b.SpriteLayer; });

		for (auto entity : spriteView) 
		{
			auto& renderable2d = spriteView.get<SpriteRendererComponent>(entity);
			auto& r2dtransform = spriteView.get<TransformComponent>(entity);

			if (!renderable2d.IsVisible) continue;

			m_drawitems.push_back({
				.m_model_to_world_transform = r2dtransform.WorldTransform,
				.m_drawitem_type = DrawItemType::SPRITE2D,
				.m_entity_id = static_cast<u32>(entity),
				.m_submesh_index = 0,
				.m_default_mesh_handle = renderable2d.Quad,
				.m_default_material_handle = 0,
				.m_default_u32texture_handle = 0,
				.m_render_layer = renderable2d.SpriteLayer,
				.m_color = renderable2d.Color,
				.m_mesh_guid = 0,
				.m_material_guid = 0,
				.m_texture_guid = renderable2d.TextureGuid
				});
		}

		//collect all text entities
		auto textView = scene->GetRegistry().view<TransformComponent, TextComponent>();
		for (auto entity : textView) {
			auto& textComp = textView.get<TextComponent>(entity);
			auto& transform = textView.get<TransformComponent>(entity);

			//skip empty text
			if (textComp.text.empty()) continue; 

			m_drawitems.push_back({
				.m_model_to_world_transform = transform.WorldTransform,
				.m_drawitem_type = DrawItemType::TEXT,
				.m_entity_id = static_cast<u32>(entity),
				.m_submesh_index = 0,
				.m_default_mesh_handle = 0,
				.m_default_material_handle = 0,
				.m_default_u32texture_handle = 0,
				.m_render_layer = 0,
				.m_color = glm::vec4(textComp.color[0], textComp.color[1],
									textComp.color[2], textComp.color[3]),
				.m_mesh_guid = 0,
				.m_material_guid = 0,
				.m_texture_guid = 0,

				.m_text = textComp.text,
				.m_fontSize = textComp.fontSize,
				.m_textAlignment = textComp.align,
				.m_lineSpacing = textComp.lineSpacing,
				.m_letterSpacing = textComp.letterSpacing,
				.m_maxWidth = textComp.maxWidth

				});
		}

		auto trailView = scene->GetRegistry().view<TrailComponent>();
		for (auto entity : trailView) {
			auto& trail = trailView.get<TrailComponent>(entity);

			if (!trail.Active || trail.Segments.size() < 2)
				continue;

			// Store trail as a single draw item
			m_drawitems.push_back({
				.m_model_to_world_transform = glm::mat4(1.0f),  // Segments have world positions
				.m_drawitem_type = DrawItemType::TRAIL,
				.m_entity_id = static_cast<u32>(entity),
				.m_submesh_index = 0,
				.m_default_mesh_handle = 0,  // Not using default mesh
				.m_default_material_handle = 0,
				.m_default_u32texture_handle = 0,
				.m_mesh_guid = 0,
				.m_material_guid = trail.MaterialGuid,
				.m_render_main_pass = true,
				.m_receive_shadows = false,
				.m_cast_shadow_type = 0,

				// Store trail data pointer for rendering
				.m_trail_data = &trail
				});
		}
		
		std::span<DrawItem> drawitem_span(m_drawitems.data(), m_drawitems.size());
		std::span<std::pair<CameraComponent, glm::vec3>> cameralist_span(m_cameralist.data(), m_cameralist.size());
		std::span<LightCPU>			light_span(m_lightlist.data(), m_lightlist.size());

		renderer.render_frame(drawitem_span, cameralist_span, light_span);
	}

	int RenderSystem::GetPriority() const { return 151; }

	const char* RenderSystem::GetName() const { return "RenderSystem"; }
}
