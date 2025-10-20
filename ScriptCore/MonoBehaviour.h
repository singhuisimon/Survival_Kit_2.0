#pragma once
/**
 * @file MonoBehaviour.h
 * @brief Declaration of the MonoBehaviour class for script template management
 *        and C# script file creation.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Defines the interface for creating MonoBehaviour scripts from
 *          templates, validating script names, and managing script files
 *          within the game engine's scripting system.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "ImportExport.h"

namespace Core
{
    class DLL_API MonoBehaviour
    {
    public:
        // Script Creation Methods
        static bool CreateScript(const std::string& scriptName);
        static bool CreateScriptFromTemplate(const std::string& scriptName, const std::string& templatePath);

        // Template Management
        static bool InitializeTemplates();
        static std::string GetTemplatesDirectory();
        static std::string GetMonoBehaviourTemplatePath();
        static bool DoesTemplateExist(const std::string& templateName);

        // Script File Operations  
        static bool WriteScriptFile(const std::string& scriptName, const std::string& content);
        static std::string GetScriptsDirectory();
        static bool DoesScriptExist(const std::string& scriptName);

        // Template Processing
        static std::string ReadTemplate();
        static std::string ProcessTemplate(const std::string& templateContent, const std::string& className);
        static std::string GetDefaultTemplate();

        // Validation
        static bool ValidateScriptName(const std::string& scriptName);
        static std::string SanitizeClassName(const std::string& name);

        // Utility
        static std::vector<std::string> GetExistingScripts();
        static void PrintScriptCreationInfo(const std::string& scriptName);

        //visual studio opening
        static bool OpenScriptInEditor(const std::string& scriptName);


    private:
        // Template content generation
        static void CreateDefaultTemplateFile();
        static std::string ReplaceTemplateTokens(const std::string& content, const std::string& className);

        // File system helpers
        static bool CreateDirectoryIfNotExists(const std::string& path);
        static bool WriteFileContent(const std::string& filePath, const std::string& content);
        static std::string ReadFileContent(const std::string& filePath);

        // Validation helpers
        static bool IsValidCSharpIdentifier(const std::string& name);
        static bool IsReservedKeyword(const std::string& name);

        // File opening helpers
        static bool OpenFileWithDefaultProgram(const std::string& filePath);

    };
}