cmake_minimum_required(VERSION 3.23)
project(ScriptAPI LANGUAGES CXX)

add_library(ScriptAPI SHARED
    src/ScriptBase.cxx
    src/ScriptEntity.cxx
    src/ScriptTransform.cxx
    src/ScriptInput.cxx
)

# Clean: All include directories in a single, correct statement
target_include_directories(ScriptAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ScriptCore/include
)

target_link_libraries(ScriptAPI PUBLIC ScriptCore)

# Output binaries to common location to avoid LNK1104 errors
set_target_properties(ScriptAPI PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable /clr for MSVC for C++/CLI
if(MSVC)
    set_target_properties(ScriptAPI PROPERTIES
        COMMON_LANGUAGE_RUNTIME ""
    )
endif()
