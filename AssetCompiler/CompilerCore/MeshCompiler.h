/*
* @file MeshCompiler.h
* @brief Mesh resource compiler for AssetCompiler
* @details Compiles FBX/OBJ meshes to optimized binary format for runtime loading 
* @author
* @date 
*/

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace AssetCompiler {

	struct MeshData {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec2> texCoords;
		std::vector<uint32_t> indices;

		bool isEmpty() const {
			return positions.empty();
		}

		size_t getVertexCount() const {
			return positions.size();
		}

		size_t getIndexCount() const {
			return indices.size();
		}

		size_t getTriangleCount() const {
			return indices.size() / 3;
		}
	};

	/*
	* @brief Mesh Settings for compilation (read from Descriptor.txt)
	*/
	struct MeshSettingsCompiler {
		//output format 
		std::string outputFormat; 

		//vertex data to include
		bool includePos = true;
		bool includeNormals = true; 
		bool includeColors = false;
		bool includeTexCoords = true;

		//index format
		std::string indexType = "UINT32"; 

		//transform 
		float scale = 1.0f;

		//optimization flags
		bool optimizeVertices = true;
		bool generateNormals = false;
		bool flipUVs = false;
		bool removeDegenerate = false;
		bool weldVertices = false;
		float weldThreshold = 0.00001f; 
	};

	/**
	 * @brief Binary mesh file header
	 */
	struct CompiledMeshHeader {
		char magic[4] = { 'M', 'S', 'H', '\0' };  // Magic number "MSH"
		uint32_t version = 1;                    // Format version

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		uint32_t hasPositions = 0;    // Always 1
		uint32_t hasNormals = 0;      // 1 if normals present
		uint32_t hasColors = 0;       // 1 if colors present
		uint32_t hasTexCoords = 0;    // 1 if UVs present

		uint32_t vertexStride = 0;    // Bytes per vertex
		uint32_t indexSize = 4;       // 2 for UINT16, 4 for UINT32

		uint32_t reserved[6] = { 0 };   // For future use
	};

	class MeshCompiler {
	public: 
		MeshCompiler() = default; 
		~MeshCompiler() = default;

		/**
		* @brief Compile a mesh from descriptor 
		* @param descriptorPath path to Descriptor folder
		* @param outputPath path to write compiled GUID.mesh file
		* @param verbose Enable verbose logging
		* @return true if compilation succeeded
		*/
		bool compile(const std::string& descriptorPath,
			const std::string& outputPath,
			bool verbose = false);

	private: 
		// === Loading ===
		bool loadFBXMesh(const std::string& path, MeshData& meshData);
		bool loadOBJMesh(const std::string& path, MeshData& meshData);

		// === Processing ===
		void scaleMesh(MeshData& meshData, float scale);
		void generateNormals(MeshData& meshData);
		void flipUVs(MeshData& meshData);
		void removeDegenerate(MeshData& meshData);
		void weldVertices(MeshData& meshData, float threshold);
		void optimizeVertexCache(MeshData& meshData);

		// === Serialization ===
		bool writeBinaryMesh(const std::string& outputPath,
			const CompiledMeshHeader& header,
			const MeshData& meshData);

		// === Helpers ===
		bool parseSettings(const std::string& descriptorPath,
			std::string& sourcePath,
			MeshSettingsCompiler& settings);

		std::string fixPathSeparators(const std::string& path);

		bool verbose_ = false;

		void log(const char* format, ...);


	};

}// end of namespace AssetCompiler