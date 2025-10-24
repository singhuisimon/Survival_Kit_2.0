cmake_minimum_required(VERSION 3.24)

if (MSVC)
  add_library(ScriptAPI SHARED
    Debug.hxx
    EngineInterface.hxx EngineInterface.cxx
    MathTypes.hxx
    Script.hxx Script.cxx
    TransformComponent.hxx TransformComponent.cxx
    RigidbodyComponent.hxx  RigidbodyComponent.cxx
  )
  target_include_directories(ScriptAPI PRIVATE
    ${CMAKE_SOURCE_DIR}/Engine
    ${CMAKE_SOURCE_DIR}/Engine/Core
  )
  # Link against your Engine target to access ScriptingBridge
  target_link_libraries(ScriptAPI PRIVATE Engine)

  # Enable C++/CLI and catch both managed & native exceptions (Part 6)
  target_compile_options(ScriptAPI PRIVATE /clr /EHa)
endif()
