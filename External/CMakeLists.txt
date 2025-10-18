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
add_library(fmod INTERFACE)
target_include_directories(fmod INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/fmod/core/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/fmod/studio/inc
)
target_link_directories(fmod INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/fmod/core/lib/x64
    ${CMAKE_CURRENT_SOURCE_DIR}/fmod/studio/lib/x64
)
target_link_libraries(fmod INTERFACE
    fmod_vc.lib
    fmodstudio_vc.lib
)

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
)

message(STATUS "External libraries configured successfully")