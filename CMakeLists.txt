cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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

# Add subdirectories
add_subdirectory(External)
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(AssetCompiler)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/Resources 
     DESTINATION ${CMAKE_BINARY_DIR})

message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "========================================")