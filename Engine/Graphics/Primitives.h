#pragma once

#include "../Graphics/MeshData.h"

namespace Engine {

	/**
	 * @brief Generates a unit cube centered at the origin
	 * @return MeshData containing vertices, normals, and UVs for a cube
	 */
	MeshData make_cube();

	/**
	 * @brief Generates a flat plane on the XZ plane
	 * @return MeshData containing vertices, normals, and UVs for a plane
	 */
	MeshData make_plane();

	/**
	 * @brief Generates a UV sphere centered at the origin
	 * @return MeshData containing vertices, normals, and UVs for a sphere
	 */
	MeshData make_sphere();

}
