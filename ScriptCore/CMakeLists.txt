cmake_minimum_required(VERSION 3.23)
project(ScriptCore LANGUAGES CXX)

add_library(ScriptCore SHARED
    src/ECSBridge.cpp
    src/ScriptEngine.cpp
)

target_include_directories(ScriptCore PUBLIC include)

target_compile_definitions(ScriptCore PRIVATE SCRIPTCORE_EXPORTS)
