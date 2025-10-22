#include <glm/gtc/constants.hpp>

#include "../Graphics/Primitives.h"
#include "../Utility/MathUtils.h"

namespace Engine {

	MeshData make_cube() {

		MeshData m;

		m.positions = {

			// +X face
			{+0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,-0.5f},
			// -X face
			{-0.5f,-0.5f,+0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f,+0.5f,-0.5f}, {-0.5f,+0.5f,+0.5f},
			// +Y face
			{-0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,+0.5f}, {-0.5f,+0.5f,+0.5f},
			// -Y face
			{-0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f},
			// +Z face
			{+0.5f,-0.5f,+0.5f}, {-0.5f,-0.5f,+0.5f}, {-0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f},
			// -Z face
			{-0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, {-0.5f,+0.5f,-0.5f}

		};

		m.normals = {

			// +X
			{+1,0,0}, {+1,0,0}, {+1,0,0}, {+1,0,0},
			// -X
			{-1,0,0}, {-1,0,0}, {-1,0,0}, {-1,0,0},
			// +Y
			{0,+1,0}, {0,+1,0}, {0,+1,0}, {0,+1,0},
			// -Y
			{0,-1,0}, {0,-1,0}, {0,-1,0}, {0,-1,0},
			// +Z
			{0,0,+1}, {0,0,+1}, {0,0,+1}, {0,0,+1},
			// -Z
			{0,0,-1}, {0,0,-1}, {0,0,-1}, {0,0,-1}

		};


		m.colors = {
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f),
			glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f), glm::vec3(0.5f)
		};

		m.texcoords = {
			// +X
			{0,0}, {1,0}, {1,1}, {0,1},
			// -X
			{0,0}, {1,0}, {1,1}, {0,1},
			// +Y
			{0,0}, {1,0}, {1,1}, {0,1},
			// -Y
			{0,0}, {1,0}, {1,1}, {0,1},
			// +Z
			{0,0}, {1,0}, {1,1}, {0,1},
			// -Z
			{0,0}, {1,0}, {1,1}, {0,1}
		};

		m.indices = {
			// +X face
			0, 2, 1,  0, 3, 2,
			// -X face
			4, 6, 5,  4, 7, 6,
			// +Y face
			8, 10, 9,  8, 11, 10,
			// -Y face
			12, 14, 13,  12, 15, 14,
			// +Z face
			16, 18, 17,  16, 19, 18,
			// -Z face
			20, 22, 21,  20, 23, 22,
		};

		return m;
	}

	MeshData make_plane() {

		MeshData m;

		m.positions = {
			{-0.5f, 0, -0.5f},
			{+0.5f, 0, -0.5f},
			{+0.5f, 0, +0.5f},
			{-0.5f, 0, +0.5f}
		};

		m.normals = {
			{0, +1, 0},
			{0, +1, 0},
			{0, +1, 0},
			{0, +1, 0}
		};

		m.colors = {
			glm::vec3(0.5f),
			glm::vec3(0.5f),
			glm::vec3(0.5f),
			glm::vec3(0.5f)
		};

		m.texcoords = {
			{0, 0},
			{1, 0},
			{1, 1},
			{0, 1}
		};

		m.indices = {
			0, 2, 1,
			0, 3, 2
		};

		return m;
	}

	MeshData make_sphere() {

		// Stack is the vertical/longitudinal axis
		// Sectors is the horizontal/lateral axis
		MeshData m;

		const int stacks = 60;
		const int sectors = 50;

		const float radius = 1.0f;
		const float lengthInv = 1.0f / radius;

		float sector_factor = MathUtils::TWO_PI / sectors;
		float stack_factor = MathUtils::PI / stacks;

		float sector_angle, stack_angle; // sector angle is theta, stack angle is phi

		for (int i = 0; i <= stacks; ++i) {

			stack_angle = glm::pi<float>() / 2.0f - i * stack_factor;
			float xy = radius * std::cosf(stack_angle);
			float z = radius * std::sinf(stack_angle);

			for (int j = 0; j <= sectors; ++j) {

				sector_angle = sector_factor * j;
				float x = xy * std::cosf(sector_angle);
				float y = xy * std::sinf(sector_angle);
				m.positions.push_back({ x,y,z });

				float nx = x * lengthInv;
				float ny = y * lengthInv;
				float nz = z * lengthInv;
				m.normals.push_back({ nx, ny, nz });

				m.colors.push_back(glm::vec3(0.5f));

				float s = static_cast<float>(j) / sectors;
				float t = static_cast<float>(i) / stacks;
				m.texcoords.push_back({ s, t });
			}
		}

		// Generate index buffer
		uint32_t v1, v2;
		for (int i = 0; i < stacks; ++i) {

			v1 = i * (sectors + 1); // beginning of current stack
			v2 = v1 + (sectors + 1); // beginning of next stack

			for (int j = 0; j < sectors; ++j, ++v1, ++v2) {

				// 2 triangles per sectors excluding first and last stacks
				// v1 => v2 => v1 + 1
				if (i != 0) {

					m.indices.push_back(v1);
					m.indices.push_back(v2);
					m.indices.push_back(v1 + 1);
				}

				if (i != (stacks - 1)) {

					m.indices.push_back(v1 + 1);
					m.indices.push_back(v2);
					m.indices.push_back(v2 + 1);
				}


				// Future consideration for line indices.
			}

		}

		return m;
	}

}