#include "../Component/BeamComponent.h"
#include "../Graphics/BeamSystem.h"
#include "../ECS/Scene.h"

namespace Engine
{

    void BeamSystem::OnUpdate(Scene* scene, Timestep ts)
    {
        auto& registry = scene->GetRegistry();
        auto view = registry.view<BeamComponent, TransformComponent>();
        float dt = ts.GetSeconds();

        for (auto e : view)
        {
            auto& beam = view.get<BeamComponent>(e);
            auto& transform = view.get<TransformComponent>(e);

            if (!beam.Active)
                continue;

            // Resolve start from own transform
            beam.StartPoint = transform.Position + beam.StartOffset;

            // Resolve end from target entity if valid, else use EndPointOffset as fixed world point
            if (beam.TargetEntity != entt::null && registry.valid(beam.TargetEntity))
            {
                auto* targetTransform = registry.try_get<TransformComponent>(beam.TargetEntity);
                if (targetTransform)
                    beam.EndPoint = targetTransform->Position + beam.EndPointOffset;
            }
            else
            {
                beam.EndPoint = beam.EndPointOffset;
            }

            // Advance accumulators
            beam.UVScrollOffset += beam.UVScrollSpeed * dt;
            beam.NoiseAccumulator += beam.NoiseSpeed * dt;
        }
    }

    int BeamSystem::GetPriority() const { return 103; }

    const char* BeamSystem::GetName() const { return "BeamSystem"; }
}