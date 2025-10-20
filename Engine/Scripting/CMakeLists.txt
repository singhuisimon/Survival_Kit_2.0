# Find .NET SDK
find_program(DOTNET dotnet)
if(NOT DOTNET)
    message(WARNING "dotnet SDK not found. Script system will be disabled.")
    return()
endif()

# ScriptCore library (native C++ with CoreCLR hosting)
add_library(ScriptCore SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptCore/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptCore/MonoBehaviour.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptCore/pch.cpp

)

target_include_directories(ScriptCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptCore
    ${CMAKE_CURRENT_SOURCE_DIR}/../../External/dotnet/include
)

target_compile_definitions(ScriptCore PRIVATE DLL_API_EXPORT)
set_target_properties(ScriptCore PROPERTIES
    CXX_STANDARD 20
    FOLDER "Engine/Scripting"
)

target_link_libraries(ScriptCore PRIVATE shlwapi)

# Copy .NET runtime to output directory
add_custom_command(TARGET ScriptCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/External/dotnet/runtime"
        "$<TARGET_FILE_DIR:ScriptCore>/dotnet"
    COMMENT "Copying .NET runtime files"
)

# ScriptAPI (C++/CLI bridge) - Only on Windows with MSVC
if(MSVC)
    # Enable C++/CLI for ScriptAPI
    set(CMAKE_VS_DOTNET_TARGET_FRAMEWORK "net8.0")
    
    add_library(ScriptAPI SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptAPI/EngineInterface.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptAPI/Script.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/../../ScriptAPI/TransformComponent.cxx
    )
    
    set_target_properties(ScriptAPI PROPERTIES
        COMMON_LANGUAGE_RUNTIME "NetCore"
        VS_DOTNET_TARGET_FRAMEWORK_VERSION "net8.0"
        VS_GLOBAL_CLRSupport "NetCore"
        FOLDER "Engine/Scripting"
    )
    
    target_link_libraries(ScriptAPI PRIVATE ScriptCore)
    
    # Build ManagedScripts.dll
    add_custom_target(ManagedScripts ALL
        COMMAND ${DOTNET} build 
            "${CMAKE_SOURCE_DIR}/ManagedScripts/ManagedScripts.csproj"
            -c $<CONFIG>
            -o "${CMAKE_BINARY_DIR}/bin"
        COMMENT "Building C# managed scripts"
    )
    
    add_dependencies(ScriptAPI ManagedScripts)
endif()