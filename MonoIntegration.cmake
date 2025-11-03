# MonoIntegration.cmake
# Uses bundled Mono from External/mono (not system Mono)

message(STATUS "Configuring bundled Mono integration...")

if(WIN32)
    # Use bundled Mono instead of system installation
    set(MONO_ROOT "${CMAKE_SOURCE_DIR}/External/mono" CACHE PATH "Path to bundled Mono")
    set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
    set(MONO_LIB_DIR "${MONO_ROOT}/lib")
    set(MONO_LIBRARIES "${MONO_LIB_DIR}/mono-2.0-sgen.lib")
    
    # Verify bundled Mono exists
    if(NOT EXISTS "${MONO_INCLUDE_DIR}")
        message(FATAL_ERROR "Bundled Mono headers not found at: ${MONO_INCLUDE_DIR}")
    endif()
    
    if(NOT EXISTS "${MONO_LIBRARIES}")
        message(FATAL_ERROR "Bundled Mono library not found at: ${MONO_LIBRARIES}")
    endif()
    
    message(STATUS "✓ Using bundled Mono (Windows): ${MONO_ROOT}")
    
elseif(UNIX AND NOT APPLE)
    # Linux support (future)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MONO REQUIRED mono-2)
    set(MONO_INCLUDE_DIR ${MONO_INCLUDE_DIRS})
    set(MONO_LIBRARIES ${MONO_LIBRARIES})
    message(STATUS "Mono found (Linux)")
    
elseif(APPLE)
    message(FATAL_ERROR "macOS Mono bundling not yet implemented")
endif()

# Export variables for use in other CMake files
set(MONO_INCLUDE_DIR ${MONO_INCLUDE_DIR} CACHE PATH "Mono include directory")
set(MONO_LIBRARIES ${MONO_LIBRARIES} CACHE STRING "Mono libraries")
set(MONO_ROOT ${MONO_ROOT} CACHE PATH "Mono root directory")

message(STATUS "Mono Configuration:")
message(STATUS "  Root: ${MONO_ROOT}")
message(STATUS "  Include: ${MONO_INCLUDE_DIR}")
message(STATUS "  Library: ${MONO_LIBRARIES}")
