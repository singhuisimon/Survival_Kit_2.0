#include  "../ParticleSystem/TrailSystem.h"

#include "../ECS/Scene.h"

namespace Engine
{
	void TrailSystem::OnUpdate(Scene* scene, Timestep ts)
	{
		auto view = scene->GetRegistry().view<TrailComponent, TransformComponent>();
		float dt = ts.GetSeconds();

		static glm::vec3 lastPrintedPos(0);

		for (auto e : view)
		{
			auto& trail = view.get<TrailComponent>(e);
			auto& transform = view.get<TransformComponent>(e);

			//glm::vec3 currentPos = transform.Position;
			//glm::vec3 delta = currentPos - lastPrintedPos;

			//std::cout << "Frame delta: (" << delta.x << ", " << delta.y << ", " << delta.z << ")\n";
			//std::cout << "Delta magnitude: " << glm::length(delta) << "\n";

			//lastPrintedPos = currentPos;

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

			// === STEP 2: Time-based sampling with distance check ===
			if (trail.EmitTrail) {
				trail.SampleAccumulator += dt;

				glm::vec3 currentPosition = transform.Position;

				// **CRITICAL: Check if we've moved enough distance**
				bool shouldSample = false;

				if (trail.Segments.empty()) {
					// First segment ever
					shouldSample = true;
				}
				else if (trail.SampleAccumulator >= trail.SampleInterval) {
					// Time threshold met, now check distance
					glm::vec3 lastSegmentPos = trail.Segments.back().Position;
					float distanceMoved = glm::distance(currentPosition, lastSegmentPos);

					// **Only sample if moved at least MinDistance**
					if (distanceMoved >= trail.MinDistance) {
						shouldSample = true;
					}
				}

				if (shouldSample) {
					// Enforce max segments limit
					if (trail.Segments.size() >= trail.MaxSegments) {
						trail.Segments.erase(trail.Segments.begin());
					}

					// Add new segment
					TrailSegment newSegment;
					newSegment.Position = currentPosition;
					newSegment.TimeStamp = 0.0f;
					newSegment.Width = trail.StartWidth;
					newSegment.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

					trail.Segments.push_back(newSegment);

					// Reset accumulator
					trail.SampleAccumulator = 0.0f;
				}
			}

			// === STEP 3: Skip rendering if not enough segments ===
			if (trail.Segments.size() < 2) {
				continue;  // Need at least 2 segments to render
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
