# ScriptAPI - C++/CLI Bridge
if(MSVC)
    project(ScriptAPI LANGUAGES CXX)
    
    # C++/CLI files
    set(SCRIPTAPI_SOURCES
        EngineInterface.cxx
        Script.cxx
        TransformComponent.cxx
    )
    
    set(SCRIPTAPI_HEADERS
        Debug.hxx
        EngineInterface.hxx
        Script.hxx
        TransformComponent.hxx
    )
    
    add_library(ScriptAPI SHARED
        ${SCRIPTAPI_SOURCES}
        ${SCRIPTAPI_HEADERS}
    )
    
    # Enable C++/CLI
    set_target_properties(ScriptAPI PROPERTIES
        COMMON_LANGUAGE_RUNTIME "NetCore"
        VS_DOTNET_TARGET_FRAMEWORK_VERSION "net8.0"
        VS_GLOBAL_CLRSupport "NetCore"
        FOLDER "Scripting"
    )
    
    # Include directories
    target_include_directories(ScriptAPI PRIVATE
        ${CMAKE_SOURCE_DIR}/ScriptCore
        ${CMAKE_SOURCE_DIR}/External/dotnet/include
    )
    
    # Link ScriptCore
    target_link_libraries(ScriptAPI PRIVATE ScriptCore)
    
    # Output to bin directory
    set_target_properties(ScriptAPI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()