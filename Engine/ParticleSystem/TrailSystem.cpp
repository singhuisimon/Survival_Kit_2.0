#include  "../ParticleSystem/TrailSystem.h"

#include "../ECS/Scene.h"

namespace Engine
{
	void TrailSystem::OnUpdate(Scene* scene, Timestep ts)
	{
		auto view = scene->GetRegistry().view<TrailComponent, TransformComponent>();
		float dt = ts.GetSeconds();

		for (auto e : view)
		{
			auto& trail = view.get<TrailComponent>(e);
			auto& transform = view.get<TransformComponent>(e);

			if (!trail.Active)
				continue;

			// Age all segments
			for (auto it = trail.Segments.begin(); it != trail.Segments.end();)
			{
				it->TimeStamp += dt;

				// Removal logic
				if (it->TimeStamp > trail.SegmentLifetime){
					it = trail.Segments.erase(it);
				}
				else {
					++it;
				}
			}

			if (trail.EmitTrail) {
				glm::vec3 currentPosition = transform.Position;

				bool shouldSample = false;

				if (!trail.HasLastPosition) {
					// Progenitor segment
					shouldSample = true;
					trail.LastPosition = currentPosition;
					trail.HasLastPosition = true;
				}
				else {
					// Check distance threshold
					float distance = glm::distance(currentPosition, trail.LastPosition);
					if (distance >= trail.MinDistance) {
						shouldSample = true;
					}
				}

				if (shouldSample) {
					if (trail.Segments.size() >= trail.MaxSegments)
						trail.Segments.erase(trail.Segments.begin());

					TrailSegment newSegment;
					newSegment.Position = currentPosition;
					newSegment.TimeStamp = 0.f;
					newSegment.Width = trail.StartWidth;
					newSegment.Normal = glm::vec3(0.f, 1.f, 0.f); // Placeholder val
					trail.Segments.push_back(newSegment);
					trail.LastPosition = currentPosition;
				}
			}

			for (size_t i = 0; i < trail.Segments.size(); ++i)
			{
				glm::vec3 tangent;

				if (i == 0 && trail.Segments.size() > 1) {
					// First segment: use direction to next
					tangent = glm::normalize(trail.Segments[i + 1].Position - trail.Segments[i].Position);
				}
				else if (i == trail.Segments.size() - 1 && trail.Segments.size() > 1) {
					// Last segment: use direction from previous
					tangent = glm::normalize(trail.Segments[i].Position - trail.Segments[i - 1].Position);
				}
				else if (trail.Segments.size() > 2) {
					// Middle segments: average of directions
					glm::vec3 toPrev = glm::normalize(trail.Segments[i].Position - trail.Segments[i - 1].Position);
					glm::vec3 toNext = glm::normalize(trail.Segments[i + 1].Position - trail.Segments[i].Position);
					tangent = glm::normalize(toPrev + toNext);
				}
				else {
					tangent = glm::vec3(0.0f, 0.0f, 1.0f);  // Fallback
				}

				// Store tangent in Normal field (we'll compute actual normal in shader based on camera)
				trail.Segments[i].Normal = tangent;

				// Update width based on lifetime (linear interpolation)
				float normalizedAge = trail.Segments[i].TimeStamp / trail.SegmentLifetime;
				trail.Segments[i].Width = glm::mix(trail.StartWidth, trail.EndWidth, normalizedAge);
			}
		}
	}

	int TrailSystem::GetPriority() const { return 102; }

	const char* TrailSystem::GetName() const { return "TrailSystem"; }
}
