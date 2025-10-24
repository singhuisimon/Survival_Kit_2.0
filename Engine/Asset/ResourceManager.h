
#pragma once
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__


#include "ResourceTypes.h"

#include "../include/xresource_mgr.h"
#include "../include/xresource_guid.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

//define acronym for easier access
#define RM Engine::ResourceManager::getInstance()

namespace Engine {

    //forward declarations for resource data structures
    struct TextureResource;
    struct MeshResource;
    struct MaterialResource;
    struct AudioResource;
    struct ShaderResource;

	class ResourceManager {
	private: 
		ResourceManager() = default;
		ResourceManager(ResourceManager const&);
		void operator=(ResourceManager const&) = delete;

		std::unique_ptr<xresource::mgr> m_resource_mgr;


	public:
		static ResourceManager& getInstance();

		int startUp();

		void shutDown();

        /**
         * @brief Load a resource by GUID.
         * @tparam T Resource type (TextureResource, MeshResource, etc.)
         * @param guid The GUID of the resource to load.
         * @return Pointer to the loaded resource, or nullptr if loading failed.
         */
        template<typename T>
        T* loadResource(const xresource::full_guid& guid);

        /**
         * @brief Release a resource reference.
         * @tparam T Resource type.
         * @param guid The GUID of the resource to release.
         */
        template<typename T>
        void releaseResource(xresource::full_guid& guid);

        /**
         * @brief Get the xresource manager (for internal use by loaders).
         * @return Reference to the xresource manager.
         */
        xresource::mgr& getXResourceManager() { return *m_resource_mgr; }

        /**
         * @brief Process end-of-frame cleanup.
         * @details Should be called once per frame to handle delayed resource cleanup.
         */
        void onEndFrame() { m_resource_mgr->OnEndFrameDelegate(); }
	};

    // Template specializations (declared here, defined in .cpp)
    template<>
    TextureResource* ResourceManager::loadResource<TextureResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<TextureResource>(xresource::full_guid& guid);

    template<>
    MeshResource* ResourceManager::loadResource<MeshResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<MeshResource>(xresource::full_guid& guid);

    template<>
    MaterialResource* ResourceManager::loadResource<MaterialResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<MaterialResource>(xresource::full_guid& guid);

    template<>
    AudioResource* ResourceManager::loadResource<AudioResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<AudioResource>(xresource::full_guid& guid);

    template<>
    ShaderResource* ResourceManager::loadResource<ShaderResource>(const xresource::full_guid& guid);

    template<>
    void ResourceManager::releaseResource<ShaderResource>(xresource::full_guid& guid);


}// end of namespace gam300


#endif // __RESOURCE_MANAGER_H__