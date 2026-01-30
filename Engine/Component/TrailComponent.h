#pragma once
#include <glm/glm.hpp>

#include "../../External/xresource_guid/include/xresource_guid.h"
#include "../Utility/Types.h"

namespace Engine
{

	struct TrailSegment
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		float TimeStamp;
		float Width;
	};

	struct TrailComponent
	{
		// Buffer of segments
		std::vector<TrailSegment> Segments;

		// Config
		u32 MaxSegments = 50;
		float SegmentLifetime = 1.f;
		float MinDistance = 0.1f;
		float SampleInterval = 0.016f;

		float SampleAccumulator = 0.0f;

		xresource::instance_guid MaterialGuid = 0;

		// Visual Properties
		glm::vec4 StartColor = glm::vec4(1.f);
		glm::vec4 EndColor = glm::vec4(1.f, 1.f, 1.f, 0.f);
		float StartWidth = 0.5f;
		float EndWidth = 0.1f;

		// Control
		bool Active = true;
		bool EmitTrail = true;

		// Last known position
		glm::vec3 LastPosition = glm::vec3(0.f);
		bool HasLastPosition = false;

		glm::vec3 LocalOffset = glm::vec3(0.f);
	};

}
