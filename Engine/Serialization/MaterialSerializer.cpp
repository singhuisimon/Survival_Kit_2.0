#include  "../Serialization/MaterialSerializer.h"
#include "../Serialization/SerializationCommon.h"
#include "../Utility/Types.h"

// RapidJSON includes
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include <fstream> // For file output

#include "Utility/AssetPath.h"
#include "Utility/Logger.h"

namespace Engine {

	void serializeMaterial(MaterialResource* mat, std::string filename) {

		using namespace rapidjson;

		Document doc;
		doc.SetObject();
		auto& allocator = doc.GetAllocator();

		doc.AddMember("shaderName", Value(mat->shaderName.c_str(), allocator), allocator);

		doc.AddMember("diffuseMap", static_cast<uint64_t>(mat->diffuseMap.m_Value), allocator);
		doc.AddMember("normalMap", static_cast<uint64_t>(mat->normalMap.m_Value), allocator);
		doc.AddMember("specularMap", static_cast<uint64_t>(mat->specularMap.m_Value), allocator);
		doc.AddMember("emissionMap", static_cast<uint64_t>(mat->emissionMap.m_Value), allocator);
		doc.AddMember("occlusionMap", static_cast<uint64_t>(mat->occlusionMap.m_Value), allocator);

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
        std::string filepath = getAssetsPath() + "Sources/Material/" + filename;
        
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
 