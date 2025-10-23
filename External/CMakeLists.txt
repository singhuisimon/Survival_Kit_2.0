# ====================================
# External Libraries Configuration
# ====================================

# GLFW - Windowing library
message(STATUS "Configuring GLFW...")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# GLAD - OpenGL loader
message(STATUS "Configuring GLAD...")
add_library(glad STATIC
    glad/src/glad.c
    glad/include/glad/glad.h
    glad/include/KHR/khrplatform.h
)
target_include_directories(glad PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)
set_target_properties(glad PROPERTIES FOLDER "External")

# GLM - Math library (header-only)
message(STATUS "Configuring GLM...")
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/glm
)

# EnTT - ECS library (header-only)
message(STATUS "Configuring EnTT...")
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/entt
)

# RapidJSON - JSON library (header-only)
message(STATUS "Configuring RapidJSON...")
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson
)

# Tracy - Profiler
message(STATUS "Configuring Tracy...")
add_library(tracy STATIC
    tracy/tracy/public/TracyClient.cpp
)
target_include_directories(tracy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/tracy/tracy/public
)
target_compile_definitions(tracy PUBLIC 
    TRACY_ENABLE
    TRACY_ON_DEMAND
)
set_target_properties(tracy PROPERTIES FOLDER "External")

# ImGuizmo - 3D Gizmos for ImGui
message(STATUS "Configuring ImGuizmo...")
add_library(imguizmo STATIC
    ImGuizmo/ImGuizmo/ImGuizmo.cpp
    ImGuizmo/ImGuizmo/ImGuizmo.h
)
target_include_directories(imguizmo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuizmo/ImGuizmo
)
target_link_libraries(imguizmo PUBLIC imgui)
set_target_properties(imguizmo PROPERTIES FOLDER "External")

# OpenFBX - FBX loader
message(STATUS "Configuring OpenFBX...")
add_library(openfbx STATIC
    openFBX/openFBX/src/ofbx.cpp
    openFBX/openFBX/src/ofbx.h
)
target_include_directories(openfbx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/openFBX/openFBX/src
)
set_target_properties(openfbx PROPERTIES FOLDER "External")

# Jolt Physics
message(STATUS "Configuring Jolt Physics...")
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(GENERATE_DEBUG_SYMBOLS ON CACHE BOOL "" FORCE)
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(CROSS_PLATFORM_DETERMINISTIC OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION ON CACHE BOOL "" FORCE)
set(FLOATING_POINT_EXCEPTIONS_ENABLED ON CACHE BOOL "" FORCE)
set(USE_SSE4_1 ON CACHE BOOL "" FORCE)
set(USE_SSE4_2 ON CACHE BOOL "" FORCE)
set(USE_AVX ON CACHE BOOL "" FORCE)
set(USE_AVX2 ON CACHE BOOL "" FORCE)
set(USE_AVX512 OFF CACHE BOOL "" FORCE)
set(USE_LZCNT ON CACHE BOOL "" FORCE)
set(USE_TZCNT ON CACHE BOOL "" FORCE)
set(USE_F16C ON CACHE BOOL "" FORCE)
set(USE_FMADD ON CACHE BOOL "" FORCE)
add_subdirectory(jolt/Build)
set_target_properties(Jolt PROPERTIES FOLDER "External")

# FMOD - Audio library
message(STATUS "Configuring FMOD...")

if(WIN32)
    set(FMOD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/fmod")
    
    # Include directories
    set(FMOD_CORE_INCLUDE "${FMOD_ROOT}/core/inc")
    set(FMOD_STUDIO_INCLUDE "${FMOD_ROOT}/studio/inc")
    
    # Library directories
    set(FMOD_LIB_DIR "${FMOD_ROOT}/lib/x64")
    set(FMOD_DLL_DIR "${FMOD_ROOT}/dll/x64")
    
    # Check if library files exist
    if(EXISTS "${FMOD_LIB_DIR}/fmod_vc.lib")
        message(STATUS "  FMOD libraries found")
        message(STATUS "    Include: ${FMOD_CORE_INCLUDE}")
        message(STATUS "    Lib dir: ${FMOD_LIB_DIR}")
        
        # Create FMOD interface library
        add_library(fmod INTERFACE)
        
        # Include directories
        target_include_directories(fmod INTERFACE
            ${FMOD_CORE_INCLUDE}
            ${FMOD_STUDIO_INCLUDE}
        )
        
        # Link libraries with full paths
        target_link_libraries(fmod INTERFACE
            # Release libraries
            $<$<CONFIG:Release>:${FMOD_LIB_DIR}/fmod_vc.lib>
            $<$<CONFIG:Release>:${FMOD_LIB_DIR}/fmodstudio_vc.lib>
            
            # Debug libraries
            $<$<CONFIG:Debug>:${FMOD_LIB_DIR}/fmodL_vc.lib>
            $<$<CONFIG:Debug>:${FMOD_LIB_DIR}/fmodstudioL_vc.lib>
        )
        
        set(FMOD_FOUND TRUE PARENT_SCOPE)
        
    else()
        message(WARNING "  FMOD .lib files not found at: ${FMOD_LIB_DIR}")
        message(WARNING "    Expected files:")
        message(WARNING "      - fmod_vc.lib")
        message(WARNING "      - fmodL_vc.lib")
        message(WARNING "      - fmodstudio_vc.lib")
        message(WARNING "      - fmodstudioL_vc.lib")
        message(WARNING "    Audio system will be disabled")
        
        # Create empty interface so project still compiles
        add_library(fmod INTERFACE)
        set(FMOD_FOUND FALSE)
    endif()
    
elseif(UNIX AND NOT APPLE)
    message(STATUS "  FMOD not configured for Linux yet")
    add_library(fmod INTERFACE)
    set(FMOD_FOUND FALSE)
    
elseif(APPLE)
    message(STATUS "  FMOD not configured for macOS yet")
    add_library(fmod INTERFACE)
    set(FMOD_FOUND FALSE)
else()
    add_library(fmod INTERFACE)
    set(FMOD_FOUND FALSE)
endif()

# ImGui - UI library
message(STATUS "Configuring ImGui...")
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui PUBLIC glfw glad)
set_target_properties(imgui PROPERTIES FOLDER "External")

# xresource_guid - Asset library (header-only)
message(STATUS "Configuring xresource_guid...")
add_library(xresource_guid INTERFACE)
target_include_directories(xresource_guid INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/xresource_guid/include
)

# xresource_mgr - Asset library (header-only)
message(STATUS "Configuring xresource_mgr...")
add_library(xresource_mgr INTERFACE)
target_include_directories(xresource_mgr INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/xresource_mgr/include
)


# External Interface Library
add_library(External INTERFACE)
target_link_libraries(External INTERFACE
    glfw
    glad
    glm
    entt
    rapidjson
    tracy
    imguizmo
    openfbx
    Jolt
    fmod
    imgui
    xresource_guid
    xresource_mgr
)

message(STATUS "External libraries configured successfully")