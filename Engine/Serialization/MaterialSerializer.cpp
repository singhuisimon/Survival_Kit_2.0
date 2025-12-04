#include  "../Serialization/MaterialSerializer.h"
#include "../Serialization/SerializationCommon.h"
#include "../Utility/Types.h"

// RapidJSON includes
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <fstream> // For file output
#include <format>  // For std::setw

#include "Asset/AssetManager.h"
#include "Utility/AssetPath.h"
#include "Utility/Logger.h"

namespace Engine {

	void serializeMaterial(MaterialResource* mat, std::string filename) {

		using namespace rapidjson;

		Document doc;
		doc.SetObject();
		auto& allocator = doc.GetAllocator();

        // --- ShaderName ---
        doc.AddMember("shaderName", Value(mat->shaderName.c_str(), allocator), allocator);

        // --- Texture Maps ---
        // Store texture maps as filenames resolved from GUIDs on disk and resolve them back into GUIDS when loading
        std::string baseMap_filename = AM.getNameFromGuid(mat->baseMap);
        doc.AddMember("baseMap", rapidjson::Value(baseMap_filename.c_str(), allocator), allocator);

        std::string normalMap_filename = AM.getNameFromGuid(mat->normalMap);
        doc.AddMember("normalMap", rapidjson::Value(normalMap_filename.c_str(), allocator), allocator);

        std::string metallicMap_filename = AM.getNameFromGuid(mat->metallicMap);
        doc.AddMember("metallicMap", rapidjson::Value(metallicMap_filename.c_str(), allocator), allocator);

        std::string roughnessMap_filename = AM.getNameFromGuid(mat->roughnessMap);
        doc.AddMember("roughnessMap", rapidjson::Value(roughnessMap_filename.c_str(), allocator), allocator);

        std::string emissionMap_filename = AM.getNameFromGuid(mat->emissionMap);
        doc.AddMember("emissionMap", rapidjson::Value(emissionMap_filename.c_str(), allocator), allocator);

        std::string occlusionMap_filename = AM.getNameFromGuid(mat->occlusionMap);
        doc.AddMember("occlusionMap", rapidjson::Value(occlusionMap_filename.c_str(), allocator), allocator);


        // --- Color Properties (Arrays) ---
        Value baseColorArray(kArrayType);
        for (float val : mat->baseColor) {
            baseColorArray.PushBack(val, allocator);
        }
        doc.AddMember("baseColor", baseColorArray, allocator);

        Value emissionColorArray(kArrayType);
        for (float val : mat->emissionColor) {
            emissionColorArray.PushBack(val, allocator);
        }
        doc.AddMember("emissionColor", emissionColorArray, allocator);

        // --- Float Properties ---
        doc.AddMember("metallic", mat->metallic, allocator);
        doc.AddMember("roughness", mat->roughness, allocator);
        doc.AddMember("opacity", mat->opacity, allocator);
        doc.AddMember("emissionStrength", mat->emissionStrength, allocator);
        doc.AddMember("alphaThreshold", mat->alphaThreshold, allocator);
        doc.AddMember("ambientOcclusion", mat->ambientOcclusion, allocator);

        // --- UV Transforms (Arrays) ---
        Value tilingArray(kArrayType);
        for (float val : mat->tiling) {
            tilingArray.PushBack(val, allocator);
        }
        doc.AddMember("tiling", tilingArray, allocator);

        Value offsetArray(kArrayType);
        for (float val : mat->offset) {
            offsetArray.PushBack(val, allocator);
        }
        doc.AddMember("offset", offsetArray, allocator);

        // --- Bools / Render Flags ---
        doc.AddMember("enableEmission", mat->enableEmission, allocator);
        doc.AddMember("alphaTest", mat->alphaTest, allocator);
        doc.AddMember("doubleSided", mat->doubleSided, allocator);
        doc.AddMember("receiveShadows", mat->receiveShadows, allocator);
        doc.AddMember("castShadows", mat->castShadows, allocator);

        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer); // PrettyWriter for readable formatting
        doc.Accept(writer);

		// append .mat extension
        filename += ".mat";

		// Write the full filepath
        std::string filepath = Engine::getAssetsPath() + "/Sources/Material/" + filename;
        
        std::ofstream ofs(filepath);
        if (ofs.is_open()) {
            ofs << buffer.GetString();
            ofs.close();
        }
        else {
            LOG_ERROR("Fail to save material: ", filepath);
        }
	}

}
 