#include <stdexcept>

#include "MeshData.h"

namespace Engine {

	MeshGL upload_mesh_data(MeshData& mesh) {

		MeshGL mgl;

		const size_t N = mesh.positions.size();
		if (N == 0 || mesh.indices.empty()) throw std::runtime_error("Corrupt mesh, check mesh position and index values!");

		mgl.vbo.create();

		// Calculate how much size we need to allocate for vbo
		// Calculate values for ease of packing
		GLsizei position_data_offset = 0;
		GLsizei position_attribute_size = sizeof(glm::vec3);
		GLsizei position_data_size = position_attribute_size * static_cast<GLsizei>(mesh.positions.size());

		GLsizei normal_data_offset = position_data_size;
		GLsizei normal_attribute_size = sizeof(glm::vec3);
		GLsizei normal_data_size = normal_attribute_size * static_cast<GLsizei>(mesh.normals.size());

		GLsizei color_data_offset = position_data_size + normal_data_size;
		GLsizei color_attribute_size = sizeof(glm::vec3);
		GLsizei color_data_size = color_attribute_size * static_cast<GLsizei>(mesh.colors.size());

		GLsizei texcoords_data_offset = position_data_size + normal_data_size + color_data_size;
		GLsizei texcoords_attribute_size = sizeof(glm::vec2);
		GLsizei texcoords_data_size = texcoords_attribute_size * static_cast<GLsizei>(mesh.texcoords.size());

		GLsizei buffer_size = position_data_size + normal_data_size + color_data_size + texcoords_data_size;

		// Wrapper for named buffer storage
		mgl.vbo.storage(buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

		// Load data into sub buffer		
		mgl.vbo.sub_data(position_data_offset, position_data_size, mesh.positions.data());
		mgl.vbo.sub_data(normal_data_offset, normal_data_size, mesh.normals.data());
		mgl.vbo.sub_data(color_data_offset, color_data_size, mesh.colors.data());
		mgl.vbo.sub_data(texcoords_data_offset, texcoords_data_size, mesh.texcoords.data());

		// Set up the VAO
		mgl.vao.create();

		// Bind the vertex array for the position
		mgl.vao.enable_attrib(0);
		mgl.vao.bind_vertex_buffer(0, mgl.vbo, position_data_offset, position_attribute_size);
		mgl.vao.attrib_format(0, 3, GL_FLOAT, false, 0);
		mgl.vao.attrib_binding(0, 0);

		// Bind the vertex array for the normals
		mgl.vao.enable_attrib(1);
		mgl.vao.bind_vertex_buffer(1, mgl.vbo, normal_data_offset, normal_attribute_size);
		mgl.vao.attrib_format(1, 3, GL_FLOAT, false, 0);
		mgl.vao.attrib_binding(1, 1);

		// Bind the vertex array for the colors
		mgl.vao.enable_attrib(2);
		mgl.vao.bind_vertex_buffer(2, mgl.vbo, color_data_offset, color_attribute_size);
		mgl.vao.attrib_format(2, 3, GL_FLOAT, false, 0);
		mgl.vao.attrib_binding(2, 2);

		// Bind the vertex array for the texcoords
		mgl.vao.enable_attrib(3);
		mgl.vao.bind_vertex_buffer(3, mgl.vbo, texcoords_data_offset, texcoords_attribute_size);
		mgl.vao.attrib_format(3, 2, GL_FLOAT, false, 0);
		mgl.vao.attrib_binding(3, 3);

		// Create an element buffer object to transfer topology
		mgl.ebo.create();
		mgl.ebo.storage(sizeof(uint32_t) * static_cast<GLsizei>(mesh.indices.size()), reinterpret_cast<GLvoid*>(mesh.indices.data()), GL_DYNAMIC_STORAGE_BIT);
		mgl.vao.bind_element_buffer(mgl.ebo);

		mgl.draw_count = static_cast<GLsizei>(mesh.indices.size());
		mgl.primitive_type = GL_TRIANGLES;

		return mgl;
	}
}