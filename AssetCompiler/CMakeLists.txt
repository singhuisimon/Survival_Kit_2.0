# ====================================
# Asset Compiler - Command Line Tool
# ====================================
message(STATUS "Configuring Asset Compiler...")

set(COMPILER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# Collect source files
file(GLOB_RECURSE COMPILER_SOURCES 
    "${COMPILER_ROOT}/*.cpp"
)

file(GLOB_RECURSE COMPILER_HEADERS 
    "${COMPILER_ROOT}/*.h"
)

# Organize into source groups for IDE
source_group("Main" FILES "${COMPILER_ROOT}/Main/Main.cpp")
source_group("CompilerCore" REGULAR_EXPRESSION 
	"${COMPILER_ROOT}/CompilerCore/.*")
source_group("Utils" REGULAR_EXPRESSION "${COMPILER_ROOT}/Utility/.*")

# Create executable
add_executable(AssetCompiler
    ${COMPILER_SOURCES}
    ${COMPILER_HEADERS}
)

# Include directories
target_include_directories(AssetCompiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/External/rapidjson/include  
    ${CMAKE_SOURCE_DIR}/External/glm   
    ${CMAKE_SOURCE_DIR}/External/openFBX/openFBX/src       
)

# link against OPENFBX library
target_link_libraries(AssetCompiler PRIVATE
    openfbx
)
add_dependencies(AssetCompiler openfbx)

# Link only what we need (NO game engine dependencies!)
# We'll link specific libraries as needed for asset processing

# Platform-specific libraries
if(WIN32)
    # Windows doesn't need extra libs for file I/O
elseif(UNIX AND NOT APPLE)
    target_link_libraries(AssetCompiler PRIVATE pthread dl)
elseif(APPLE)
    target_link_libraries(AssetCompiler PRIVATE
        "-framework CoreFoundation"
    )
endif()

# Set output directory
set_target_properties(AssetCompiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "AssetCompiler"
)

# Preprocessor definitions
target_compile_definitions(AssetCompiler PRIVATE
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG RELEASE>
)

# The compiler is run manually not during build

message(STATUS "Asset Compiler configured successfully")
