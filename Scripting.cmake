# Scripting System Configuration
if(WIN32 AND MSVC)
    option(ENABLE_SCRIPTING "Enable C# scripting system" ON)
    
    if(ENABLE_SCRIPTING)
        message(STATUS "========================================")
        message(STATUS "Configuring C# Scripting System...")
        
        # Find .NET SDK
        find_program(DOTNET dotnet)
        if(NOT DOTNET)
            message(WARNING "dotnet SDK not found. Scripting will be disabled.")
            set(ENABLE_SCRIPTING OFF)
            return()
        endif()
        
        message(STATUS "Found .NET SDK: ${DOTNET}")
        
        # ScriptCore (Native DLL with CoreCLR hosting)
        add_library(ScriptCore SHARED
            ScriptCore/Application.cpp
            ScriptCore/MonoBehaviour.cpp
            ScriptCore/ScriptBridge.cpp
            ScriptCore/pch.cpp
            ScriptCore/dllmain.cpp
        )
        
        target_include_directories(ScriptCore PUBLIC
            ${CMAKE_SOURCE_DIR}/ScriptCore
            ${CMAKE_SOURCE_DIR}/External/dotnet/include
            ${CMAKE_SOURCE_DIR}/External/glm  # For glm types
        )
        
        target_compile_definitions(ScriptCore PRIVATE DLL_API_EXPORT)
        target_precompile_headers(ScriptCore PRIVATE ScriptCore/pch.h)
        
        set_target_properties(ScriptCore PROPERTIES
            CXX_STANDARD 20
            FOLDER "Scripting"
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        
        target_link_libraries(ScriptCore PRIVATE shlwapi)
        
        # Copy .NET runtime files
        add_custom_command(TARGET ScriptCore POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/External/dotnet/runtime"
                "$<TARGET_FILE_DIR:ScriptCore>/dotnet"
            COMMENT "Copying .NET runtime files"
        )
        
        # ScriptAPI (C++/CLI Bridge)
        add_subdirectory(ScriptAPI)
        
        # Build ManagedScripts
        add_custom_target(ManagedScripts ALL
            COMMAND ${DOTNET} build 
                "${CMAKE_SOURCE_DIR}/ManagedScripts/ManagedScripts.csproj"
                -c $<CONFIG>
                -o "${CMAKE_BINARY_DIR}/bin"
            DEPENDS ScriptAPI
            COMMENT "Building C# managed scripts"
        )
        
        # Add to engine
        target_compile_definitions(EngineLib PUBLIC SCRIPTING_ENABLED)
        target_link_libraries(EngineLib PUBLIC ScriptCore)
        
        message(STATUS "C# Scripting System configured successfully")
        message(STATUS "========================================")
    endif()
endif()