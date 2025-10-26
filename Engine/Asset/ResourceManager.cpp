
#include "ResourceManager.h"
#include "AssetManager.h"
#include "ResourceData.h"

namespace Engine {

	ResourceManager::ResourceManager() {
		//setType("ResourceManager");
		m_resource_mgr = std::make_unique<xresource::mgr>();
	}

	//get 
	ResourceManager& ResourceManager::getInstance() {
		static ResourceManager instance;
		return instance;
	}

	//startUP
	int ResourceManager::startUp() {

        try {
            // Initialize the xresource manager
            m_resource_mgr->Initiallize(10000); // Support up to 10,000 resources

            // Set this ResourceManager as user data for the xresource manager
            // This allows loaders to access ResourceManager methods
            m_resource_mgr->setUserData(this, false);

            // Set the root path for xresource_mgr (convert to wide string)
            std::string descriptor_root = AM.config().descriptorRoot;
            std::wstring wide_descriptor_path(descriptor_root.begin(), descriptor_root.end());
            m_resource_mgr->setRootPath(std::move(wide_descriptor_path));

            //LM.writeLog("ResourceManager::startUp() - Resource Manager started successfully");
            //LM.writeLog("ResourceManager::startUp() - Descriptor root: %s", descriptor_root.c_str());
            //LM.writeLog("ResourceManager::startUp() - Intermediate root: %s", m_paths->getIntermediateRootPath().c_str());
            //LM.writeLog("ResourceManager::startUp() - Compiled root: %s", m_paths->getCompiledRootPath().c_str());

            return 0;
        }
        catch (const std::exception& e) {
         //   LM.writeLog("ResourceManager::startUp() - Exception: %s", e.what());
            return -1;
        }

	}

	void ResourceManager::shutDown() {

       // LM.writeLog("ResourceManager::shutDown() - ResourceManager shutting down");

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