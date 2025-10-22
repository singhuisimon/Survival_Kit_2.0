# ====================================
# Engine Library Configuration
# ====================================
message(STATUS "Configuring Engine...")

# Creating a variable locally for the engine
set(ENGINE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Collect each module's source and header files recursively

# Core Module
file(GLOB_RECURSE CORE_SOURCES "${ENGINE_ROOT}/Core/*.cpp")
file(GLOB_RECURSE CORE_HEADERS "${ENGINE_ROOT}/Core/*.h")

# ECS Module
file(GLOB_RECURSE ECS_SOURCES "${ENGINE_ROOT}/ECS/*.cpp")
file(GLOB_RECURSE ECS_HEADERS "${ENGINE_ROOT}/ECS/*.h")

# Graphics Module
file(GLOB_RECURSE GRAPHICS_SOURCES "${ENGINE_ROOT}/Graphics/*.cpp")
file(GLOB_RECURSE GRAPHICS_HEADERS "${ENGINE_ROOT}/Graphics/*.h")

# Asset Module
file(GLOB_RECURSE ASSET_SOURCES "${ENGINE_ROOT}/Asset/*.cpp")
file(GLOB_RECURSE ASSET_HEADERS "${ENGINE_ROOT}/Asset/*.h")

# Utility Module
file(GLOB_RECURSE UTILITY_SOURCES "${ENGINE_ROOT}/Utility/*.cpp")
file(GLOB_RECURSE UTILITY_HEADERS "${ENGINE_ROOT}/Utility/*.h")

# Editor Module
file(GLOB_RECURSE EDITOR_SOURCES "${ENGINE_ROOT}/Editor/*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "${ENGINE_ROOT}/Editor/*.h")

# Serialization Module
file(GLOB_RECURSE SERIALIZATION_SOURCES "${ENGINE_ROOT}/Serialization/*.cpp")
file(GLOB_RECURSE SERIALIZATION_HEADERS "${ENGINE_ROOT}/Serialization/*.h")

# Prefabs Module
file(GLOB_RECURSE PREFAB_SOURCES "${ENGINE_ROOT}/Prefab/*.cpp")
file(GLOB_RECURSE PREFAB_HEADERS "${ENGINE_ROOT}/Prefab/*.h")

# Combine all files
set(ENGINE_SOURCES
    ${CORE_SOURCES}
    ${ECS_SOURCES}
    ${GRAPHICS_SOURCES}
    ${ASSET_SOURCES}
    ${UTILITY_SOURCES}
    ${EDITOR_SOURCES}
    ${SERIALIZATION_SOURCES}
    ${PREFAB_SOURCES}
)

set(ENGINE_HEADERS
    ${CORE_HEADERS}
    ${ECS_HEADERS}
    ${GRAPHICS_HEADERS}
    ${ASSET_HEADERS}
    ${UTILITY_HEADERS}
    ${EDITOR_HEADERS}
    ${SERIALIZATION_HEADERS}
    ${PREFAB_HEADERS}
)

# Create engine static library
if(ENGINE_SOURCES)
    add_library(EngineLib STATIC
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
    )
    message(STATUS "Engine sources found: ${ENGINE_SOURCES}")
else()
    # Create dummy source file if no sources exist yet
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/dummy.cpp" "// Temporary file for empty library")
    add_library(EngineLib STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/dummy.cpp
    )
    message(WARNING "No engine sources found, using dummy file")
endif()

# ====================================
# SOURCE GROUPS - Header/Source Split
# ====================================

# Core Module
source_group("Core\\Header" FILES ${CORE_HEADERS})
source_group("Core\\Source" FILES ${CORE_SOURCES})

# ECS Module
source_group("ECS\\Header" FILES ${ECS_HEADERS})
source_group("ECS\\Source" FILES ${ECS_SOURCES})

# Graphics Module
source_group("Graphics\\Header" FILES ${GRAPHICS_HEADERS})
source_group("Graphics\\Source" FILES ${GRAPHICS_SOURCES})

# Asset Module
source_group("Asset\\Header" FILES ${ASSET_HEADERS})
source_group("Asset\\Source" FILES ${ASSET_SOURCES})

# Utility Module
source_group("Utility\\Header" FILES ${UTILITY_HEADERS})
source_group("Utility\\Source" FILES ${UTILITY_SOURCES})

# Editor Module
source_group("Editor\\Header" FILES ${EDITOR_HEADERS})
source_group("Editor\\Source" FILES ${EDITOR_SOURCES})

# Serialization Module
source_group("Serialization\\Header" FILES ${SERIALIZATION_HEADERS})
source_group("Serialization\\Source" FILES ${SERIALIZATION_SOURCES})

# Prefab Module
source_group("Prefab\\Header" FILES ${PREFAB_HEADERS})
source_group("Prefab\\Source" FILES ${PREFAB_SOURCES})

# Set target properties
set_target_properties(EngineLib PROPERTIES
    FOLDER "Engine"
    OUTPUT_NAME "Engine"
)

# Include directories for engine
target_include_directories(EngineLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link external dependencies
target_link_libraries(EngineLib PUBLIC External)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(EngineLib PUBLIC opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(EngineLib PUBLIC GL pthread dl)
elseif(APPLE)
    target_link_libraries(EngineLib PUBLIC
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Preprocessor definitions
target_compile_definitions(EngineLib PUBLIC
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG RELEASE>
)

# Additional compile options for debug builds
if(MSVC)
    target_compile_options(EngineLib PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:Release>:/O2>
    )
endif()

# Copy Resources to build directory
add_custom_command(TARGET EngineLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "$<TARGET_FILE_DIR:EngineLib>/../Resources/Scenes"
    COMMENT "Creating Resources/Scenes directory"
)

message(STATUS "Engine library configured successfully")