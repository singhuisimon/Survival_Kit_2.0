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

		doc.AddMember("shaderName", Value(mat->shaderName.c_str(), allocator), allocator);

        // diffuseMap (Formatted as hex string)
        std::string diffuseMapHex = std::format("{:x}", mat->diffuseMap.m_Value);
        doc.AddMember("diffuseMap", rapidjson::Value(diffuseMapHex.c_str(), allocator), allocator);

        // normalMap (Formatted as hex string)
        std::string normalMapHex = std::format("{:x}", mat->normalMap.m_Value);
        doc.AddMember("normalMap", rapidjson::Value(normalMapHex.c_str(), allocator), allocator);

        // specularMap (Formatted as hex string)
        std::string specularMapHex = std::format("{:x}", mat->specularMap.m_Value);
        doc.AddMember("specularMap", rapidjson::Value(specularMapHex.c_str(), allocator), allocator);

        // emissionMap (Formatted as hex string)
        std::string emissionMapHex = std::format("{:x}", mat->emissionMap.m_Value);
        doc.AddMember("emissionMap", rapidjson::Value(emissionMapHex.c_str(), allocator), allocator);

        // occlusionMap (Formatted as hex string)
        std::string occlusionMapHex = std::format("{:x}", mat->occlusionMap.m_Value);
        doc.AddMember("occlusionMap", rapidjson::Value(occlusionMapHex.c_str(), allocator), allocator);

        Value diffuseColorArray(kArrayType);
        for (float val : mat->diffuseColor) {
            diffuseColorArray.PushBack(val, allocator);
        }
        doc.AddMember("diffuseColor", diffuseColorArray, allocator);

        Value specularColorArray(kArrayType);
        for (float val : mat->specularColor) {
            specularColorArray.PushBack(val, allocator);
        }
        doc.AddMember("specularColor", specularColorArray, allocator);

        Value emissionColorArray(kArrayType);
        for (float val : mat->emissionColor) {
            emissionColorArray.PushBack(val, allocator);
        }
        doc.AddMember("emissionColor", emissionColorArray, allocator);
        doc.AddMember("shininess", mat->shininess, allocator);
        doc.AddMember("emissionStrength", mat->emissionStrength, allocator);
        doc.AddMember("alphaThreshold", mat->alphaThreshold, allocator);

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
        std::string filepath = AssetManager::GetSourceResourcesPath() + "/Sources/Material/" + filename;
        
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
 