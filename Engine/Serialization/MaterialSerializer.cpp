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

        // --- Texture Maps (Formatted as hex strings) ---
        std::string baseMapHex = std::format("{:x}", mat->baseMap.m_Value);
        doc.AddMember("baseMap", rapidjson::Value(baseMapHex.c_str(), allocator), allocator);

        std::string normalMapHex = std::format("{:x}", mat->normalMap.m_Value);
        doc.AddMember("normalMap", rapidjson::Value(normalMapHex.c_str(), allocator), allocator);

        std::string metallicMapHex = std::format("{:x}", mat->metallicMap.m_Value);
        doc.AddMember("metallicMap", rapidjson::Value(metallicMapHex.c_str(), allocator), allocator);

        std::string roughnessMapHex = std::format("{:x}", mat->roughnessMap.m_Value);
        doc.AddMember("roughnessMap", rapidjson::Value(roughnessMapHex.c_str(), allocator), allocator);

        std::string emissionMapHex = std::format("{:x}", mat->emissionMap.m_Value);
        doc.AddMember("emissionMap", rapidjson::Value(emissionMapHex.c_str(), allocator), allocator);

        std::string occlusionMapHex = std::format("{:x}", mat->occlusionMap.m_Value);
        doc.AddMember("occlusionMap", rapidjson::Value(occlusionMapHex.c_str(), allocator), allocator);

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
        std::string filepath = Engine::getAssetsPath() + "Material/" + filename;
        
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
 