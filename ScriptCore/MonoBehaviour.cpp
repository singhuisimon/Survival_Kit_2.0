#include "pch.h"
/**
 * @file MonoBehaviour.cpp
 * @brief Implementation of the MonoBehaviour class for creating and managing
 *        C# script files from templates.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Provides functionality for script creation, template processing,
 *          validation, and file system operations for MonoBehaviour scripts.
 *          Handles script name sanitization, C# identifier validation, and
 *          automatic opening of created scripts in the default editor.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "MonoBehaviour.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>



namespace Core
{
    // Public Methods

    bool MonoBehaviour::CreateScript(const std::string& scriptName)
    {
        try
        {
            std::cout << "Creating MonoBehaviour script: " << scriptName << std::endl;

            // Validate script name
            if (!ValidateScriptName(scriptName))
            {
                std::cout << "Error: Invalid script name '" << scriptName << "'" << std::endl;
                return false;
            }

            // Check if script already exists
            if (DoesScriptExist(scriptName))
            {
                std::cout << "Error: Script '" << scriptName << "' already exists" << std::endl;
                return false;
            }

            // Initialize templates (creates template file if it doesn't exist)
            if (!InitializeTemplates())
            {
                std::cout << "Error: Failed to initialize templates" << std::endl;
                return false;
            }

            // Read and process template
            std::string templateContent = ReadTemplate();
            if (templateContent.empty())
            {
                std::cout << "Warning: Template file empty, using default template" << std::endl;
                templateContent = GetDefaultTemplate();
            }

            std::string className = SanitizeClassName(scriptName);
            std::string processedContent = ProcessTemplate(templateContent, className);

            // Write the script file
            if (!WriteScriptFile(className, processedContent))
            {
                std::cout << "Error: Failed to write script file" << std::endl;
                return false;
            }

            PrintScriptCreationInfo(className);

            // Auto-open the created script
            std::cout << "Opening script in editor..." << std::endl;
            OpenScriptInEditor(className);

            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception creating script: " << e.what() << std::endl;
            return false;
        }
    }

    bool MonoBehaviour::CreateScriptFromTemplate(const std::string& scriptName, const std::string& templatePath)
    {
        try
        {
            std::string templateContent = ReadFileContent(templatePath);
            if (templateContent.empty())
            {
                std::cout << "Error: Could not read template from " << templatePath << std::endl;
                return false;
            }

            std::string className = SanitizeClassName(scriptName);
            std::string processedContent = ProcessTemplate(templateContent, className);

            return WriteScriptFile(className, processedContent);
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception creating script from template: " << e.what() << std::endl;
            return false;
        }
    }

    // Template Management

    bool MonoBehaviour::InitializeTemplates()
    {
        try
        {
            std::string templatesDir = GetTemplatesDirectory();

            // Create templates directory
            if (!CreateDirectoryIfNotExists(templatesDir))
            {
                std::cout << "Error: Could not create templates directory" << std::endl;
                return false;
            }

            // ALWAYS recreate the template file to use the latest version
            std::string templatePath = GetMonoBehaviourTemplatePath();
            CreateDefaultTemplateFile();  // Remove the if condition
            std::cout << "Created/Updated default MonoBehaviour template" << std::endl;

            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception initializing templates: " << e.what() << std::endl;
            return false;
        }
    }

    std::string MonoBehaviour::GetTemplatesDirectory()
    {
        return "..\\..\\Templates\\Scripts\\";
    }

    std::string MonoBehaviour::GetMonoBehaviourTemplatePath()
    {
        return GetTemplatesDirectory() + "MonoBehaviour.cs.template";
    }

    bool MonoBehaviour::DoesTemplateExist(const std::string& templateName)
    {
        std::string templatePath = GetTemplatesDirectory() + templateName + ".cs.template";
        return std::filesystem::exists(templatePath);
    }

    // Script File Operations

    bool MonoBehaviour::WriteScriptFile(const std::string& scriptName, const std::string& content)
    {
        try
        {
            std::string scriptsDir = GetScriptsDirectory();

            // Create scripts directory if it doesn't exist
            if (!CreateDirectoryIfNotExists(scriptsDir))
            {
                return false;
            }

            std::string fileName = scriptName + ".cs";
            std::string fullPath = scriptsDir + fileName;

            return WriteFileContent(fullPath, content);
        }
        catch (const std::exception& e)
        {
            std::cout << "Exception writing script file: " << e.what() << std::endl;
            return false;
        }
    }

    std::string MonoBehaviour::GetScriptsDirectory()
    {
        return "..\\..\\ManagedScripts\\";
    }

    bool MonoBehaviour::DoesScriptExist(const std::string& scriptName)
    {
        std::string className = SanitizeClassName(scriptName);
        std::string scriptsDir = GetScriptsDirectory();
        std::string fileName = className + ".cs";
        std::string fullPath = scriptsDir + fileName;

        return std::filesystem::exists(fullPath);
    }

    // Template Processing

    std::string MonoBehaviour::ReadTemplate()
    {
        std::string templatePath = GetMonoBehaviourTemplatePath();
        return ReadFileContent(templatePath);
    }

    std::string MonoBehaviour::ProcessTemplate(const std::string& templateContent, const std::string& className)
    {
        return ReplaceTemplateTokens(templateContent, className);
    }

    std::string MonoBehaviour::GetDefaultTemplate()
    {
        return R"(using ScriptAPI;

public class {CLASS_NAME} : Script
{
    // Simple test script - no SerializeField for now
    private int health = 100;
    private float speed = 5.0f;
    private string objectName = "{CLASS_NAME}";

    public override void Update()
    {
        // Called every frame
        // Add your update logic here
    }
})";
    }

    // Validation

    bool MonoBehaviour::ValidateScriptName(const std::string& scriptName)
    {
        if (scriptName.empty())
        {
            std::cout << "Script name cannot be empty" << std::endl;
            return false;
        }

        if (scriptName.length() > 100)
        {
            std::cout << "Script name too long (max 100 characters)" << std::endl;
            return false;
        }

        return IsValidCSharpIdentifier(scriptName);
    }

    std::string MonoBehaviour::SanitizeClassName(const std::string& name)
    {
        if (name.empty())
            return "NewMonoBehaviour";

        std::string result;
        bool firstChar = true;

        for (char c : name)
        {
            if (std::isalnum(c) || c == '_')
            {
                if (firstChar)
                {
                    result += std::toupper(c);
                    firstChar = false;
                }
                else
                {
                    result += c;
                }
            }
        }

        // Ensure it doesn't start with a number
        if (!result.empty() && std::isdigit(result[0]))
        {
            result = "Script" + result;
        }

        return result.empty() ? "NewMonoBehaviour" : result;
    }

    // Utility

    std::vector<std::string> MonoBehaviour::GetExistingScripts()
    {
        std::vector<std::string> scripts;
        std::string scriptsDir = GetScriptsDirectory();

        try
        {
            if (!std::filesystem::exists(scriptsDir))
                return scripts;

            for (const auto& entry : std::filesystem::directory_iterator(scriptsDir))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cs")
                {
                    std::string filename = entry.path().stem().string();
                    scripts.push_back(filename);
                }
            }

            // Sort alphabetically
            std::sort(scripts.begin(), scripts.end());
        }
        catch (const std::exception& e)
        {
            std::cout << "Error reading scripts directory: " << e.what() << std::endl;
        }

        return scripts;
    }

    void MonoBehaviour::PrintScriptCreationInfo(const std::string& scriptName)
    {
        std::cout << "\n=== Script Created Successfully ===" << std::endl;
        std::cout << "Script Name: " << scriptName << ".cs" << std::endl;
        std::cout << "Location: " << GetScriptsDirectory() << scriptName << ".cs" << std::endl;
        std::cout << "Type: MonoBehaviour" << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "1. The script will be compiled automatically" << std::endl;
        std::cout << "2. You can now attach it to entities in your scene" << std::endl;
        std::cout << "3. Edit the script to add custom behavior" << std::endl;
        std::cout << "===================================\n" << std::endl;
    }

    // Private Helper Methods

    void MonoBehaviour::CreateDefaultTemplateFile()
    {
        std::string templatePath = GetMonoBehaviourTemplatePath();
        std::string defaultContent = GetDefaultTemplate();

        WriteFileContent(templatePath, defaultContent);
    }

    std::string MonoBehaviour::ReplaceTemplateTokens(const std::string& content, const std::string& className)
    {
        std::string result = content;
        std::string token = "{CLASS_NAME}";

        size_t pos = 0;
        while ((pos = result.find(token, pos)) != std::string::npos)
        {
            result.replace(pos, token.length(), className);
            pos += className.length();
        }

        return result;
    }

    // File System Helpers

    bool MonoBehaviour::CreateDirectoryIfNotExists(const std::string& path)
    {
        try
        {
            if (!std::filesystem::exists(path))
            {
                std::filesystem::create_directories(path);
                std::cout << "Created directory: " << path << std::endl;
            }
            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "Error creating directory " << path << ": " << e.what() << std::endl;
            return false;
        }
    }

    bool MonoBehaviour::WriteFileContent(const std::string& filePath, const std::string& content)
    {
        try
        {
            std::ofstream file(filePath);
            if (!file.is_open())
            {
                std::cout << "Error: Could not open file for writing: " << filePath << std::endl;
                return false;
            }

            file << content;
            file.close();

            std::cout << "Successfully wrote file: " << filePath << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cout << "Error writing file " << filePath << ": " << e.what() << std::endl;
            return false;
        }
    }

    std::string MonoBehaviour::ReadFileContent(const std::string& filePath)
    {
        try
        {
            std::ifstream file(filePath);
            if (!file.is_open())
            {
                std::cout << "Warning: Could not open file: " << filePath << std::endl;
                return "";
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            return buffer.str();
        }
        catch (const std::exception& e)
        {
            std::cout << "Error reading file " << filePath << ": " << e.what() << std::endl;
            return "";
        }
    }

    // Validation Helpers

    bool MonoBehaviour::IsValidCSharpIdentifier(const std::string& name)
    {
        if (name.empty())
            return false;

        // Must start with letter or underscore
        if (!std::isalpha(name[0]) && name[0] != '_')
        {
            std::cout << "Script name must start with a letter or underscore" << std::endl;
            return false;
        }

        // All characters must be alphanumeric or underscore
        for (char c : name)
        {
            if (!std::isalnum(c) && c != '_')
            {
                std::cout << "Script name can only contain letters, numbers, and underscores" << std::endl;
                return false;
            }
        }

        // Check if it's a reserved keyword
        if (IsReservedKeyword(name))
        {
            std::cout << "Script name cannot be a C# reserved keyword" << std::endl;
            return false;
        }

        return true;
    }

    bool MonoBehaviour::IsReservedKeyword(const std::string& name)
    {
        static const std::set<std::string> keywords = {
            "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked",
            "class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else",
            "enum", "event", "explicit", "extern", "false", "finally", "fixed", "float", "for",
            "foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
            "long", "namespace", "new", "null", "object", "operator", "out", "override", "params",
            "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short",
            "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true",
            "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual",
            "void", "volatile", "while"
        };

        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        return keywords.find(lowerName) != keywords.end();
    }

    // File opening functionality
    bool MonoBehaviour::OpenScriptInEditor(const std::string& scriptName)
    {
        std::string className = SanitizeClassName(scriptName);
        std::string scriptsDir = GetScriptsDirectory();
        std::string fileName = className + ".cs";
        std::string fullPath = scriptsDir + fileName;

        if (!std::filesystem::exists(fullPath))
        {
            std::cout << "Script file does not exist: " << fullPath << std::endl;
            return false;
        }

        return OpenFileWithDefaultProgram(fullPath);
    }

    bool MonoBehaviour::OpenFileWithDefaultProgram(const std::string& filePath)
    {
        try
        {
            // Convert forward slashes to backslashes for Windows
            std::string windowsPath = filePath;
            std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');

            // Use Windows file association to open with default program
            std::string command = "start \"\" \"" + windowsPath + "\"";
            std::cout << "Opening file: " << windowsPath << std::endl;

            int result = system(command.c_str());
            return result == 0;
        }
        catch (const std::exception& e)
        {
            std::cout << "Error opening file: " << e.what() << std::endl;
            return false;
        }
    }
}