cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ====================================
# Output directories (fixed to <source>/build/bin/<Config>)
# Minimal change: anchor outputs to SOURCE dir so Ninja doesn't flip to build/<cfg>/bin
# ====================================
set(_OUT_ROOT "${CMAKE_SOURCE_DIR}/build")

# Base (used for copies / messages)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_OUT_ROOT}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_OUT_ROOT}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_OUT_ROOT}/lib")

if(CMAKE_CONFIGURATION_TYPES)  # multi-config (VS/Xcode/Ninja-Multi)
  foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${cfg} CFGU)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFGU} "${_OUT_ROOT}/bin/${cfg}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFGU} "${_OUT_ROOT}/lib/${cfg}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFGU} "${_OUT_ROOT}/lib/${cfg}")
  endforeach()
else()  # single-config (Ninja/Make)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
  endif()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_OUT_ROOT}/bin/${CMAKE_BUILD_TYPE}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_OUT_ROOT}/lib/${CMAKE_BUILD_TYPE}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_OUT_ROOT}/lib/${CMAKE_BUILD_TYPE}")
endif()

# ====================================
# Compiler Flags - All Configurations
# ====================================

if(MSVC)
    # Visual Studio warnings
    add_compile_options(/W4)
    # Disable specific warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Multi-processor compilation
    add_compile_options(/MP)
    # Enable function-level linking
    add_compile_options(/Gy)
    # Enable intrinsic functions
    add_compile_options(/Oi)
    # Fast floating point
    add_compile_options(/fp:fast)
else()
    # GCC/Clang warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ====================================
# Debug Configuration
# ====================================

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /RTC1")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
endif()

# ====================================
# Release Configuration
# ====================================

if(MSVC)
    # Release optimization flags
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Ot /GL")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /Ot /GL")
    
    # Link-time code generation (LTCG) for Release
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
else()
    # GCC/Clang Release optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -ffast-math")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG -ffast-math")
    
    # Link-time optimization (LTO) for Release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        include(CheckIPOSupported)
        check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
        if(IPO_SUPPORTED)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            message(STATUS "Link-Time Optimization (LTO) enabled for Release")
        else()
            message(STATUS "LTO not supported: ${IPO_ERROR}")
        endif()
    endif()
endif()

# ====================================
# RelWithDebInfo Configuration (Release + Debug Info)
# ====================================

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Ob2 /Zi")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /O2 /Ob2 /Zi")
else()
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O2 -g")
endif()

# ====================================
# Global Preprocessor Definitions
# ====================================

# Tracy profiler (always enabled)
add_compile_definitions(
    TRACY_ENABLE
    TRACY_ON_DEMAND
)

# Configuration-specific defines (works with both single-config and multi-config generators)
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generators (Visual Studio, Xcode)
    add_compile_definitions(
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Release>:RELEASE>
        $<$<CONFIG:RelWithDebInfo>:NDEBUG>
        $<$<CONFIG:RelWithDebInfo>:RELEASE>
    )
else()
    # Single-config generators (Makefiles, Ninja)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_definitions(DEBUG _DEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_definitions(NDEBUG RELEASE)
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_definitions(NDEBUG RELEASE)
    endif()
endif()

# ====================================
# Add Subdirectories
# ====================================

add_subdirectory(External)
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(AssetCompiler)

# Include Mono configuration (this just sets variables, doesn't modify targets)
include(MonoIntegration.cmake)

# Add Scripting source files to your engine
set(SCRIPTING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Scripting/MonoScriptEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Scripting/ScriptSystem.cpp
)

# Assuming your engine library is called "GameEngine" (replace with your actual target name)
target_sources(GameEngine PRIVATE ${SCRIPTING_SOURCES})
target_include_directories(GameEngine PUBLIC ${MONO_INCLUDE_DIR})
target_link_libraries(GameEngine PUBLIC ${MONO_LIBRARIES})

# ====================================
# Copy Resources & Script DLLs next to runtime dir
# ====================================

file(COPY "${CMAKE_SOURCE_DIR}/Resources"
     DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# ===== Build C# scripts (GameScripts.dll) =====
set(SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/Scripts")
set(SCRIPTS_CSPROJ "${SCRIPTS_DIR}/GameScripts.csproj")

if(NOT EXISTS "${SCRIPTS_CSPROJ}")
  message(FATAL_ERROR "C# scripts project not found at: ${SCRIPTS_CSPROJ}")
endif()

# Prefer dotnet; fallback to MSBuild (Visual Studio generators)
find_program(DOTNET_EXE dotnet)
if(NOT DOTNET_EXE)
  # For VS generators, CMAKE_MAKE_PROGRAM is MSBuild.exe
  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(MSBUILD_EXE "${CMAKE_MAKE_PROGRAM}")
  endif()
endif()

# Helper: choose config expression for multi/single config
if(CMAKE_CONFIGURATION_TYPES)
  set(_CFG_GEN "$<CONFIG>")
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  endif()
  set(_CFG_GEN "${CMAKE_BUILD_TYPE}")
endif()

# Build output: put GameScripts.dll directly next to the runtime for the active config
# - dotnet/MSBuild OutDir must end with a trailing slash
set(_CFG_DIR "$<$<BOOL:${CMAKE_CONFIGURATION_TYPES}>:$<CONFIG>>$<$<NOT:$<BOOL:${CMAKE_CONFIGURATION_TYPES}>>:${CMAKE_BUILD_TYPE}>")
set(_SCRIPTS_OUTDIR "${_OUT_ROOT}/bin/${_CFG_DIR}/")

if(DOTNET_EXE)
  add_custom_target(GameScripts
    COMMAND "${DOTNET_EXE}" build "${SCRIPTS_CSPROJ}"
            -c "${_CFG_GEN}"
            -nologo -v:m
            -p:WarningLevel=4
            -p:OutDir="${_SCRIPTS_OUTDIR}"
    WORKING_DIRECTORY "${SCRIPTS_DIR}"
    COMMENT "Building C# scripts with dotnet build (${_CFG_GEN}) -> ${_SCRIPTS_OUTDIR}"
  )
else()
  if(NOT MSBUILD_EXE)
    message(FATAL_ERROR "Neither 'dotnet' nor Visual Studio MSBuild found to build C# scripts.")
  endif()
  add_custom_target(GameScripts
    COMMAND "${MSBUILD_EXE}" "${SCRIPTS_CSPROJ}"
            /restore
            /t:Build
            /p:Configuration="${_CFG_GEN}"
            /p:OutDir="${_SCRIPTS_OUTDIR}"
    WORKING_DIRECTORY "${SCRIPTS_DIR}"
    COMMENT "Building C# scripts with MSBuild (${_CFG_GEN}) -> ${_SCRIPTS_OUTDIR}"
  )
endif()

# Make your main run target depend on scripts
# Adjust these to your actual targets (keep both if you have both)
if(TARGET Survival_Kit_2.0)
  add_dependencies(Survival_Kit_2.0 GameScripts)
endif()
if(TARGET Game) # e.g., your game executable target
  add_dependencies(Game GameScripts)
endif()
if(TARGET GameEngine) # if you want the engine lib to trigger it too
  add_dependencies(GameEngine GameScripts)
endif()


# ====================================
# Build Information
# ====================================

message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        message(STATUS "Optimizations: /O2 /Ob2 /Ot /GL /LTCG")
    else()
        message(STATUS "Optimizations: -O3 -ffast-math")
        if(IPO_SUPPORTED)
            message(STATUS "LTO: Enabled")
        endif()
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug Mode: Full debug info, no optimizations")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Multi-config generator detected")
    message(STATUS "Available configurations: ${CMAKE_CONFIGURATION_TYPES}")
endif()

message(STATUS "SOURCE = ${CMAKE_SOURCE_DIR}")
message(STATUS "BUILD  = ${CMAKE_BINARY_DIR}")
message(STATUS "RUN    = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
