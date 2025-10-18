# ====================================
# Engine Library Configuration
# ====================================
message(STATUS "Configuring Engine...")

# Collect all engine source files recursively
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ECS/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Editor/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serialization/*.cpp"
)

# Collect all engine header files recursively
file(GLOB_RECURSE ENGINE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ECS/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Utility/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Editor/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serialization/*.h"
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