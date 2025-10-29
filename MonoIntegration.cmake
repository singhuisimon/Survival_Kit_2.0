# MonoIntegration.cmake
# Mono C# Scripting System Integration for CMake
# This file finds and configures Mono for your game engine

# Find Mono based on platform
if(WIN32)
    # Windows: Look for Mono installation
    set(MONO_SEARCH_PATHS
        "C:/Program Files/Mono"
        "C:/Program Files (x86)/Mono"
        "$ENV{ProgramFiles}/Mono"
        "$ENV{MONO_ROOT}"
    )
    
    foreach(search_path ${MONO_SEARCH_PATHS})
        if(EXISTS "${search_path}/include/mono-2.0/mono/jit/jit.h")
            set(MONO_ROOT "${search_path}")
            break()
        endif()
    endforeach()
    
    if(NOT MONO_ROOT)
        message(FATAL_ERROR "Mono not found! Please install Mono or set MONO_ROOT environment variable")
    endif()
    
    set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
    set(MONO_LIB_DIR "${MONO_ROOT}/lib")
    
    # Find the correct library name
    if(EXISTS "${MONO_LIB_DIR}/mono-2.0-sgen.lib")
        set(MONO_LIBRARIES "${MONO_LIB_DIR}/mono-2.0-sgen.lib")
    elseif(EXISTS "${MONO_LIB_DIR}/mono-2.0.lib")
        set(MONO_LIBRARIES "${MONO_LIB_DIR}/mono-2.0.lib")
    elseif(EXISTS "${MONO_LIB_DIR}/monosgen-2.0.lib")
        set(MONO_LIBRARIES "${MONO_LIB_DIR}/monosgen-2.0.lib")
    else()
        message(FATAL_ERROR "Mono library not found in ${MONO_LIB_DIR}")
    endif()
    
    message(STATUS "Mono found (Windows): ${MONO_ROOT}")
    
elseif(UNIX AND NOT APPLE)
    # Linux: Use pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MONO mono-2)
    
    if(NOT MONO_FOUND)
        pkg_check_modules(MONO monosgen-2)
    endif()
    
    if(NOT MONO_FOUND)
        message(FATAL_ERROR "Mono not found! Install with: sudo apt-get install mono-devel")
    endif()
    
    set(MONO_INCLUDE_DIR ${MONO_INCLUDE_DIRS})
    set(MONO_LIBRARIES ${MONO_LIBRARIES})
    
    message(STATUS "Mono found (Linux)")
    
elseif(APPLE)
    # macOS: Check for framework or Homebrew installation
    if(EXISTS "/Library/Frameworks/Mono.framework")
        set(MONO_ROOT "/Library/Frameworks/Mono.framework/Versions/Current")
        set(MONO_INCLUDE_DIR "${MONO_ROOT}/include/mono-2.0")
        set(MONO_LIBRARIES "${MONO_ROOT}/lib/libmonosgen-2.0.dylib")
    else()
        # Try Homebrew
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(MONO REQUIRED mono-2)
        set(MONO_INCLUDE_DIR ${MONO_INCLUDE_DIRS})
        set(MONO_LIBRARIES ${MONO_LIBRARIES})
    endif()
    
    message(STATUS "Mono found (macOS)")
endif()

# Export variables for use in main CMakeLists.txt
set(MONO_INCLUDE_DIR ${MONO_INCLUDE_DIR} CACHE PATH "Mono include directory")
set(MONO_LIBRARIES ${MONO_LIBRARIES} CACHE STRING "Mono libraries")

# Print configuration
message(STATUS "Mono Configuration:")
message(STATUS "  Include Dir: ${MONO_INCLUDE_DIR}")
message(STATUS "  Libraries: ${MONO_LIBRARIES}")

# DON'T automatically add to targets - let the main CMakeLists.txt handle this
# The main CMakeLists.txt should use these variables like:
#   target_include_directories(YourTarget PUBLIC ${MONO_INCLUDE_DIR})
#   target_link_libraries(YourTarget PUBLIC ${MONO_LIBRARIES})