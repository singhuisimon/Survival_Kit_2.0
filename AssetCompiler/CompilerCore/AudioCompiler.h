/*
* @file AudioCompiler.h
* @brief Audio resource compiler for AssetCompiler
* @details Compiles WAV/OGG/MP3 audio to optimized binary format for runtime loading
* @author 
* @date October 2025
*/

#pragma once

#include <string>
#include <vector>

namespace AssetCompiler {

	/**
	 * @brief Raw audio data loaded from source files
	 */
	struct AudioData {
		std::vector<unsigned char> samples;  // Raw audio samples
		int sampleRate = 44100;               // Samples per second
		int channels = 2;                     // 1=mono, 2=stereo
		int bitDepth = 16;                    // Bits per sample (8, 16, 24, 32)
		int sampleCount = 0;                  // Total number of samples

		bool isEmpty() const {
			return samples.empty();
		}

		size_t getDataSize() const {
			return samples.size();
		}

		float getDuration() const {
			if (sampleRate == 0 || channels == 0) return 0.0f;
			return static_cast<float>(sampleCount) / static_cast<float>(sampleRate);
		}
	};

	/**
	 * @brief Audio Settings for compilation (read from Descriptor.txt)
	 */
	struct AudioSettingsCompiler {
		// Output format
		std::string outputFormat = "PCM";  // PCM, VORBIS, etc.

		// Quality settings
		float quality = 0.7f;              // Compression quality 0.0-1.0 (for compressed formats)
		int targetSampleRate = 0;          // 0 = keep original, otherwise resample
		std::string channelMode = "AUTO";  // AUTO, MONO, STEREO

		// Processing flags
		bool normalize = false;            // Normalize audio levels
		float volume = 1.0f;               // Volume multiplier
		bool convertToMono = false;        // Force mono conversion
	};

	/**
	 * @brief Binary audio file header
	 */
	struct CompiledAudioHeader {
		char magic[4] = { 'A', 'U', 'D', '\0' };  // Magic number "AUD"
		uint32_t version = 1;                      // Format version

		uint32_t sampleRate = 44100;               // Samples per second
		uint32_t channels = 2;                     // Number of channels (1=mono, 2=stereo)
		uint32_t bitDepth = 16;                    // Bits per sample (8, 16, 24, 32)
		uint32_t sampleCount = 0;                  // Total number of samples

		uint32_t format = 0;                       // Audio format (0=PCM)
		uint32_t compressed = 0;                   // 1 if compressed, 0 if PCM

		uint32_t reserved[6] = { 0 };              // For future use
	};

	class AudioCompiler {
	public:
		AudioCompiler() = default;
		~AudioCompiler() = default;

		/**
		* @brief Compile an audio file from descriptor
		* @param descriptorPath path to Descriptor.txt folder
		* @param outputPath path to write compiled GUID.audio file
		* @param verbose Enable verbose logging
		* @return true if compilation succeeded
		*/
		bool compile(const std::string& descriptorPath,
			const std::string& outputPath,
			bool verbose = false);

	private:
		// === Loading ===
		bool loadWAV(const std::string& path, AudioData& audioData);
		bool loadOGG(const std::string& path, AudioData& audioData);

		// === Processing ===
		void resampleAudio(AudioData& audioData, int targetSampleRate);
		void convertToMono(AudioData& audioData);
		void normalizeAudio(AudioData& audioData);
		void adjustVolume(AudioData& audioData, float volume);

		// === Serialization ===
		bool writeBinaryAudio(const std::string& outputPath,
			const CompiledAudioHeader& header,
			const AudioData& audioData);

		// === Helpers ===
		bool parseSettings(const std::string& descriptorPath,
			std::string& sourcePath,
			AudioSettingsCompiler& settings);

		std::string fixPathSeparators(const std::string& path);

		bool verbose_ = false;

		void log(const char* format, ...);
	};


}//end of namespace AssetCompiler