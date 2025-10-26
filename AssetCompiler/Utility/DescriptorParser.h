/**
 * @file DescriptorParser.h
 * @brief Utility functions for parsing descriptor files
 * @details Provides JSON parsing and data extraction from Info.txt and Descriptor.txt
 */

#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "../rapidjson/document.h"
#include "../rapidjson/istreamwrapper.h"

namespace AssetCompiler {

/**
 * @brief Parse a JSON file and return a RapidJSON document
 * @param filepath Path to JSON file
 * @param doc Output document
 * @return True if parsing succeeded
 */
inline bool parseJsonFile(const std::string& filepath, rapidjson::Document& doc) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "ERROR: Could not open file: " << filepath << "\n";
        return false;
    }
    
    rapidjson::IStreamWrapper isw(ifs);
    doc.ParseStream(isw);
    
    if (doc.HasParseError()) {
        std::cerr << "ERROR: JSON parse error in " << filepath << "\n";
        std::cerr << "  Error code: " << doc.GetParseError() << "\n";
        std::cerr << "  Offset: " << doc.GetErrorOffset() << "\n";
        return false;
    }
    
    return true;
}

/**
 * @brief Extract source path from Descriptor.txt
 * @param descriptorPath Path to Descriptor.txt
 * @return Source asset path, or empty string on error
 */
inline std::string extractSourcePath(const std::string& descriptorPath) {
    rapidjson::Document doc;
    if (!parseJsonFile(descriptorPath, doc)) {
        return "";
    }
    
    if (!doc.HasMember("sourcePath") || !doc["sourcePath"].IsString()) {
        std::cerr << "ERROR: No 'sourcePath' field in descriptor: " << descriptorPath << "\n";
        return "";
    }
    
    return doc["sourcePath"].GetString();
}

/**
 * @brief Extract display name from Info.txt
 * @param infoPath Path to Info.txt
 * @return Display name, or empty string on error
 */
inline std::string extractDisplayName(const std::string& infoPath) {
    rapidjson::Document doc;
    if (!parseJsonFile(infoPath, doc)) {
        return "";
    }
    
    if (doc.HasMember("name") && doc["name"].IsString()) {
        return doc["name"].GetString();
    }
    
    return "";
}

/**
 * @brief Check if a compiled asset is up-to-date
 * @param descriptorPath Path to Descriptor.txt
 * @param compiledPath Path to compiled output file
 * @return True if compiled file is newer than descriptor
 */
inline bool isUpToDate(const std::string& descriptorPath, const std::string& compiledPath) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(compiledPath)) {
        return false;  // Compiled file doesn't exist
    }
    
    auto descriptorTime = fs::last_write_time(descriptorPath);
    auto compiledTime = fs::last_write_time(compiledPath);
    
    return compiledTime >= descriptorTime;
}

} // namespace AssetCompiler
