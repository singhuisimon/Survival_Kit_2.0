/**
 * @file Main.cpp
 * @brief Asset Compiler - Command-line tool for compiling game assets
 * @details Reads descriptor files and compiles source assets into optimized runtime formats
 * @author 
 * @date 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <chrono>

#include <thread>
#include <algorithm>

//include heaader files
#include "../Utility/DescriptorParser.h"
#include "../CompilerCore/MeshCompiler.h"


namespace fs = std::filesystem;

// ============================================================================
// COMMAND LINE ARGUMENTS
// ============================================================================

struct CompilerConfig {
    std::string descriptorsPath = "Resources/Descriptors/";
    std::string outputPath = "Resources/Compiled/";
    std::string resourceType = "all";  // "all", "texture", "mesh", "audio", "shader"
    bool verbose = false;
    bool force = false;  // Force recompile even if up-to-date
    int threadCount = 4;
};

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void printUsage() {
    std::cout << "\n===========================================\n";
    std::cout << "  Asset Compiler v1.0\n";
    std::cout << "===========================================\n\n";
    
    std::cout << "Usage: AssetCompiler [options]\n\n";
    
    std::cout << "Options:\n";
    std::cout << "  --input <path>      Path to Descriptors folder (default: Resources/Descriptors/)\n";
    std::cout << "  --output <path>     Path to output compiled assets (default: Resources/Compiled/)\n";
    std::cout << "  --type <type>       Asset type to compile: all, texture, mesh, audio, shader (default: all)\n";
    std::cout << "  --threads <n>       Number of worker threads (default: 4)\n";
    std::cout << "  --force             Force recompile all assets\n";
    std::cout << "  --verbose           Enable verbose logging\n";
    std::cout << "  --help              Show this help message\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  AssetCompiler\n";
    std::cout << "  AssetCompiler --type texture --verbose\n";
    std::cout << "  AssetCompiler --input Assets/Descriptors --output Build/Compiled\n";
    std::cout << "  AssetCompiler --force --threads 8\n\n";
}

bool parseArguments(int argc, char* argv[], CompilerConfig& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return false;
        }
        else if (arg == "--input" && i + 1 < argc) {
            config.descriptorsPath = argv[++i];
        }
        else if (arg == "--output" && i + 1 < argc) {
            config.outputPath = argv[++i];
        }
        else if (arg == "--type" && i + 1 < argc) {
            config.resourceType = argv[++i];
        }
        else if (arg == "--threads" && i + 1 < argc) {
            config.threadCount = std::stoi(argv[++i]);
        }
        else if (arg == "--force" || arg == "-f") {
            config.force = true;
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else {
            std::cerr << "ERROR: Unknown argument: " << arg << "\n";
            printUsage();
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// DESCRIPTOR DISCOVERY
// ============================================================================

struct DescriptorInfo {
    std::string guidFolder;        // e.g., "Descriptors/Texture/03/40/5F7ED05B14224003/"
    std::string infoFile;          // Full path to Info.txt
    std::string descriptorFile;    // Full path to Descriptor.txt
    std::string resourceType;      // "Texture", "Mesh", etc.
    std::string guid;              // "5F7ED05B14224003"
};

std::vector<DescriptorInfo> discoverDescriptors(const std::string& descriptorsRoot, 
                                                  const std::string& typeFilter) {
    std::vector<DescriptorInfo> descriptors;
    
    if (!fs::exists(descriptorsRoot)) {
        std::cerr << "ERROR: Descriptors path does not exist: " << descriptorsRoot << "\n";
        return descriptors;
    }
    
    std::cout << "Scanning descriptors in: " << descriptorsRoot << "\n";
    
    // Iterate through type folders (Texture, Mesh, Audio, Shader)
    for (const auto& typeEntry : fs::directory_iterator(descriptorsRoot)) {
        if (!typeEntry.is_directory()) continue;
        
        std::string typeName = typeEntry.path().filename().string();
        
        // Filter by type if specified
        if (typeFilter != "all") {
            std::string lowerType = typeName;
            std::string lowerFilter = typeFilter;
            std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(),
                [](unsigned char c) { return std::tolower(c); });
            std::transform(lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower);
            
            if (lowerType != lowerFilter) continue;
        }
        
        // Recursively find all GUID folders (they end with a 16-char hex name)
        for (auto it = fs::recursive_directory_iterator(typeEntry.path()); 
             it != fs::recursive_directory_iterator(); ++it) {
            
            if (!it->is_directory()) continue;
            
            std::string folderName = it->path().filename().string();
            
            // Check if this looks like a GUID folder (16 hex characters)
            if (folderName.length() == 16) {
                bool isHex = std::all_of(folderName.begin(), folderName.end(), ::isxdigit);
                
                if (isHex) {
                    fs::path infoPath = it->path() / "Info.txt";
                    fs::path descPath = it->path() / "Descriptor.txt";
                    
                    if (fs::exists(infoPath) && fs::exists(descPath)) {
                        DescriptorInfo info;
                        info.guidFolder = it->path().string();
                        info.infoFile = infoPath.string();
                        info.descriptorFile = descPath.string();
                        info.resourceType = typeName;
                        info.guid = folderName;
                        
                        descriptors.push_back(info);
                    }
                }
            }
        }
    }
    
    return descriptors;
}

// ============================================================================
// COMPILATION (PLACEHOLDER - TO BE IMPLEMENTED)
// ============================================================================

bool compileAsset(const DescriptorInfo& descriptor, const CompilerConfig& config) {
    // TODO: Implement actual compilation
    // 1. Parse Descriptor.txt to get source path and settings
    // 2. Load source asset
    // 3. Process/compile based on type and settings
    // 4. Write compiled output
    
    // Step 1: Parse Descriptor.txt to get source path
    std::string sourcePath = AssetCompiler::extractSourcePath(descriptor.descriptorFile);
    if (sourcePath.empty()) {
        std::cerr << "ERROR: Could not extract source path from descriptor: "
            << descriptor.descriptorFile << "\n";
        return false;
    }

    // Fix path separators (handle \\ vs /)
    std::replace(sourcePath.begin(), sourcePath.end(), '\\', '/');
    if (!sourcePath.empty() && sourcePath[0] == '/') {
        sourcePath = sourcePath.substr(1);  // Remove leading slash
    }
    
    // Verbose output
    if (config.verbose) {
        std::cout << "  [" << descriptor.resourceType << "] "
            << descriptor.guid << "\n";
        std::cout << "    Source: " << sourcePath << "\n";

        // Check if source file exists
        if (fs::exists(sourcePath)) {
            std::cout << "    Status: Source file exists\n";
        }
        else {
            std::cout << "    Status: Source file NOT FOUND!\n";
        }
    }

    // Step 2: Verify source file exists
    if (!fs::exists(sourcePath)) {
        std::cerr << "ERROR: Source file not found: " << sourcePath << "\n";
        return false;
    }

    // Step 3: Compile based on asset type
    bool success = false;

    if (descriptor.resourceType == "Mesh") {
        // Use MeshCompiler
        AssetCompiler::MeshCompiler meshCompiler;

        // Generate output path: Resources/Compiled/Mesh/GUID.mesh
        std::string outputPath = config.outputPath + descriptor.resourceType + "/" + descriptor.guid + ".mesh";

        success = meshCompiler.compile(descriptor.descriptorFile, outputPath, config.verbose);
    }

    else if (descriptor.resourceType == "Texture") {
        // TODO: Implement TextureCompiler
        if (config.verbose) {
            std::cout << "    TODO: Texture compilation not yet implemented\n";
        }
        // Simulate work for now
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        success = true;
    }
    else if (descriptor.resourceType == "Audio") {
        // TODO: Implement AudioCompiler
        if (config.verbose) {
            std::cout << "    TODO: Audio compilation not yet implemented\n";
        }
        // Simulate work for now
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        success = true;
    }
    else if (descriptor.resourceType == "Shader") {
        // TODO: Implement ShaderCompiler
        if (config.verbose) {
            std::cout << "    TODO: Shader compilation not yet implemented\n";
        }
        // Simulate work for now
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        success = true;
    }
    else {
        std::cerr << "ERROR: Unknown resource type: " << descriptor.resourceType << "\n";
        return false;
    }

    return success; 
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "\n===========================================\n";
    std::cout << "  Asset Compiler v1.0\n";
    std::cout << "===========================================\n\n";
    
    // Parse command line arguments
    CompilerConfig config;
    if (!parseArguments(argc, argv, config)) {
        return 1;
    }
    
    // Print configuration
    std::cout << "Configuration:\n";
    std::cout << "  Input:   " << config.descriptorsPath << "\n";
    std::cout << "  Output:  " << config.outputPath << "\n";
    std::cout << "  Type:    " << config.resourceType << "\n";
    std::cout << "  Threads: " << config.threadCount << "\n";
    std::cout << "  Force:   " << (config.force ? "Yes" : "No") << "\n";
    std::cout << "  Verbose: " << (config.verbose ? "Yes" : "No") << "\n\n";
    
    // Discover all descriptors
    auto descriptors = discoverDescriptors(config.descriptorsPath, config.resourceType);
    
    if (descriptors.empty()) {
        std::cout << "No descriptors found to compile.\n";
        return 0;
    }
    
    std::cout << "Found " << descriptors.size() << " asset(s) to compile.\n\n";
    
    // Create output directory
    if (!fs::exists(config.outputPath)) {
        fs::create_directories(config.outputPath);
    }
    
    // Compile each asset
    int successCount = 0;
    int failCount = 0;
    
    std::cout << "Compiling assets...\n";
    
    for (const auto& descriptor : descriptors) {
        if (compileAsset(descriptor, config)) {
            successCount++;
        } else {
            failCount++;
            std::cerr << "  FAILED: " << descriptor.guid << "\n";
        }
    }
    
    // Print summary
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "\n===========================================\n";
    std::cout << "  Compilation Complete\n";
    std::cout << "===========================================\n";
    std::cout << "  Success: " << successCount << "\n";
    std::cout << "  Failed:  " << failCount << "\n";
    std::cout << "  Time:    " << duration.count() / 1000.0 << "s\n";
    std::cout << "===========================================\n\n";
    
    return (failCount > 0) ? 1 : 0;
}
