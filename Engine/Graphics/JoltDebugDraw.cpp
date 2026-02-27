#include "../Graphics/JoltDebugDraw.h"

#include <glm/gtc/type_ptr.hpp>

namespace Engine {

	static void SetupDebugVAO(GLuint vao, GLuint vbo)
	{
		// Vertex format: position (3x float) at location 0, color (4x u8 normalized) at location 1
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, 16);

		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribFormat(vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 12);
		glVertexArrayAttribBinding(vao, 1, 0);
	}

	JoltDebugDraw::JoltDebugDraw()
	{
		Initialize(); // Jolt DebugRenderer initialization
	}

	JoltDebugDraw::~JoltDebugDraw()
	{
		if (m_lineVBO) glDeleteBuffers(1, &m_lineVBO);
		if (m_triVBO) glDeleteBuffers(1, &m_triVBO);
		if (m_lineVAO) glDeleteVertexArrays(1, &m_lineVAO);
		if (m_triVAO) glDeleteVertexArrays(1, &m_triVAO);
	}

	void JoltDebugDraw::InitializeGL()
	{
		if (m_gl_initialized) return;

		glCreateVertexArrays(1, &m_lineVAO);
		glCreateBuffers(1, &m_lineVBO);
		glNamedBufferData(m_lineVBO, 0, nullptr, GL_STREAM_DRAW);
		SetupDebugVAO(m_lineVAO, m_lineVBO);

		glCreateVertexArrays(1, &m_triVAO);
		glCreateBuffers(1, &m_triVBO);
		glNamedBufferData(m_triVBO, 0, nullptr, GL_STREAM_DRAW);
		SetupDebugVAO(m_triVAO, m_triVBO);

		m_gl_initialized = true;
	}

	void JoltDebugDraw::BeginFrame(const glm::vec3& camera_world_pos)
	{
		std::scoped_lock lock(m_mutex);
		m_camera_pos = camera_world_pos;
		m_lines.clear();
		m_tris.clear();
		m_geometries.clear();
	}

	void JoltDebugDraw::EndFrame()
	{
		// Let Jolt release cached debug geometry between frames
		NextFrame();
	}

	JoltDebugDraw::DebugVertex JoltDebugDraw::MakeVertex(JPH::RVec3Arg p, JPH::ColorArg c)
	{
		DebugVertex v;
		v.px = (float)p.GetX();
		v.py = (float)p.GetY();
		v.pz = (float)p.GetZ();
		v.r = c.r;
		v.g = c.g;
		v.b = c.b;
		v.a = c.a;
		return v;
	}

	JoltDebugDraw::DebugVertex JoltDebugDraw::MakeVertex(const JPH::Float3& p, JPH::ColorArg c)
	{
		DebugVertex v;
		v.px = p.x;
		v.py = p.y;
		v.pz = p.z;
		v.r = c.r;
		v.g = c.g;
		v.b = c.b;
		v.a = c.a;
		return v;
	}

	glm::vec4 JoltDebugDraw::ToGlmColor(JPH::ColorArg c)
	{
		return glm::vec4(
			(float)c.r / 255.0f,
			(float)c.g / 255.0f,
			(float)c.b / 255.0f,
			(float)c.a / 255.0f);
	}

	glm::mat4 JoltDebugDraw::ToGlmMat4(JPH::RMat44Arg m)
	{
		auto c0 = m.GetColumn4(0);
		auto c1 = m.GetColumn4(1);
		auto c2 = m.GetColumn4(2);
		auto c3 = m.GetColumn4(3);

		glm::mat4 out(1.0f);
		out[0] = glm::vec4((float)c0.GetX(), (float)c0.GetY(), (float)c0.GetZ(), (float)c0.GetW());
		out[1] = glm::vec4((float)c1.GetX(), (float)c1.GetY(), (float)c1.GetZ(), (float)c1.GetW());
		out[2] = glm::vec4((float)c2.GetX(), (float)c2.GetY(), (float)c2.GetZ(), (float)c2.GetW());
		out[3] = glm::vec4((float)c3.GetX(), (float)c3.GetY(), (float)c3.GetZ(), (float)c3.GetW());
		return out;
	}

	void JoltDebugDraw::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
	{
		std::scoped_lock lock(m_mutex);
		m_lines.emplace_back(MakeVertex(inFrom, inColor));
		m_lines.emplace_back(MakeVertex(inTo, inColor));
	}

	void JoltDebugDraw::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow)
	{
		std::scoped_lock lock(m_mutex);
		m_tris.emplace_back(MakeVertex(inV1, inColor));
		m_tris.emplace_back(MakeVertex(inV2, inColor));
		m_tris.emplace_back(MakeVertex(inV3, inColor));
	}

	JoltDebugDraw::Batch JoltDebugDraw::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
	{
		if (inTriangles == nullptr || inTriangleCount <= 0)
			return Batch();

		auto *raw = new GLTriangleBatch();
		Batch batch(raw); // Batch = Ref<RefTargetVirtual>
		raw->vertices.reserve((size_t)inTriangleCount * 3);
		raw->indices.reserve((size_t)inTriangleCount * 3);

		for (int t = 0; t < inTriangleCount; ++t)
		{
			const Triangle& tri = inTriangles[t];
			const uint32_t base = (uint32_t)raw->vertices.size();

			raw->vertices.push_back(MakeVertex(tri.mV[0].mPosition, tri.mV[0].mColor));
			raw->vertices.push_back(MakeVertex(tri.mV[1].mPosition, tri.mV[1].mColor));
			raw->vertices.push_back(MakeVertex(tri.mV[2].mPosition, tri.mV[2].mColor));

			raw->indices.push_back(base + 0);
			raw->indices.push_back(base + 1);
			raw->indices.push_back(base + 2);
		}

		return batch;
	}

	JoltDebugDraw::Batch JoltDebugDraw::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
	{
		if (inVertices == nullptr || inVertexCount <= 0)
			return Batch();

		auto *raw = new GLTriangleBatch();
		Batch batch(raw);
		raw->vertices.reserve((size_t)inVertexCount);
		if (inIndices != nullptr && inIndexCount > 0)
			raw->indices.reserve((size_t)inIndexCount);

		for (int i = 0; i < inVertexCount; ++i)
		{
			const Vertex& v = inVertices[i];
			raw->vertices.push_back(MakeVertex(v.mPosition, v.mColor));
		}

		if (inIndices != nullptr)
			for (int i = 0; i < inIndexCount; ++i)
				raw->indices.push_back((uint32_t)inIndices[i]);

		return batch;
	}

	void JoltDebugDraw::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor,
		const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow /*inCastShadow*/, EDrawMode inDrawMode)
	{
		// Choose LOD based on camera distance
		glm::vec3 cam_pos;
		{
			std::scoped_lock lock(m_mutex);
			cam_pos = m_camera_pos;
		}
		JPH::Vec3 cam(cam_pos.x, cam_pos.y, cam_pos.z);
		const LOD& lod = inGeometry->GetLOD(cam, inWorldSpaceBounds, inLODScaleSq);
		if (lod.mTriangleBatch == nullptr)
			return;

		std::scoped_lock lock(m_mutex);
		m_geometries.push_back({
			lod.mTriangleBatch,
			ToGlmMat4(inModelMatrix),
			ToGlmColor(inModelColor),
			inCullMode,
			inDrawMode
		});
	}

	void JoltDebugDraw::DrawText3D(JPH::RVec3Arg /*inPosition*/, const JPH::string_view& /*inString*/, JPH::ColorArg /*inColor*/, float /*inHeight*/)
	{
		// Optional: route into your text system. No-op for now.
	}


	void JoltDebugDraw::GLTriangleBatch::EnsureUploaded()
	{
		if (uploaded)
			return;

		if (vao == 0) glCreateVertexArrays(1, &vao);
		if (vbo == 0) glCreateBuffers(1, &vbo);
		if (ebo == 0) glCreateBuffers(1, &ebo);

		glNamedBufferData(vbo, (GLsizeiptr)(vertices.size() * sizeof(DebugVertex)), vertices.data(), GL_STATIC_DRAW);
		glNamedBufferData(ebo, (GLsizeiptr)(indices.size() * sizeof(std::uint32_t)), indices.data(), GL_STATIC_DRAW);

		glVertexArrayElementBuffer(vao, ebo);
		glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(DebugVertex));

		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(vao, 0, 0);

		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribFormat(vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 12);
		glVertexArrayAttribBinding(vao, 1, 0);

		uploaded = true;
	}

	void JoltDebugDraw::GLTriangleBatch::DestroyGL()
	{
		if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
		if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
		if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
		uploaded = false;
	}

	void JoltDebugDraw::UploadDynamic(GLuint vbo, const std::vector<DebugVertex>& verts)
	{
		const GLsizeiptr sz = (GLsizeiptr)(verts.size() * sizeof(DebugVertex));
		glNamedBufferData(vbo, sz, verts.empty() ? nullptr : verts.data(), GL_STREAM_DRAW);
	}

	void JoltDebugDraw::Flush(GLuint program, const glm::mat4& view_proj)
	{
		if (!m_gl_initialized)
			InitializeGL();

		// Uniform locations (queried once per flush; debug-only, OK)
		const GLint locVP = glGetUniformLocation(program, "u_ViewProjection");
		const GLint locM = glGetUniformLocation(program, "u_Model");
		const GLint locMC = glGetUniformLocation(program, "u_ModelColor");

		if (locVP >= 0)
			glProgramUniformMatrix4fv(program, locVP, 1, GL_FALSE, glm::value_ptr(view_proj));

		// Immediate lines / triangles are already in world space
		const glm::mat4 I(1.0f);
		const glm::vec4 white(1.0f);
		if (locM >= 0) glProgramUniformMatrix4fv(program, locM, 1, GL_FALSE, glm::value_ptr(I));
		if (locMC >= 0) glProgramUniform4fv(program, locMC, 1, glm::value_ptr(white));

		std::vector<DebugVertex> lines;
		std::vector<DebugVertex> tris;
		std::vector<GeometryDraw> geometries;
		{
			std::scoped_lock lock(m_mutex);
			lines = m_lines;
			tris = m_tris;
			geometries = m_geometries;
		}

		// Lines
		if (!lines.empty())
		{
			UploadDynamic(m_lineVBO, lines);
			glBindVertexArray(m_lineVAO);
			glLineWidth(1.0f);
			glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());
		}

		// Triangles (wireframe filled controlled by DrawGeometry; immediate triangles default to wireframe if enabled)
		if (!tris.empty())
		{
			UploadDynamic(m_triVBO, tris);
			glBindVertexArray(m_triVAO);
			if (m_settings.wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0f, -1.0f);
			}
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)tris.size());
			if (m_settings.wireframe) {
				glDisable(GL_POLYGON_OFFSET_LINE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// Geometry batches
		for (const GeometryDraw& g : geometries)
		{
			if (g.batch == nullptr)
				continue;

			auto* batch = static_cast<GLTriangleBatch*>(g.batch.GetPtr());
			if (batch == nullptr)
				continue;

			batch->EnsureUploaded();

			// Cull mode
			switch (g.cull_mode)
			{
			case ECullMode::CullBackFace:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case ECullMode::CullFrontFace:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			default:
				glDisable(GL_CULL_FACE);
				break;
			}

			if (locM >= 0)
				glProgramUniformMatrix4fv(program, locM, 1, GL_FALSE, glm::value_ptr(g.model));
			if (locMC >= 0)
				glProgramUniform4fv(program, locMC, 1, glm::value_ptr(g.model_color));

			const bool wf = (g.draw_mode == EDrawMode::Wireframe);
			if (wf) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(-1.0f, -1.0f);
			}

				glBindVertexArray(batch->vao);
				if (!batch->indices.empty())
					glDrawElements(GL_TRIANGLES, (GLsizei)batch->indices.size(), GL_UNSIGNED_INT, nullptr);
				else
					glDrawArrays(GL_TRIANGLES, 0, (GLsizei)batch->vertices.size());

			if (wf) {
				glDisable(GL_POLYGON_OFFSET_LINE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}

		// Restore common state
		glBindVertexArray(0);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

} // namespace Engine
