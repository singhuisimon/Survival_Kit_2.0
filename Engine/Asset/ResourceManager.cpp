/**
 * @file ResourceManager.cpp
 * @brief Runtime resource loading and lifecycle management
 * @details
 * Manages loading, caching, and reference counting of compiled runtime resources.
 * Provides type-safe access to textures, meshes, materials
 * through the xresource::mgr system. Resources are loaded on-demand from compiled
 * binary files and automatically unloaded when no longer referenced.
 * @author Wai Lwin Thit
 * @date September 15 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */



#include "ResourceManager.h"
#include "AssetManager.h"
#include "ResourceData.h"


namespace Engine {

	//get 
	ResourceManager& ResourceManager::getInstance() {
		static ResourceManager instance;
		return instance;
	}

	//startUP
	int ResourceManager::startUp() {


        try {

            m_resource_mgr = std::make_unique<xresource::mgr>();


            // Initialize the xresource manager
            m_resource_mgr->Initiallize(10000); // Support up to 10,000 resources

            // Set this ResourceManager as user data for the xresource manager
            // This allows loaders to access ResourceManager methods
            m_resource_mgr->setUserData(this, false);

            // Set the root path for xresource_mgr (convert to wide string)
            std::string descriptor_root = AM.getDescriptorRoot();
            std::wstring wide_descriptor_path(descriptor_root.begin(), descriptor_root.end());
            m_resource_mgr->setRootPath(std::move(wide_descriptor_path));

      

            return 0;
        }
        catch (const std::exception& e) {
            LOG_DEBUG("Resource Manager failed to Initialize: ", e.what());
            return -1;
        }

	}

	void ResourceManager::shutDown() {


		m_resource_mgr.reset();

	}


    //texture load and release 

    //template for loadResource
    template<>
    TextureResource* ResourceManager::loadResource<TextureResource>(const xresource::full_guid& guid) {

        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<TextureResource*>(resource);

    }

    //release texture resource
    template<>
    void ResourceManager::releaseResource<TextureResource>(xresource::full_guid& guid) {

        m_resource_mgr->ReleaseRef(guid);

    }

    //mesh load 
    template<>
    MeshResource* ResourceManager::loadResource<MeshResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        return static_cast<MeshResource*>(resource);
    }

    //mesh release 
    template<>
    void ResourceManager::releaseResource<MeshResource>(xresource::full_guid& guid) {

        m_resource_mgr->ReleaseRef(guid);
    }


    //Material Load
    template<>
    MaterialResource* ResourceManager::loadResource<MaterialResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<MaterialResource*>(resource);
    }

    //Material Release
    template<>
    void ResourceManager::releaseResource<MaterialResource>(xresource::full_guid& guid) {
        m_resource_mgr->ReleaseRef(guid);
    }

    //load audio
    template<>
    AudioResource* ResourceManager::loadResource<AudioResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<AudioResource*>(resource);
    }

    //release audio
    template<>
    void ResourceManager::releaseResource<AudioResource>(xresource::full_guid& guid) {
        m_resource_mgr->ReleaseRef(guid);
    }

    //shader load
    template<>
    ShaderResource* ResourceManager::loadResource<ShaderResource>(const xresource::full_guid& guid) {
        //make a copy
        xresource::full_guid copy = guid;

        //use universal getResource method
        void* resource = m_resource_mgr->getResource(copy);

        //return the expected type resource
        return static_cast<ShaderResource*>(resource);
    }

    //shader release
    template<>
    void ResourceManager::releaseResource<ShaderResource>(xresource::full_guid& guid) {

        m_resource_mgr->ReleaseRef(guid);
    }
     
}// end of namespace gam300