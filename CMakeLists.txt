cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler-specific flags
if(MSVC)
    # Visual Studio warnings
    add_compile_options(/W4)
    # Disable specific warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Multi-processor compilation
    add_compile_options(/MP)
else()
    # GCC/Clang warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Scripting Module (if enabled)
if(WIN32 AND MSVC)
    option(ENABLE_SCRIPTING "Enable C# scripting system" ON)
    if(ENABLE_SCRIPTING)
        file(GLOB_RECURSE SCRIPTING_SOURCES 
            "${ENGINE_ROOT}/Scripting/*.cpp"
            "${CMAKE_SOURCE_DIR}/ScriptCore/*.cpp"
        )
        file(GLOB_RECURSE SCRIPTING_HEADERS 
            "${ENGINE_ROOT}/Scripting/*.h"
            "${CMAKE_SOURCE_DIR}/ScriptCore/*.h"
        )
        
        # Add to engine sources
        set(ENGINE_SOURCES ${ENGINE_SOURCES} ${SCRIPTING_SOURCES})
        set(ENGINE_HEADERS ${ENGINE_HEADERS} ${SCRIPTING_HEADERS})
        
        # Add include directories for scripting
        target_include_directories(EngineLib PUBLIC
            ${CMAKE_SOURCE_DIR}/ScriptCore
            ${CMAKE_SOURCE_DIR}/External/dotnet/include
        )
        
        # Define scripting enabled
        target_compile_definitions(EngineLib PUBLIC SCRIPTING_ENABLED)
        
        # Link required libraries
        target_link_libraries(EngineLib PUBLIC shlwapi)
    endif()
endif()

# Add subdirectories
add_subdirectory(External)
add_subdirectory(Engine)
add_subdirectory(Game)

# After the External and Engine subdirectories, add:
# Scripting System (Windows only for now)
include(${CMAKE_SOURCE_DIR}/Scripting.cmake)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/Resources 
     DESTINATION ${CMAKE_BINARY_DIR})

message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "========================================")