#ifndef __ASSET_DESCGENERATOR_H__
#define __ASSET_DESCGENERATOR_H__


#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include "AssetDatabase.h"
#include "../Utility/AssetPath.h"
#include "../Utility/Logger.h"



	// Forward declaration to avoid coupling. Defined in AssetDatabase.h
	struct AssetRecord;
	//for basic audio compilation settings 

    struct TextureSettings{

		std::string usageType;
		std::string compression;
		float quality = 1.0f;
		bool generateMipmaps = false;
		bool srgb = false;
    };


	struct AudioSettings {

		//output format 
		std::string outputFormat = "OGG"; //the target format could be OGG, WAV
		std::string compression = "VORBIS"; //VORBIS, PCM

		//quality
		float quality = 0.7f;	//compression quality 0.0-1.0
		int sampleRate = 44100; //output sample rate 
		//channels
		std::string channelMode = "STEREO"; //MONO or STEREO
	};

	//for basic mesh compilation settings
	struct MeshSettings {

		//output format
		std::string outputFormat = "CUSTOM"; //custom binary format for the engine

		//vertex Data 
		bool includePos = true; //need position
		bool includeNormals = true; //for lighting
		bool includeColors = false; //vertex colors
		bool includeTexCoords = true; //for texturing

		//index format
		std::string indexType = "UINT32"; //UINT16 or UINT32

		//transform 
		float scale = 1.0f; //uniform scale

		//optimizations 
		bool optimizeVertices = true; //remove duplicates and optimize cache
		bool generateNormals = false; //generate if missing
	};

	//basic shader compilation settings
	struct ShaderSettings {
		//input files
		std::string vertexShader = ""; //Path to .vert source
		std::string fragmentShader = ""; //Path to .frag source

		//output format 
		std::string outputFormat = "GLSL"; //SPIRV or GLSL

		//target platform is OPENGL
		std::string targetAPI = "OPENGL";
		std::string targetVersion = "460"; //what is thisisssssss

		//optimization 
		std::string optimizationLevel = "PERFORMANCE"; //none, size, performance
		bool stripDebugInfo = true; //remove debug symbols 
	};

	/**
	* @brief To display for the Info.txt
	* @details 
	*/
	struct DescriptorExtras
	{
		std::string displayName; //human readable file name
		std::string category;
		std::string comment; //optimal description
		std::vector<std::string> tags; //user-defined lables for organization
		std::time_t lastImported = 0; //last-imported time
        std::vector<xresource::full_guid> resourceLinks;
	};

	// /**
	// * @brief To display for the Descriptor.txt
	// * @details contains the compile settings and the relative source paths
	// */
    // struct WriteCompilerSettings{

    //     //contain the relative path to the actual file
    //     std::string sourcePath; 

    //     //new texture settings (for compiler)
    //     TextrueSettings textureSettings;
	// 	//new audio settings (for compile settings)
	// 	AudioSettings audioSettings;
	// 	//new mesh settings (for compile settings)
	// 	MeshSettings meshSettings;
	// 	//new shader settings (for compiler)
	// 	ShaderSettings shaderSettings;
    // };

	/**
	* @class AssetDescriptorGenerator
	* @brief Emits .desc files with metadata for editor/importer use.
	*/
	class AssetDescriptorGenerator
	{
	public:

		/** @brief Set centralized output root */
		void SetOutputRoot(const std::string& root);

/**
		 * @brief Generate Info.txt and Descriptor.txt for an asset
		 * @tparam SettingsType Compile settings type (TextureSettings, ShaderSettings, etc.)
		 * @param rec Asset record with GUID and metadata
		 * @param extras Optional display metadata (name, tags, etc.)
		 * @param settings Compilation settings for this asset type
		 * @param outPath Optional output folder path
		 * @return True on success
		 */
        template<typename SettingType>
        bool GenerateFor(const AssetRecord& rec, 
                        const DescriptorExtras* extras = nullptr,
                        const SettingType& settings = nullptr,
                        std::string* outPath = nullptr) const{
                            //get folder path for this GUID
                            std::string folderPath = GetDescriptorFolderPath(rec);

                            //ensure directory exists
                            if(!EnsureDirectory(folderPath)) return false;

                            //write Info.txt (metadata)
                            if(!WriteInfoFile(folderPath, rec, extras)) return false;


                            //write descriptor.txt (compile settings)
                            if(!WriteDescriptorFile(folderPath, rec.sourcePath, settings)) return false;

                            //return folder path if requested
                            if(outPath) {
                                *outPath = folderPath;
                            }

                            return true;
                        }

/**
     * @brief Get the folder path for a GUID's descriptors
     * Example: "Assets/Descriptors/Shader/03/40/5F7ED05B14224003/"
     */
    std::string GetDescriptorFolderPath(const AssetRecord& rec) const;

	private:

    // ==================== INFO.TXT GENERATION ====================
    // Generate Info.txt (metadata)
        bool WriteInfoFile(const std::string& folderPath,
                        const AssetRecord& rec,
                        const DescriptorExtras* extras) const;
    // Build JSON for Info.txt
        std::string BuildInfoJson(const AssetRecord& rec,
                                const DescriptorExtras* extras) const;

    // ==================== DESCRIPTOR.TXT GENERATION ====================

		template<typename SettingsType>
		bool WriteDescriptorFile(const std::string& folderPath,
			const std::string& sourcePath,
			const SettingsType& settings) const
		{
			std::string descriptorPath = folderPath + "Descriptor.txt";
			std::string json = BuildDescriptorJson(sourcePath, settings);
			//std::string json = BuildDescriptorJson(sourcePath, settings);

			bool result = WriteText(descriptorPath, json);
			if (result) {
				LOG_DEBUG("Descriptor.txt written successfully");
			}
			else {
				LOG_ERROR("Failed to write Descriptor.txt");
			}

			return result;
		}

		// Overloaded JSON builders for each settings type
		std::string BuildDescriptorJson(const std::string& sourcePath,
			const TextureSettings& settings) const;

		std::string BuildDescriptorJson(const std::string& sourcePath,
			const ShaderSettings& settings) const;

		std::string BuildDescriptorJson(const std::string& sourcePath,
			const MeshSettings& settings) const;

		std::string BuildDescriptorJson(const std::string& sourcePath,
			const AudioSettings& settings) const;        
			
		// ==================== HELPERS ====================

		static std::string EscapeJson(const std::string& s);
		static bool EnsureDirectory(const std::string& path);
		static bool WriteText(const std::string& path, const std::string& text);

        //for output root of the descriptor files
		std::string m_outputRoot; 
		
	};


#endif // __ASSET_DESCGENERATOR_H__