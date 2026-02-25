#pragma once

// Jolt
#include <Jolt/Jolt.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Renderer/DebugRenderer.h>
// DebugRenderer::Vertex uses Float3 but DebugRenderer.h doesn't always include Float3.h.
#include <Jolt/Math/Float3.h>

#include <string_view>

// OpenGL
#include <glad/glad.h>

// GLM
#include <glm/glm.hpp>

// STL
#include <vector>
#include <mutex>
#include <cstdint>

namespace Engine {

	// Small OpenGL implementation of Jolt's DebugRenderer.
	//
	// - Records lines / triangles / geometry submissions from Jolt each frame.
	// - Flushes them using a simple vertex format: position (vec3) + color (rgba8).
	// - Uses OpenGL 4.6 DSA (glCreateBuffers, glNamedBufferData, glVertexArrayAttribFormat, ...).
	//
	// Expected shader interface (your Debug Pass shader):
	//   layout(location=0) in vec3 aPos;
	//   layout(location=1) in vec4 aColor;   // normalized from RGBA8
	//   uniform mat4 u_ViewProjection;
	//   uniform mat4 u_Model;
	//   uniform vec4 u_ModelColor;
	//	 out vec4 vColor;
	//
	class JoltDebugDraw final : public JPH::DebugRenderer
	{
	public:
		struct Settings {
			bool draw_bodies = true;
			bool wireframe = true;
			bool draw_bounding_box = false;
			bool draw_center_of_mass = false;
			bool draw_constraints = true;
			bool draw_constraint_limits = false;
		};

		JoltDebugDraw();
		~JoltDebugDraw() override;

		// Create GL resources (must be called after GL context is ready).
		void InitializeGL();

		// Clear per-frame CPU buffers.
		void BeginFrame(const glm::vec3& camera_world_pos);

		// Call once per frame after drawing to let Jolt release cached debug geometry.
		void EndFrame();

		// Flush recorded primitives to the currently bound framebuffer.
		// program must be your debug shader program handle.
		void Flush(GLuint program, const glm::mat4& view_proj);

		Settings& GetSettings() { return m_settings; }
		const Settings& GetSettings() const { return m_settings; }

	private:
		struct DebugVertex {
			float px, py, pz;
			std::uint8_t r, g, b, a;
		};
		static_assert(sizeof(DebugVertex) == 16, "DebugVertex must be 16 bytes");

		// DebugRenderer::Batch is Ref<RefTargetVirtual>. RefTargetVirtual is a pure virtual interface
		// (AddRef/Release), so we implement it while storing the actual refcount in RefTarget<T>.
		struct GLTriangleBatch final : public JPH::RefTarget<GLTriangleBatch>, public JPH::RefTargetVirtual
		{
			JPH_OVERRIDE_NEW_DELETE

			void AddRef() override { JPH::RefTarget<GLTriangleBatch>::AddRef(); }
			void Release() override { JPH::RefTarget<GLTriangleBatch>::Release(); }

			std::vector<DebugVertex> vertices;
			std::vector<std::uint32_t> indices;

			// GPU
			GLuint vao = 0;
			GLuint vbo = 0;
			GLuint ebo = 0;
			bool uploaded = false;

			void EnsureUploaded();
			void DestroyGL();

			~GLTriangleBatch() override { DestroyGL(); }
		};

		struct GeometryDraw {
			Batch batch; // JPH::DebugRenderer::Batch (Ref<RefTargetVirtual>)
			glm::mat4 model;
			glm::vec4 model_color;
			ECullMode cull_mode;
			EDrawMode draw_mode;
		};

		// JPH::DebugRenderer overrides
		void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
		void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;
		Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
		Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;
		void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor,
			const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
		void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

		// Helpers
		static DebugVertex MakeVertex(JPH::RVec3Arg p, JPH::ColorArg c);
		static DebugVertex MakeVertex(const JPH::Float3& p, JPH::ColorArg c);
		static glm::mat4 ToGlmMat4(JPH::RMat44Arg m);
		static glm::vec4 ToGlmColor(JPH::ColorArg c);

		void UploadDynamic(GLuint vbo, const std::vector<DebugVertex>& verts);

		// Per-frame recorded data
		std::mutex m_mutex;
		std::vector<DebugVertex> m_lines;
		std::vector<DebugVertex> m_tris;
		std::vector<GeometryDraw> m_geometries;

		glm::vec3 m_camera_pos{ 0.0f };
		Settings m_settings{};

		// Dynamic GPU buffers for immediate-mode lines & triangles
		GLuint m_lineVAO = 0;
		GLuint m_lineVBO = 0;
		GLuint m_triVAO = 0;
		GLuint m_triVBO = 0;

		bool m_gl_initialized = false;
	};

} // namespace Engine
